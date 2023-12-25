#include "vulkan_buffer.h"
#include "core/mem.h"
#include "vulkan_command_buffer.h"

void vulkan_buffer_create(VulkanBackend* backend, VkBufferUsageFlags usages, u32 memory_flags,
                          u64 size, bool bind_on_create, Buffer* buffer) {
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usages;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_FN_CHECK(
        vkCreateBuffer(backend->device.logical, &buffer_info, backend->allocator, &buffer->handle));

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(backend->device.logical, buffer->handle, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex =
        backend->find_memory_type(mem_requirements.memoryTypeBits, memory_flags);

    VK_FN_CHECK(vkAllocateMemory(backend->device.logical, &alloc_info, backend->allocator,
                                 &buffer->memory));

    if (bind_on_create) {
        vulkan_buffer_bind(backend, buffer, 0);
    }
}
void vulkan_buffer_write(VulkanBackend* context, Buffer* buffer, u64 offset, u64 size, u32 flags,
                         void* data) {
    void* unlocked_region;
    VK_FN_CHECK(vkMapMemory(context->device.logical, buffer->memory, offset, size, flags,
                            &unlocked_region));
    mem_copy(unlocked_region, data, size);
    vkUnmapMemory(context->device.logical, buffer->memory);
}

void vulkan_buffer_bind(VulkanBackend* context, Buffer* buffer, u64 offset) {
    VK_FN_CHECK(
        vkBindBufferMemory(context->device.logical, buffer->handle, buffer->memory, offset));
}

void vulkan_buffer_copy(VulkanBackend* context, VkCommandPool pool, VkQueue queue, Buffer* src,
                        Buffer* dst, u64 size, u64 src_offset, u64 dst_offset) {
    CommandBuffer command_buffer;
    vulkan_command_buffer_allocate_and_begin_single_use(context, pool, &command_buffer);
    VkBufferCopy copy = {0};
    copy.srcOffset = src_offset;
    copy.dstOffset = dst_offset;
    copy.size = size;
    vkCmdCopyBuffer(command_buffer.handle, src->handle, dst->handle, 1, &copy);
    vulkan_command_buffer_end_single_use(context, pool, &command_buffer, queue);
}

void vulkan_buffer_destroy(VulkanBackend* context, Buffer* buffer) {
    if (buffer->memory) {
        vkFreeMemory(context->device.logical, buffer->memory, context->allocator);
        buffer->memory = 0;
    }

    if (buffer->handle) {
        vkDestroyBuffer(context->device.logical, buffer->handle, context->allocator);
        buffer->handle = 0;
    }
}