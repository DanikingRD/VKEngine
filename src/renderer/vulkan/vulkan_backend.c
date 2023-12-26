#include "vulkan_backend.h"
#include "collections/vector.h"
#include "core/log.h"
#include "core/str.h"
#include "defines.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_renderpass.h"
#include "vulkan_shader.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.h"
#include "vulkan_utils.h"
#include "window.h"

VkBool32 vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                               VkDebugUtilsMessageTypeFlagsEXT type,
                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                               void* data);

i32 find_memory_Type(u32 type_filter, VkMemoryPropertyFlags properties);
static void allocate_command_buffers(VulkanBackend* backend);
static void create_framebuffers(VulkanBackend* backend);
static bool recreate_swapchain(void);
static void buffer_data_transfer(VulkanBackend* ctx, VkCommandPool pool, VkFence fence,
                                 VkQueue queue, Buffer* dst, u64 dst_offset, u64 size, void* data);

static VulkanBackend backend;

bool vulkan_backend_create(const char* app_name, Window* window) {

    backend.find_memory_type = find_memory_Type;

    window_get_framebuffer_size(window, &backend.framebuffer_width, &backend.framebuffer_height);

    INFO("Current framebuffer (width, height): (%d, %d)", backend.framebuffer_width,
         backend.framebuffer_height);

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = app_name;
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    Vector(const char*) extensions = vector_new(const char*);
    window_required_vulkan_extensions(&extensions);
    // NOTE: this only exists for debug builds.
    Vector(const char*) validation_layers = 0;
    vector_push(extensions, &VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

#ifdef DEBUG
    vector_push(extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    DEBUG("Required Extensions: ");
    for (u32 i = 0; i < vector_length(extensions); i++) {
        DEBUG(" - %s ", extensions[i]);
    }
    validation_layers = vector_new(const char*);
    vector_push(validation_layers, &"VK_LAYER_KHRONOS_validation");
    u32 layer_count;
    VK_FN_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, 0));
    VkLayerProperties available_layers[layer_count];
    VK_FN_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, available_layers));

    DEBUG("Required Validation Layers: ");
    for (u32 i = 0; i < 1; i++) {
        bool found = false;
        const char* required_layer = validation_layers[i];
        DEBUG(" - %s", required_layer);
        for (u32 j = 0; j < layer_count; j++) {
            if (str_equals(required_layer, available_layers[j].layerName)) {
                found = true;
                break;
            }
        }
        if (!found) {
            ERROR("Missing Vulkan Validation Layer: %s", required_layer);
        }
    }
    DEBUG("Validation Layers available");
#endif

    create_info.enabledExtensionCount = vector_length(extensions);
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount = vector_length(validation_layers);
    create_info.ppEnabledLayerNames = validation_layers;
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VK_FN_CHECK(vkCreateInstance(&create_info, backend.allocator, &backend.instance));
    // ensure these are freed
    vector_free(extensions);
    vector_free(validation_layers);

#ifdef _DEBUG
    u32 severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    // NOTE: It might be useful to add info & verbose severity for debugging
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vulkan_debug_callback;
    debug_create_info.pUserData = 0;
    PFN_vkCreateDebugUtilsMessengerEXT fn =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(backend.instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");

    VK_FN_CHECK(fn(backend.instance, &debug_create_info, backend.allocator, &backend.debugger));
    DEBUG("Vulkan Debugger created");
#endif

    window_create_vulkan_surface(window, &backend);
    DEBUG("Vulkan Surface created");

    if (!vulkan_device_create(&backend)) {
        ERROR("Failed to create Vulkan Device");
        return false;
    }
    DEBUG("Vulkan Device created");
    if (!vulkan_swapchain_create(&backend, backend.framebuffer_width, backend.framebuffer_height,
                                 &backend.swapchain)) {
        ERROR("Failed to create Vulkan Swapchain");
        return false;
    }
    DEBUG("Vulkan Swapchain created");
    Vec4 area = (Vec4){0, 0, backend.framebuffer_width, backend.framebuffer_height};
    Vec4 clear_color = (Vec4){0.4, 0.5, 0.6, 1.0};
    vulkan_renderpass_create(&backend, area, clear_color, 1.0f, 0.0f, &backend.main_pass);
    DEBUG("Vulkan Main Renderpass created");
    allocate_command_buffers(&backend);
    DEBUG("Vulkan Command Buffers allocated");
    create_framebuffers(&backend);
    DEBUG("Vulkan Framebuffers created");

    for (u32 i = 0; i < backend.swapchain.max_frames_in_flight; i++) {
        VkSemaphoreCreateInfo semaphore_info = {0};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_FN_CHECK(vkCreateSemaphore(backend.device.logical, &semaphore_info, backend.allocator,
                                      &backend.image_available_semaphores[i]));
        VK_FN_CHECK(vkCreateSemaphore(backend.device.logical, &semaphore_info, backend.allocator,
                                      &backend.queue_complete_semaphores[i]));

        VkFenceCreateInfo fence_info = {0};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_FN_CHECK(vkCreateFence(backend.device.logical, &fence_info, backend.allocator,
                                  &backend.in_flight_fences[i]));
    }

    if (!vulkan_shader_create(&backend, &backend.basic_shader)) {
        ERROR("Failed to create Vulkan Shader");
        return false;
    }

    VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    u64 vertex_buffer_size = sizeof(Vertex) * 1024 * 1024;
    vulkan_buffer_create(&backend,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         memory_flags, vertex_buffer_size, true, &backend.vertex_buffer);
    INFO("Vertex Buffer created");

    u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
    vulkan_buffer_create(&backend,
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         memory_flags, index_buffer_size, true, &backend.index_buffer);

    INFO("Index Buffer created");

    const u32 vertices = 4;
    Vertex verts[vertices];

    verts[0].pos.x = -0.5f;
    verts[0].pos.y = -0.5f;

    verts[1].pos.x = 0.5f;
    verts[1].pos.y = -0.5f;

    verts[2].pos.x = 0.5f;
    verts[2].pos.y = 0.5f;

    verts[3].pos.x = -0.5f;
    verts[3].pos.y = 0.5f;

    buffer_data_transfer(&backend, backend.device.graphics_command_pool, 0,
                         backend.device.graphics_queue, &backend.vertex_buffer, 0,
                         sizeof(Vertex) * vertices, verts);

    const u32 indices = 6;
    u32 indices_data[] = {0, 1, 2, 2, 3, 0};

    buffer_data_transfer(&backend, backend.device.graphics_command_pool, 0,
                         backend.device.graphics_queue, &backend.index_buffer, 0,
                         sizeof(u32) * indices, indices_data);

    INFO("Vulkan Backend initializated");
    return true;
}

void vulkan_backend_resize(u16 width, u16 height) {
    backend.framebuffer_width = width;
    backend.framebuffer_height = height;
    backend.swapchain_needs_resize = true;
    INFO("Resizing backend to: (%dx%d)", width, height);
}

bool vulkan_backend_begin_frame(f32 dt) {
    Device* device = &backend.device;
    if (backend.recreating_swapchain) {
        if (!vulkan_result_is_ok(vkDeviceWaitIdle(device->logical))) {
            ERROR("A fatal error occurred while waiting for recreating swapchain");
            return false;
        }
        return false;
    }
    if (backend.swapchain_needs_resize) {
        if (!vulkan_result_is_ok(vkDeviceWaitIdle(device->logical))) {
            ERROR("A fatal error occurred while waiting for recreating swapchain");
            return false;
        }
        if (!recreate_swapchain()) {
            return false;
        }
        return false;
    }

    // Wait for the current frame to be completed.
    VkResult result = vkWaitForFences(
        device->logical, 1, &backend.in_flight_fences[backend.current_frame], VK_TRUE, UINT64_MAX);

    if (result != VK_SUCCESS) {
        ERROR("Failed to wait for InFlight fence");
        return false;
    }

    if (!vulkan_swapchain_acquire_next_image(
            &backend, backend.image_available_semaphores[backend.current_frame], 0,
            &backend.swapchain, UINT64_MAX, &backend.image_index)) {
        ERROR("Could not acquire image.");
        return false;
    }

    CommandBuffer* gfx_cmdbuf = &backend.graphics_command_buffers[backend.image_index];
    vkResetCommandBuffer(gfx_cmdbuf->handle, 0);
    vulkan_command_buffer_begin(gfx_cmdbuf, false, false, false);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = backend.framebuffer_height;
    viewport.width = (f32)backend.framebuffer_width;
    viewport.height = -(f32)backend.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = backend.framebuffer_width;
    scissor.extent.height = backend.framebuffer_height;

    vkCmdSetViewport(gfx_cmdbuf->handle, 0, 1, &viewport);
    vkCmdSetScissor(gfx_cmdbuf->handle, 0, 1, &scissor);

    backend.main_pass.render_area.width = backend.framebuffer_width;
    backend.main_pass.render_area.height = backend.framebuffer_height;
    vulkan_renderpass_begin(&backend, &backend.main_pass, gfx_cmdbuf,
                            backend.framebuffers[backend.image_index]);

    vulkan_shader_bind(&backend, &backend.basic_shader);
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(gfx_cmdbuf->handle, 0, 1, &backend.vertex_buffer.handle, offsets);
    vkCmdBindIndexBuffer(gfx_cmdbuf->handle, backend.index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(gfx_cmdbuf->handle, 6, 1, 0, 0, 0);
    return true;
}

bool vulkan_backend_end_frame(f32 dt) {
    CommandBuffer* gfx_cmdbuf = &backend.graphics_command_buffers[backend.image_index];
    vulkan_renderpass_end(&backend.main_pass, gfx_cmdbuf);
    vulkan_command_buffer_end(gfx_cmdbuf);
    if (backend.images_in_flight[backend.image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(backend.device.logical, 1, backend.images_in_flight[backend.image_index],
                        VK_TRUE, UINT64_MAX);
    }

    backend.images_in_flight[backend.image_index] =
        &backend.in_flight_fences[backend.current_frame];

    // Reset the fence so that it can be reused in the next frame
    vkResetFences(backend.device.logical, 1, &backend.in_flight_fences[backend.current_frame]);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // Each semaphore will wait on a pipeline stage to complete
    // In this case, we want to wait for the color attachment output stage
    // which means one frame will be presented at a time
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &gfx_cmdbuf->handle;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &backend.queue_complete_semaphores[backend.current_frame];
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &backend.image_available_semaphores[backend.current_frame];

    VkResult submit_result = vkQueueSubmit(backend.device.graphics_queue, 1, &submit_info,
                                           backend.in_flight_fences[backend.current_frame]);

    if (submit_result != VK_SUCCESS) {
        ERROR("Failed to submit work.");
        return false;
    }

    vulkan_command_buffer_set_submitted(gfx_cmdbuf);

    vulkan_swapchain_present(
        &backend, &backend.swapchain, backend.device.graphics_queue, backend.device.present_queue,
        backend.queue_complete_semaphores[backend.current_frame], backend.image_index);

    return true;
}

static bool recreate_swapchain(void) {
    if (backend.recreating_swapchain) {
        return false;
    }
    if (backend.framebuffer_width == 0 || backend.framebuffer_height == 0) {
        return false;
    }

    backend.recreating_swapchain = true;
    vkDeviceWaitIdle(backend.device.logical);

    vulkan_device_query_swapchain_support(backend.device.physical, backend.surface,
                                          &backend.device.swapchain_support);

    vulkan_swapchain_recreate(&backend, backend.framebuffer_width, backend.framebuffer_height,
                              &backend.swapchain);

    backend.main_pass.render_area.width = backend.framebuffer_width;
    backend.main_pass.render_area.height = backend.framebuffer_height;

    for (u32 i = 0; i < backend.swapchain.image_count; i++) {
        vulkan_command_buffer_free(&backend, backend.device.graphics_command_pool,
                                   &backend.graphics_command_buffers[i]);
    }

    for (u32 i = 0; i < backend.swapchain.image_count; i++) {
        vkDestroyFramebuffer(backend.device.logical, backend.framebuffers[i], backend.allocator);
    }

    create_framebuffers(&backend);
    allocate_command_buffers(&backend);
    backend.recreating_swapchain = false;
    backend.swapchain_needs_resize = false;
    return true;
}

// Checks the available memory types and returns the index of the first one that
// matches the filter and has all the required properties.
i32 find_memory_Type(u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(backend.device.physical, &mem_properties);
    for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return -1;
}

static void allocate_command_buffers(VulkanBackend* backend) {
    // create a command buffer per swapchain image.
    for (u32 i = 0; i < backend->swapchain.image_count; i++) {
        vulkan_command_buffer_allocate(backend, backend->device.graphics_command_pool, true,
                                       &backend->graphics_command_buffers[i]);
    }
}

static void create_framebuffers(VulkanBackend* backend) {
    // create a framebuffer per swapchain image.
    for (u32 i = 0; i < backend->swapchain.image_count; i++) {
        u32 count = 2;
        VkImageView attachments[] = {
            backend->swapchain.views[i],
            backend->swapchain.depth_image.view,
        };

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = backend->main_pass.handle;
        framebuffer_info.attachmentCount = count;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = backend->framebuffer_width;
        framebuffer_info.height = backend->framebuffer_height;
        framebuffer_info.layers = 1;

        VK_FN_CHECK(vkCreateFramebuffer(backend->device.logical, &framebuffer_info,
                                        backend->allocator, &backend->framebuffers[i]));
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* data) {
    UNUSED(severity);
    UNUSED(data);
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        TRACE(callback_data->pMessage);
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        INFO(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        WARN(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ERROR(callback_data->pMessage);
        break;
    }
    return VK_FALSE;
}

void buffer_data_transfer(VulkanBackend* ctx, VkCommandPool pool, VkFence fence, VkQueue queue,
                          Buffer* dst, u64 dst_offset, u64 size, void* data) {
    u32 memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    u32 buffer_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    Buffer stanging;
    // 1) Create a stanging buffer
    vulkan_buffer_create(ctx, buffer_flags, memory_flags, size, true, &stanging);
    // 2) Write data to the stanging buffer
    vulkan_buffer_write(ctx, &stanging, 0, size, 0, data);
    // 3) Copy the stanging buffer to the destination buffer
    vulkan_buffer_copy(ctx, pool, queue, &stanging, dst, size, 0, dst_offset);
    // 4) Destroy the stanging buffer
    vulkan_buffer_destroy(ctx, &stanging);
}

void vulkan_backend_destroy(void) {
    vkDeviceWaitIdle(backend.device.logical);
    INFO("Destroying Vulkan Vertex Buffer...");
    vulkan_buffer_destroy(&backend, &backend.vertex_buffer);
    INFO("Destroying Vulkan Index Buffer...");
    vulkan_buffer_destroy(&backend, &backend.index_buffer);
    INFO("Destroying Vulkan Shaders...");
    vulkan_shader_destroy(&backend, &backend.basic_shader);
    // Destroy sync objects
    INFO("Destroying Vulkan Sync Objects...");
    for (u32 i = 0; i < backend.swapchain.max_frames_in_flight; i++) {
        vkDestroySemaphore(backend.device.logical, backend.image_available_semaphores[i],
                           backend.allocator);
        vkDestroySemaphore(backend.device.logical, backend.queue_complete_semaphores[i],
                           backend.allocator);
        vkDestroyFence(backend.device.logical, backend.in_flight_fences[i], backend.allocator);
    }
    INFO("Destroying Vulkan Framebuffers...");
    for (u32 i = 0; i < backend.swapchain.image_count; i++) {
        vkDestroyFramebuffer(backend.device.logical, backend.framebuffers[i], backend.allocator);
    }
    // Destroy command buffers
    INFO("Destroying Vulkan Command Buffers...");
    for (u32 i = 0; i < backend.swapchain.image_count; i++) {
        vulkan_command_buffer_free(&backend, backend.device.graphics_command_pool,
                                   &backend.graphics_command_buffers[i]);
    }
    INFO("Destroying Vulkan Renderpass...");
    vulkan_renderpass_destroy(&backend, &backend.main_pass);
    INFO("Destroying Vulkan Swapchain...");
    vulkan_swapchain_destroy(&backend, &backend.swapchain);

#ifdef _DEBUG
    DEBUG("Destroying debugger...");
    if (backend.debugger) {
        PFN_vkDestroyDebugUtilsMessengerEXT fn =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                backend.instance, "vkDestroyDebugUtilsMessengerEXT");
        fn(backend.instance, backend.debugger, backend.allocator);
    }
#endif
    INFO("Destroying Vulkan Device...");
    vulkan_device_destroy(&backend);
    INFO("Destroying Vulkan Surface...");
    vkDestroySurfaceKHR(backend.instance, backend.surface, backend.allocator);
    INFO("Destroying Vulkan Instance...");
    vkDestroyInstance(backend.instance, backend.allocator);
}
