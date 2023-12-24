#include "vulkan_image.h"
#include "renderer/vulkan/vulkan_types.h"

void vulkan_image_create(VulkanBackend* backend, VkImageType type, u32 width, u32 height,
                         VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags memory_flags, bool create_view,
                         VkImageAspectFlags aspect_flags, Image* image) {
    image->width = width;
    image->height = height;

    VkImageCreateInfo image_create_info = {0};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = type;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = format;
    image_create_info.tiling = tiling;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_FN_CHECK(vkCreateImage(backend->device.logical, &image_create_info, backend->allocator,
                              &image->handle));

    // Get image memory requirements
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(backend->device.logical, image->handle, &memory_requirements);
    i32 memory_index = backend->find_memory_type(memory_requirements.memoryTypeBits, memory_flags);

    if (memory_index == -1) {
        ERROR("Failed to find suitable memory type for image.");
    }

    VkMemoryAllocateInfo memory_allocate_info = {0};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_index;

    VK_FN_CHECK(vkAllocateMemory(backend->device.logical, &memory_allocate_info, backend->allocator,
                                 &image->memory));

    VK_FN_CHECK(vkBindImageMemory(backend->device.logical, image->handle, image->memory, 0));

    if (create_view) {
        vulkan_image_create_view(backend, format, aspect_flags, image);
    }
}

void vulkan_image_create_view(VulkanBackend* backend, VkFormat format,
                              VkImageAspectFlags aspect_flags, Image* image) {
    VkImageViewCreateInfo view_info = {0};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    view_info.image = image->handle;
    VK_FN_CHECK(
        vkCreateImageView(backend->device.logical, &view_info, backend->allocator, &image->view));
}

void vulkan_image_destroy(VulkanBackend* backend, Image* image) {
    if (image->view) {
        vkDestroyImageView(backend->device.logical, image->view, backend->allocator);
        image->view = 0;
    }
    if (image->memory) {
        vkFreeMemory(backend->device.logical, image->memory, backend->allocator);
        image->memory = 0;
    }
    if (image->handle) {
        vkDestroyImage(backend->device.logical, image->handle, backend->allocator);
        image->handle = 0;
    }
}
