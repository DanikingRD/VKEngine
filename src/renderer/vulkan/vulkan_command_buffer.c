#include "vulkan_command_buffer.h"

void vulkan_command_buffer_allocate(
    VulkanBackend* backend, VkCommandPool pool, bool is_primary, CommandBuffer* buffer
) {
    buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    // TODO: track command buffer states
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = pool;
    alloc_info.level =
        is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 1;
    VK_FN_CHECK(vkAllocateCommandBuffers(backend->device.logical, &alloc_info, &buffer->handle));
    buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_begin(
    CommandBuffer* command_buffer, bool single_use, bool render_pass_continue, bool simultaneous_use
) {
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    if (single_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (render_pass_continue) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (simultaneous_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }
    VK_FN_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end(CommandBuffer* command_buffer) {
    VK_FN_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_FINISHED;
}

void vulkan_command_buffer_set_submitted(CommandBuffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_allocate_and_begin_single_use(
    VulkanBackend* backend, VkCommandPool pool, CommandBuffer* buffer
) {
    vulkan_command_buffer_allocate(backend, pool, true, buffer);
    vulkan_command_buffer_begin(buffer, true, false, false);
}

void vulkan_command_buffer_end_single_use(
    VulkanBackend* backend, VkCommandPool pool, CommandBuffer* out_command_buffer, VkQueue queue
) {
    vulkan_command_buffer_end(out_command_buffer);
    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &out_command_buffer->handle;
    VK_FN_CHECK(vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));
    // Wait for this queue to finish executing the commands in the command buffer.
    VK_FN_CHECK(vkQueueWaitIdle(queue));
    vulkan_command_buffer_free(backend, pool, out_command_buffer);
}
void vulkan_command_buffer_free(
    VulkanBackend* backend, VkCommandPool pool, CommandBuffer* command_buffer
) {
    vkFreeCommandBuffers(backend->device.logical, pool, 1, &command_buffer->handle);
    command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    command_buffer->handle = VK_NULL_HANDLE;
}