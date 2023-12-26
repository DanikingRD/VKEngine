#ifndef VULKAN_DESCRIPTOR_SET_H
#define VULKAN_DESCRIPTOR_SET_H

#include "vulkan_types.h"

void vulkan_descriptor_set_layout_create(VulkanBackend* backend, u32 binding,
                                         VkDescriptorSetLayout* layout);

void vulkan_descriptor_set_pool_create(VulkanBackend* backend, u32 max_set, VkDescriptorPool* out);

VkDescriptorSet* vulkan_descriptor_set_create(VulkanBackend* backend, VkDescriptorPool pool,
                                              u32 set_count);

void vulkan_descriptor_set_update(VulkanBackend* context, VkDescriptorSet* sets,
                                  Buffer* globals_buffer, u32 set_count);

void vulkan_descriptor_set_destroy(VulkanBackend* context, VkDescriptorSetLayout* layout,
                                   Buffer* global_buffers, VkDescriptorPool pool);

#endif
