#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include "vulkan_types.h"

bool vulkan_device_create(VulkanBackend* backend);
void vulkan_device_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface, SwapchainSupport* out);
bool vulkan_device_detect_depth_format(Device* device);
void vulkan_device_destroy(VulkanBackend* backend);

#endif
