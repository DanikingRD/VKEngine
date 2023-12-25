#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include "vulkan_types.h"

void vulkan_buffer_create(VulkanBackend* context, VkBufferUsageFlags usages, u32 memory_flags,
                          u64 size, bool bind_on_create, Buffer* buffer);

void vulkan_buffer_bind(VulkanBackend* context, Buffer* buffer, u64 offset);

void vulkan_buffer_write(VulkanBackend* context, Buffer* buffer, u64 offset, u64 size, u32 flags,
                         void* data);

void vulkan_buffer_copy(VulkanBackend* context, VkCommandPool pool, VkQueue queue, Buffer* src,
                        Buffer* dst, u64 size, u64 src_offset, u64 dst_offset);

void vulkan_buffer_destroy(VulkanBackend* context, Buffer* buffer);

#endif
