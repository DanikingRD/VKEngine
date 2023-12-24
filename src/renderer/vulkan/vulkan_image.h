#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include "vulkan_types.h"

void vulkan_image_create(
    VulkanBackend* backend, VkImageType type, u32 width, u32 height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags memory_flags, bool create_view, VkImageAspectFlags aspect_flags,
    Image* image
);

void vulkan_image_create_view(VulkanBackend* backend, VkFormat format, VkImageAspectFlags aspect_flags, Image* image);

void vulkan_image_destroy(VulkanBackend* backend, Image* image);
#endif
