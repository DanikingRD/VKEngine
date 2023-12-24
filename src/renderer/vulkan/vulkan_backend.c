#include "vulkan_backend.h"
#include "collections/vector.h"
#include "core/log.h"
#include "core/str.h"
#include "defines.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_renderpass.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.h"
#include "window.h"

VkBool32 vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* data
);

i32 find_memory_Type(u32 type_filter, VkMemoryPropertyFlags properties);
static void allocate_command_buffers(VulkanBackend* backend);
static void create_framebuffers(VulkanBackend* backend);

static VulkanBackend backend;

bool vulkan_backend_create(const char* app_name, Window* window) {

    backend.find_memory_type = find_memory_Type;

    window_get_framebuffer_size(window, &backend.framebuffer_width, &backend.framebuffer_height);

    INFO(
        "Current framebuffer (width, height): (%d, %d)", backend.framebuffer_width,
        backend.framebuffer_height
    );

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
    PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT
    )vkGetInstanceProcAddr(backend.instance, "vkCreateDebugUtilsMessengerEXT");

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
    if (!vulkan_swapchain_create(
            &backend, backend.framebuffer_width, backend.framebuffer_height, &backend.swapchain
        )) {
        ERROR("Failed to create Vulkan Swapchain");
        return false;
    }
    DEBUG("Vulkan Swapchain created");
    RenderArea area = {
        .x = 0,
        .y = 0,
        .width = backend.framebuffer_width,
        .height = backend.framebuffer_height,
    };
    Color clear_color = {
        .r = 0.0f,
        .g = 0.0f,
        .b = 0.25f,
        .a = 1.0f,
    };
    vulkan_renderpass_create(&backend, area, clear_color, 1.0f, 0.0f, &backend.main_pass);
    DEBUG("Vulkan Main Renderpass created");
    allocate_command_buffers(&backend);
    DEBUG("Vulkan Command Buffers allocated");
    backend.framebuffers = vector_with_capacity(VkFramebuffer, backend.swapchain.image_count);
    create_framebuffers(&backend);
    DEBUG("Vulkan Framebuffers created");

    // Create sync objects
    backend.image_available_semaphores =
        vector_with_capacity(VkSemaphore, backend.swapchain.max_frames_in_flight);
    backend.queue_complete_semaphores =
        vector_with_capacity(VkSemaphore, backend.swapchain.max_frames_in_flight);
    backend.in_flight_fences =
        vector_with_capacity(VkFence, backend.swapchain.max_frames_in_flight);

    for (u32 i = 0; i < backend.swapchain.max_frames_in_flight; i++) {
        VkSemaphoreCreateInfo semaphore_info = {0};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_FN_CHECK(vkCreateSemaphore(
            backend.device.logical, &semaphore_info, backend.allocator,
            &backend.image_available_semaphores[i]
        ));
        VK_FN_CHECK(vkCreateSemaphore(
            backend.device.logical, &semaphore_info, backend.allocator,
            &backend.queue_complete_semaphores[i]
        ));

        VkFenceCreateInfo fence_info = {0};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_FN_CHECK(vkCreateFence(
            backend.device.logical, &fence_info, backend.allocator, &backend.in_flight_fences[i]
        ));
    }

    return true;
}

void vulkan_backend_resize(u16 width, u16 height) {}
bool vulkan_backend_begin_frame(f32 dt) { return true; }
bool vulkan_backend_end_frame(f32 dt) { return true; }

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
    if (!backend->graphics_command_buffers) {
        backend->graphics_command_buffers =
            vector_with_capacity(CommandBuffer, backend->swapchain.image_count);
    }
    // create a command buffer per swapchain image.
    for (u32 i = 0; i < backend->swapchain.image_count; i++) {
        vulkan_command_buffer_allocate(
            backend, backend->device.graphics_command_pool, true,
            &backend->graphics_command_buffers[i]
        );
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

        VK_FN_CHECK(vkCreateFramebuffer(
            backend->device.logical, &framebuffer_info, backend->allocator,
            &backend->framebuffers[i]
        ));
    }
}
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* data
) {
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

void vulkan_backend_destroy(void) {
    vkDeviceWaitIdle(backend.device.logical);
    // Destroy sync objects
    INFO("Destroying Vulkan Sync Objects...");
    for (u32 i = 0; i < backend.swapchain.max_frames_in_flight; i++) {
        if (backend.image_available_semaphores[i]) {
            vkDestroySemaphore(
                backend.device.logical, backend.image_available_semaphores[i], backend.allocator
            );
        }
        if (backend.queue_complete_semaphores[i]) {
            vkDestroySemaphore(
                backend.device.logical, backend.queue_complete_semaphores[i], backend.allocator
            );
        }
        if (backend.in_flight_fences[i]) {
            vkDestroyFence(backend.device.logical, backend.in_flight_fences[i], backend.allocator);
        }
    }
    vector_free(backend.image_available_semaphores);
    vector_free(backend.queue_complete_semaphores);
    vector_free(backend.in_flight_fences);

    INFO("Destroying Vulkan Framebuffers...");
    for (u32 i = 0; i < backend.swapchain.image_count; i++) {
        vkDestroyFramebuffer(backend.device.logical, backend.framebuffers[i], backend.allocator);
    }
    // Destroy command buffers
    INFO("Destroying Vulkan Command Buffers...");
    for (u32 i = 0; i < backend.swapchain.image_count; i++) {
        if (backend.graphics_command_buffers[i].handle) {
            vulkan_command_buffer_free(
                &backend, backend.device.graphics_command_pool, &backend.graphics_command_buffers[i]
            );
        }
    }
    vector_free(backend.graphics_command_buffers);
    INFO("Destroying Vulkan Renderpass...");
    vulkan_renderpass_destroy(&backend, &backend.main_pass);
    INFO("Destroying Vulkan Swapchain...");
    vulkan_swapchain_destroy(&backend, &backend.swapchain);

#ifdef _DEBUG
    DEBUG("Destroying debugger...");
    if (backend.debugger) {
        PFN_vkDestroyDebugUtilsMessengerEXT fn = (PFN_vkDestroyDebugUtilsMessengerEXT
        )vkGetInstanceProcAddr(backend.instance, "vkDestroyDebugUtilsMessengerEXT");
        fn(backend.instance, backend.debugger, backend.allocator);
    }
#endif

    INFO("Destroying Vulkan Device...");

    vulkan_device_destroy(&backend);

    INFO("Destroying Vulkan Surface...");
    vkDestroySurfaceKHR(backend.instance, backend.surface, backend.allocator);

    INFO("Destroying Vulkan Instance...");
    vkDestroyInstance(backend.instance, 0);
}
