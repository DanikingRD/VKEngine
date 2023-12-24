#include "vulkan_swapchain.h"
#include "defines.h"
#include "vulkan_device.h"
#include "vulkan_image.h"
#include <stdlib.h>

static void create(VulkanBackend* backend, u32 w, u32 h, Swapchain* out);
static void destroy(VulkanBackend* backend, Swapchain* out);

bool vulkan_swapchain_create(VulkanBackend* context, u32 width, u32 height, Swapchain* ptr) {
    create(context, width, height, ptr);
    return true;
}
void vulkan_swapchain_recreate(VulkanBackend* context, u32 width, u32 height, Swapchain* ptr) {
    destroy(context, ptr);
    create(context, width, height, ptr);
}

void vulkan_swapchain_present(
    VulkanBackend* context, Swapchain* swapchain, VkQueue graphics_queue, VkQueue present_queue,
    VkSemaphore render_complete_semaphore, u32 image_present_index
) {
    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain->handle;
    present_info.pImageIndices = &image_present_index;

    VkResult res = vkQueuePresentKHR(present_queue, &present_info);
    switch (res) {
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        vulkan_swapchain_recreate(
            context, context->framebuffer_width, context->framebuffer_height, swapchain
        );
        break;
    case VK_SUCCESS:
        break;
    default:
        ERROR("Failed to present swapchain");
        break;
    }
    context->current_frame = (context->current_frame + 1) % swapchain->max_frames_in_flight;
}

static void create(VulkanBackend* backend, u32 w, u32 h, Swapchain* out) {
    bool preferred_image_format_found = false;
    SwapchainSupport* support = &backend->device.swapchain_support;
    out->max_frames_in_flight = 2;
    for (u32 i = 0; i < support->format_count; i++) {
        VkSurfaceFormatKHR format = support->formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            out->format = format;
            preferred_image_format_found = true;
            break;
        }
    }

    if (!preferred_image_format_found) {
        out->format = support->formats[0];
    }

    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < support->present_mode_count; i++) {
        VkPresentModeKHR supported_mode = support->present_modes[i];
        // Break if we found a better mode
        if (supported_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            mode = supported_mode;
            break;
        }
    }
    // Requery swapchain support
    vulkan_device_query_swapchain_support(backend->device.physical, backend->surface, support);

    VkExtent2D swapchain_extent = {w, h};

    if (support->capabilities.currentExtent.width != UINT32_MAX) {
        // Override the current swapchain extent with something that is valid.
        swapchain_extent = support->capabilities.currentExtent;
    }
    // Clamp to a supported range
    VkExtent2D min = support->capabilities.minImageExtent;
    VkExtent2D max = support->capabilities.maxImageExtent;
    swapchain_extent.width = CLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = CLAMP(swapchain_extent.height, min.height, max.height);

    u32 image_count = support->capabilities.minImageCount + 1;
    // We ignore zero because it means there is no limit.
    // However if it is not zero and image_count is greater than the max, we clamp it.
    if (support->capabilities.maxImageCount > 0 &&
        image_count > support->capabilities.maxImageCount) {
        image_count = support->capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapchain_create_info = {0};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.surface = backend->surface;
    swapchain_create_info.imageFormat = out->format.format;
    swapchain_create_info.imageColorSpace = out->format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue family indices
    if (backend->device.queue_family_indices.graphics_family !=
        backend->device.queue_family_indices.present_family) {
        u32 queue_family_indices[] = {
            backend->device.queue_family_indices.graphics_family,
            backend->device.queue_family_indices.present_family
        };
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = 0;
    }

    swapchain_create_info.preTransform =
        backend->device.swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = 0;
    VK_FN_CHECK(vkCreateSwapchainKHR(
        backend->device.logical, &swapchain_create_info, backend->allocator, &out->handle
    ));
    // Get images
    VK_FN_CHECK(vkGetSwapchainImagesKHR(backend->device.logical, out->handle, &out->image_count, 0)
    );
    if (!out->images) {
        out->images = malloc(sizeof(VkImage) * out->image_count);
    }
    if (!out->views) {
        out->views = malloc(sizeof(VkImageView) * out->image_count);
    }

    VK_FN_CHECK(vkGetSwapchainImagesKHR(
        backend->device.logical, out->handle, &out->image_count, out->images
    ));

    // Create an image view for each image.
    for (u32 i = 0; i < out->image_count; i++) {
        VkImageViewCreateInfo view_info = {0};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = out->format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.layerCount = 1;
        view_info.image = out->images[i];
        VK_FN_CHECK(vkCreateImageView(
            backend->device.logical, &view_info, backend->allocator, &out->views[i]
        ));
    }

    if (!vulkan_device_detect_depth_format(&backend->device)) {
        backend->device.depth_format = VK_FORMAT_UNDEFINED;
        ERROR("Failed to find a supported Depth format for the current device.");
    }

    vulkan_image_create(
        backend, VK_IMAGE_TYPE_2D, swapchain_extent.width, swapchain_extent.height,
        backend->device.depth_format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true,
        VK_IMAGE_ASPECT_DEPTH_BIT, &out->depth_image
    );
}

bool vulkan_swapchain_acquire_next_image(
    VulkanBackend* backend, VkSemaphore semaphore, VkFence fence, Swapchain* swapchain, u64 timeout,
    u32* image_index
) {
    VkResult result = vkAcquireNextImageKHR(
        backend->device.logical, swapchain->handle, timeout, semaphore, fence, image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        WARN("Swapchain out of date. Recreating...");
        vulkan_swapchain_recreate(
            backend, backend->framebuffer_width, backend->framebuffer_height, swapchain
        );
        return false;
    }
    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        return true;
    }
    ERROR("Failed to acquire next swapchain image");
    return false;
}

void vulkan_swapchain_destroy(VulkanBackend* context, Swapchain* swapchain) {
    destroy(context, swapchain);
}

static void destroy(VulkanBackend* backend, Swapchain* out) {
    vulkan_image_destroy(backend, &out->depth_image);
    // The swapchain takes care of destroying the images.
    // however we need to destroy the image views.
    for (u32 i = 0; i < out->image_count; i++) {
        vkDestroyImageView(backend->device.logical, out->views[i], backend->allocator);
    }

    if (out->images) {
        free(out->images);
        out->images = 0;
    }

    if (out->views) {
        free(out->views);
        out->views = 0;
    }

    vkDestroySwapchainKHR(backend->device.logical, out->handle, backend->allocator);
    out->handle = 0;

    DEBUG("Swapchain destroyed");
}
