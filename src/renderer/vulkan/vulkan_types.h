#ifndef VULKAN_TYPES
#define VULKAN_TYPES

#include "core/log.h"
#include "vulkan/vulkan.h"

typedef struct SwapchainSupport {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} SwapchainSupport;

typedef struct DeviceQueueFamilyIndices {
    u32 graphics_family;
    u32 present_family;
    u32 transfer_family;
    u32 compute_family;
} DeviceQueueFamilyIndices;

typedef struct Device {
    VkPhysicalDevice physical;
    VkDevice logical;

    SwapchainSupport swapchain_support;
    DeviceQueueFamilyIndices queue_family_indices;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    VkFormat depth_format;

} Device;

typedef struct VulkanBackend {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkAllocationCallbacks* allocator;
    Device device;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugger;
#endif
} VulkanBackend;

#define VK_FN_CHECK(fn)                                                                                                \
    {                                                                                                                  \
        VkResult result = fn;                                                                                          \
        if (result != VK_SUCCESS) {                                                                                    \
            ERROR("Vulkan Function Error: %d", result);                                                                \
        }                                                                                                              \
    }

#endif
