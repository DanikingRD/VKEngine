#include "vulkan_shader.h"
#include "core/mem.h"
#include "core/str.h"
#include "platform/fs.h"
#include "renderer/vulkan/vulkan_descriptor_set.h"
#include "vulkan_buffer.h"
#include "vulkan_pipeline.h"
#include <vulkan/vulkan_core.h>

#define BUILTIN_SHADER_NAME "builtin.shader"

static bool create_shader_module(VulkanBackend* backend, const char* name, const char* type,
                                 VkShaderStageFlagBits flag, u32 stage, Shader* shader) {
    char file_name[256];
    str_format(file_name, 256, "bin/assets/shaders/%s.%s.spv", name, type);

    File file;
    if (!fs_open(file_name, OPEN_FILE_MODE_READ_BINARY, &file)) {
        ERROR("Failed to open shader file: %s", file_name);
        return false;
    }
    u64 bytes_read;
    u8* shader_buffer = fs_read_all(&file, &bytes_read);

    if (!shader_buffer) {
        ERROR("Failed to read shader file: %s", file_name);
        return false;
    }

    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = bytes_read;
    create_info.pCode = (u32*)shader_buffer;
    fs_close(&file);

    ShaderModule* module = &shader->modules[stage];

    VK_FN_CHECK(vkCreateShaderModule(backend->device.logical, &create_info, backend->allocator,
                                     &module->handle));

    VkPipelineShaderStageCreateInfo stage_info = {0};
    stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info.stage = flag;
    stage_info.module = module->handle;
    stage_info.pName = "main";

    module->stage_info = stage_info;

    if (shader_buffer) {
        mem_free(shader_buffer);
        shader_buffer = 0;
    }

    DEBUG("Loaded shader module: %s, bytes read: %d", file_name, bytes_read);
    return true;
}

bool vulkan_shader_create(VulkanBackend* backend, Shader* shader) {
    VkShaderStageFlagBits shader_types[AVAILABLE_SHADER_STAGES] = {VK_SHADER_STAGE_VERTEX_BIT,
                                                                   VK_SHADER_STAGE_FRAGMENT_BIT};
    char* shader_type_names[] = {"vert", "frag"};
    for (u32 i = 0; i < AVAILABLE_SHADER_STAGES; i++) {
        if (!create_shader_module(backend, BUILTIN_SHADER_NAME, shader_type_names[i],
                                  shader_types[i], i, shader)) {
            return false;
        }
    }

    vulkan_descriptor_set_layout_create(backend, 0, &shader->descriptor_layout);

    // GlobalsUBO buffer

    Buffer* buffers = mem_alloc(sizeof(Buffer) * backend->swapchain.max_frames_in_flight);

    for (u32 i = 0; i < backend->swapchain.max_frames_in_flight; i++) {
        vulkan_buffer_create(
            backend, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            sizeof(GlobalsUBO), true, &buffers[i]);
    }

    shader->globals_buffer = buffers;

    vulkan_descriptor_set_pool_create(backend, backend->swapchain.max_frames_in_flight,
                                      &shader->descriptor_pool);
    const u32 descriptor_set_layout_count = 1;
    VkDescriptorSetLayout descriptor_set_layouts[1] = {shader->descriptor_layout};

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = backend->framebuffer_height;
    viewport.width = (f32)backend->framebuffer_width;
    viewport.height = -(f32)backend->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = (VkExtent2D){backend->framebuffer_width, backend->framebuffer_height};

    const u32 attribute_count = 1;
    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
    VkFormat formats[] = {VK_FORMAT_R32G32B32_SFLOAT};

    u32 strides[] = {sizeof(Vertex)};
    u32 offset = 0;
    for (u32 i = 0; i < attribute_count; i++) {
        VkVertexInputAttributeDescription attribute_description = {0};
        attribute_description.binding = 0;
        attribute_description.location = i;
        attribute_description.format = formats[i];
        attribute_descriptions[i] = attribute_description;
        attribute_descriptions[i].offset = offset;
        offset += strides[i];
    }

    VkPipelineShaderStageCreateInfo stage_create_infos[AVAILABLE_SHADER_STAGES];
    for (u32 i = 0; i < AVAILABLE_SHADER_STAGES; i++) {
        stage_create_infos[i] = shader->modules[i].stage_info;
    }

    vulkan_render_pipeline_create(
        backend, &backend->main_pass, attribute_count, attribute_descriptions,
        descriptor_set_layout_count, descriptor_set_layouts, AVAILABLE_SHADER_STAGES,
        stage_create_infos, viewport, scissor, false, &backend->basic_shader.pipeline);

    shader->descriptor_sets = vulkan_descriptor_set_create(backend, shader->descriptor_pool,
                                                           backend->swapchain.max_frames_in_flight);

    vulkan_descriptor_set_update(backend, shader->descriptor_sets, shader->globals_buffer,
                                 backend->swapchain.max_frames_in_flight);

    DEBUG("Vulkan Basic Pipeline created.");
    return true;
}
void vulkan_shader_bind(VulkanBackend* backend, Shader* shader) {
    u32 image_index = backend->image_index;
    vulkan_pipeline_bind(backend, backend->graphics_command_buffers[image_index],
                         &backend->basic_shader.pipeline);
}

void vulkan_shader_destroy(VulkanBackend* backend, Shader* shader) {

    vulkan_descriptor_set_destroy(backend, &shader->descriptor_layout, shader->globals_buffer,
                                  shader->descriptor_pool);

    vulkan_pipeline_destroy(backend, &backend->basic_shader.pipeline);
    for (u32 i = 0; i < AVAILABLE_SHADER_STAGES; i++) {
        ShaderModule* module = &shader->modules[i];
        vkDestroyShaderModule(backend->device.logical, module->handle, backend->allocator);
        shader->modules[i].handle = 0;
    }
    DEBUG("Vulkan Shader Modules destroyed.");
}
