#include "vulkan_descriptor_set.h"
#include "core/log.h"
#include "core/mem.h"
#include "vulkan_buffer.h"

void vulkan_descriptor_set_layout_create(VulkanBackend* backend, u32 binding,
                                         VkDescriptorSetLayout* layout) {

    // descriptors
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {0};
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorCount = 1;
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {0};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
    VK_FN_CHECK(vkCreateDescriptorSetLayout(
        backend->device.logical, &descriptor_set_layout_create_info, backend->allocator, layout));
}

void vulkan_descriptor_set_pool_create(VulkanBackend* backend, u32 maxSet, VkDescriptorPool* out) {
    VkDescriptorPoolSize descriptor_pool_size = {0};
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_pool_size.descriptorCount = maxSet;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {0};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.poolSizeCount = 1; // number of PoolSize objects
    descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
    descriptor_pool_create_info.maxSets = maxSet;

    VK_FN_CHECK(vkCreateDescriptorPool(backend->device.logical, &descriptor_pool_create_info,
                                       backend->allocator, out));
}

VkDescriptorSet* vulkan_descriptor_set_create(VulkanBackend* backend, VkDescriptorPool pool,
                                              u32 set_count) {
    VkDescriptorSetLayout* layouts = mem_alloc(sizeof(VkDescriptorSetLayout) * set_count);

    for (int i = 0; i < set_count; i++) {
        layouts[i] = backend->basic_shader.descriptor_layout;
    }

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool;
    alloc_info.descriptorSetCount = set_count;
    alloc_info.pSetLayouts = layouts;

    VkDescriptorSet* sets = mem_alloc(sizeof(VkDescriptorSet) * set_count);
    VK_FN_CHECK(vkAllocateDescriptorSets(backend->device.logical, &alloc_info, sets));

    mem_free(layouts);
    return sets;
}

void vulkan_descriptor_set_update(VulkanBackend* context, VkDescriptorSet* sets,
                                  Buffer* globals_buffer, u32 set_count) {

    for (u32 i = 0; i < set_count; i++) {
        VkDescriptorBufferInfo buffer_info = {0};
        buffer_info.buffer = globals_buffer[i].handle;
        buffer_info.offset = 0;
        buffer_info.range = sizeof(GlobalsUBO);

        VkWriteDescriptorSet write = {0};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = sets[i];
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &buffer_info;
        vkUpdateDescriptorSets(context->device.logical, 1, &write, 0, 0);
    }
}

void vulkan_descriptor_set_destroy(VulkanBackend* context, VkDescriptorSetLayout* layout,
                                   Buffer* global_buffers, VkDescriptorPool pool) {
    DEBUG("Destroying descriptor pool");
    // Note: destroying the pool will also destroy the descriptor sets
    vkDestroyDescriptorPool(context->device.logical, pool, NULL);
    if (global_buffers) {
        DEBUG("Destroying global buffers")

        for (u32 i = 0; i < context->swapchain.max_frames_in_flight; i++) {
            vulkan_buffer_destroy(context, &global_buffers[i]);
        }
        mem_free(global_buffers);
        global_buffers = 0;
    }
    DEBUG("Destroying descriptor set layout")
    vkDestroyDescriptorSetLayout(context->device.logical, *layout, NULL);
}
