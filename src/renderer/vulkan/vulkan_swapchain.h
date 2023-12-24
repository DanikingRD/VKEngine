#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include "vulkan_types.h"

bool vulkan_swapchain_create(VulkanBackend* context, u32 width, u32 height, Swapchain* ptr);
void vulkan_swapchain_recreate(VulkanBackend* context, u32 width, u32 height, Swapchain* ptr);
void vulkan_swapchain_present(
    VulkanBackend* context, Swapchain* swapchain, VkQueue graphics_queue, VkQueue present_queue,
    VkSemaphore render_complete_semaphore, u32 image_present_index
);
bool vulkan_swapchain_acquire_next_image(
    VulkanBackend* backend, VkSemaphore semaphore, VkFence fence, Swapchain* swapchain, u64 timeout,
    u32* image_index
);
void vulkan_swapchain_destroy(VulkanBackend* context, Swapchain* swapchain);

#endif
