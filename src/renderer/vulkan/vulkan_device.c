#include "vulkan_device.h"
#include "collections/vector.h"
#include "core/mem.h"

#include "core/str.h"

typedef struct DeviceSelectionCriteria {
    bool with_graphics_queue;
    bool with_present_queue;
    bool with_transfer_queue;
    bool with_compute_queue;
    bool with_discrete_gpu;
    Vector(const char*) device_extensions;
} DeviceSelectionCriteria;

static bool pick_physical_device(VulkanBackend* backend);
static bool create_logical_device(VulkanBackend* backend);

bool vulkan_device_create(VulkanBackend* backend) {
    DEBUG("Creating Vulkan Device...");
    if (!pick_physical_device(backend)) {
        return false;
    }
    if (!create_logical_device(backend)) {
        return false;
    }
    return true;
}

void vulkan_device_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface,
                                           SwapchainSupport* out) {
    VK_FN_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &out->capabilities));

    VK_FN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out->format_count, 0));

    if (out->format_count != 0) {
        if (!out->formats) {
            out->formats = mem_alloc(sizeof(VkSurfaceFormatKHR) * out->format_count);
        }
        VK_FN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &out->format_count,
                                                         out->formats));
    }

    VK_FN_CHECK(
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &out->present_mode_count, 0));

    if (out->present_mode_count != 0) {
        if (!out->present_modes) {
            out->present_modes = mem_alloc(sizeof(VkPresentModeKHR) * out->present_mode_count);
        }
        VK_FN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &out->present_mode_count, out->present_modes));
    }
}

bool device_selection_criteria_satisfied(DeviceSelectionCriteria* criteria,
                                         VkPhysicalDeviceProperties* properties,
                                         VkPhysicalDevice device, VkSurfaceKHR surface,
                                         DeviceQueueFamilyIndices* queue_indices,
                                         SwapchainSupport* swapchain_support) {
    if (criteria->with_discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    // Get available queue family indices

    bool graphics_family = false;
    bool present_family = false;
    bool transfer_family = false;
    bool compute_family = false;

    for (u32 i = 0; i < queue_family_count; i++) {
        VkQueueFamilyProperties queue_family_props = queue_families[i];

        if (queue_family_props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            // Do not switch your family index if you already found one
            if (!graphics_family) {
                queue_indices->graphics_family = i;
                graphics_family = true;
            }
        }
        if (queue_family_props.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            if (!compute_family) {
                queue_indices->compute_family = i;
                compute_family = true;
            }
        }
        if (queue_family_props.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (!transfer_family) {
                queue_indices->transfer_family = i;
                transfer_family = true;
            }
        }
        if (!present_family) {
            VkBool32 supports_present_queue = false;
            VK_FN_CHECK(
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present_queue));
            if (supports_present_queue) {
                queue_indices->present_family = i;
                present_family = true;
            }
        }
    }
    if (criteria->with_graphics_queue && !graphics_family) {
        return false;
    }
    if (criteria->with_present_queue && !present_family) {
        return false;
    }
    if (criteria->with_transfer_queue && !transfer_family) {
        return false;
    }
    if (criteria->with_compute_queue && !compute_family) {
        return false;
    }

    vulkan_device_query_swapchain_support(device, surface, swapchain_support);

    if (swapchain_support->format_count <= 0 || swapchain_support->present_mode_count <= 0) {
        if (swapchain_support->formats) {
            mem_free(swapchain_support->formats);
        }
        if (swapchain_support->present_modes) {
            mem_free(swapchain_support->present_modes);
        }
        DEBUG("Swapchain support does not meet requirements");
        return false;
    }

    if (criteria->device_extensions) {
        u32 available_extension_count = 0;
        VK_FN_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, 0));

        if (available_extension_count != 0) {
            VkExtensionProperties available_extensions[available_extension_count];
            VK_FN_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count,
                                                             available_extensions));

            for (u32 i = 0; i < vector_length(criteria->device_extensions); i++) {
                bool found = false;
                const char* required_extension = criteria->device_extensions[i];
                for (u32 j = 0; j < available_extension_count; j++) {
                    if (str_equals(required_extension, available_extensions[j].extensionName)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    DEBUG("Missing Vulkan Device Extension: %s", required_extension);
                    return false;
                }
            }
        }
    }

    DEBUG("Queue Family Indices:");

    if (graphics_family) {
        DEBUG(" - Graphics: %d", queue_indices->graphics_family);
    } else {
        DEBUG(" - Graphics: not available");
    }
    if (present_family) {
        DEBUG(" - Present: %d", queue_indices->present_family);
    } else {
        DEBUG(" - Present: not available");
    }
    if (transfer_family) {
        DEBUG(" - Transfer: %d", queue_indices->transfer_family);
    } else {
        DEBUG(" - Transfer: not available");
    }
    if (compute_family) {
        DEBUG(" - Compute: %d", queue_indices->compute_family);
    } else {
        DEBUG(" - Compute: not available");
    }

    return true;
}

static bool pick_physical_device(VulkanBackend* backend) {
    u32 device_count = 0;
    VK_FN_CHECK(vkEnumeratePhysicalDevices(backend->instance, &device_count, 0));

    if (device_count == 0) {
        ERROR("Failed to find GPUs with Vulkan support!");
        return false;
    }

    VkPhysicalDevice candidates[device_count];
    VK_FN_CHECK(vkEnumeratePhysicalDevices(backend->instance, &device_count, candidates));

    // TODO: make this more configurable
    DeviceSelectionCriteria criteria = {0};
    criteria.with_graphics_queue = true;
    criteria.with_present_queue = true;
    criteria.with_transfer_queue = true;
    criteria.with_compute_queue = false;
    criteria.with_discrete_gpu = false;
    // TODO: is a vector necessary here?. Most likely we will only have a fixed set of extensions
    // known at compile time.
    criteria.device_extensions = vector_new(const char*);
    vector_push(criteria.device_extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    for (u32 i = 0; i < device_count; i++) {
        const VkPhysicalDevice candidate = candidates[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(candidate, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(candidate, &features);

        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(candidate, &memory_properties);

        DeviceQueueFamilyIndices queue_indices = {0};
        SwapchainSupport swapchain_support = {0};

        INFO("Evaluating device %s", properties.deviceName);

        if (!device_selection_criteria_satisfied(&criteria, &properties, candidate,
                                                 backend->surface, &queue_indices,
                                                 &swapchain_support)) {
            DEBUG("Device selection criteria was not satifisfied. Skipping device %s",
                  properties.deviceName);
        }

        INFO("Device %s was picked.", properties.deviceName);

        switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            INFO("Device Type: Unknown");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            INFO("Device Type: Integrated GPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            INFO("Device Type: Discrete GPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            INFO("Device Type: Virtual GPU");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            INFO("Device Type: CPU");
            break;
        }

        INFO("GPU Driver Version: %d.%d.%d", VK_VERSION_MAJOR(properties.driverVersion),
             VK_VERSION_MINOR(properties.driverVersion),
             VK_VERSION_PATCH(properties.driverVersion));

        INFO("API Version: %d.%d.%d", VK_VERSION_MAJOR(properties.apiVersion),
             VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
        f32 total_dedicated_mem = 0.0f;
        f32 total_shared_mem = 0.0f;
        for (u32 i = 0; i < memory_properties.memoryHeapCount; i++) {
            VkMemoryHeap heap = memory_properties.memoryHeaps[i];
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                total_dedicated_mem += heap.size;
            } else {
                total_shared_mem += heap.size;
            }
        }
        INFO("Total Dedicated Memory: %.2f MB", total_dedicated_mem / 1024.0f / 1024.0f);
        INFO("Total Shared Memory: %.2f MB", total_shared_mem / 1024.0f / 1024.0f);
        backend->device.physical = candidate;
        backend->device.queue_family_indices = queue_indices;
        backend->device.swapchain_support = swapchain_support;

        backend->device.properties = properties;
        backend->device.features = features;
        backend->device.memory_properties = memory_properties;
        break;
    }

    vector_free(criteria.device_extensions);

    // ensure a device was found
    if (backend->device.physical == VK_NULL_HANDLE) {
        ERROR("Failed to find a suitable GPU!");
        return false;
    }

    return true;
}

static bool create_logical_device(VulkanBackend* backend) {

    bool present_share_graphics_queue = backend->device.queue_family_indices.graphics_family ==
                                        backend->device.queue_family_indices.present_family;
    bool transfer_share_graphics_queue = backend->device.queue_family_indices.graphics_family ==
                                         backend->device.queue_family_indices.transfer_family;
    u32 family_count = 1;

    if (!present_share_graphics_queue) {
        family_count++;
    }

    if (!transfer_share_graphics_queue) {
        family_count++;
    }

    u32 family_indices[family_count];
    u32 i = 0;

    family_indices[i++] = backend->device.queue_family_indices.graphics_family;

    if (!present_share_graphics_queue) {
        family_indices[i++] = backend->device.queue_family_indices.present_family;
    }

    if (!transfer_share_graphics_queue) {
        family_indices[i++] = backend->device.queue_family_indices.transfer_family;
    }

    VkDeviceQueueCreateInfo queue_create_infos[family_count];

    for (u32 i = 0; i < family_count; i++) {
        VkDeviceQueueCreateInfo info = {0};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family_indices[i];
        info.queueCount = 1;
        f32 queue_priority = 1.0f;
        info.pQueuePriorities = &queue_priority;
        queue_create_infos[i] = info;
    }

    bool portability_required = false;
    u32 available_device_extensions = 0;
    VK_FN_CHECK(vkEnumerateDeviceExtensionProperties(backend->device.physical, 0,
                                                     &available_device_extensions, 0));
    if (available_device_extensions != 0) {
        VkExtensionProperties extension_properties[available_device_extensions];
        VK_FN_CHECK(vkEnumerateDeviceExtensionProperties(
            backend->device.physical, 0, &available_device_extensions, extension_properties));
        for (u32 i = 0; i < available_device_extensions; i++) {
            VkExtensionProperties prop = extension_properties[i];
            if (str_equals(prop.extensionName, "VK_KHR_portability_subset")) {
                portability_required = true;
                DEBUG("Required Device extension: %s", "VK_KHR_portability_subset");
                break;
            }
        }
    }

    const char* extensions[4] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    u32 c = 0;
    extensions[c++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    if (portability_required)
        extensions[c++] = "VK_KHR_portability_subset";

    VkPhysicalDeviceFeatures features = {0};
    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = family_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.enabledExtensionCount = c;
    device_create_info.ppEnabledExtensionNames = extensions;
    device_create_info.pEnabledFeatures = &features;

    VK_FN_CHECK(vkCreateDevice(backend->device.physical, &device_create_info, backend->allocator,
                               &backend->device.logical));
    // Get Queues
    vkGetDeviceQueue(backend->device.logical, backend->device.queue_family_indices.graphics_family,
                     0, &backend->device.graphics_queue);
    vkGetDeviceQueue(backend->device.logical, backend->device.queue_family_indices.present_family,
                     0, &backend->device.present_queue);
    vkGetDeviceQueue(backend->device.logical, backend->device.queue_family_indices.transfer_family,
                     0, &backend->device.transfer_queue);

    if (backend->device.graphics_queue && backend->device.present_queue &&
        backend->device.transfer_queue) {
        DEBUG("Graphics, Present, Transfer queues ready");
    } else {
        DEBUG("Failed to obtain queues.");
    }
    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = backend->device.queue_family_indices.graphics_family;
    // allows resetting individual command buffers
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_FN_CHECK(vkCreateCommandPool(backend->device.logical, &pool_info, backend->allocator,
                                    &backend->device.graphics_command_pool));
    DEBUG("Graphics Command Pool created");
    return true;
}

bool vulkan_device_detect_depth_format(Device* device) {
    const u32 c = 3;
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    VkFormatFeatureFlags flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u32 i = 0; i < c; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physical, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }
    }
    return false;
}

void vulkan_device_destroy(VulkanBackend* backend) {

    DEBUG("Destroying Vulkan Command Pools...");
    if (backend->device.graphics_command_pool) {
        vkDestroyCommandPool(backend->device.logical, backend->device.graphics_command_pool,
                             backend->allocator);
    }
    DEBUG("Destroying Vulkan Device...");
    backend->device.graphics_queue = VK_NULL_HANDLE;
    backend->device.present_queue = VK_NULL_HANDLE;
    backend->device.transfer_queue = VK_NULL_HANDLE;
    if (backend->device.logical) {
        vkDestroyDevice(backend->device.logical, backend->allocator);
    }
    backend->device.physical = VK_NULL_HANDLE;
    // TODO: destroy logical device
    if (backend->device.swapchain_support.formats) {
        mem_free(backend->device.swapchain_support.formats);
        backend->device.swapchain_support.formats = 0;
        backend->device.swapchain_support.format_count = 0;
    }
    if (backend->device.swapchain_support.present_modes) {
        mem_free(backend->device.swapchain_support.present_modes);
        backend->device.swapchain_support.present_modes = 0;
        backend->device.swapchain_support.present_mode_count = 0;
    }
}
