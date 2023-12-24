#ifndef VULKAN_TYPES
#define VULKAN_TYPES

#include "collections/vector.h"
#include "core/log.h"
#include "vulkan/vulkan.h"

typedef enum CommandBufferState {
    // Initial state
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
    // Allocated and ready to be recorded
    COMMAND_BUFFER_STATE_READY,
    // Recording, now we can issue commands
    COMMAND_BUFFER_STATE_RECORDING,
    // Same as recording, but inside a render pass
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    // Recording finished, ready to be submitted
    COMMAND_BUFFER_STATE_RECORDING_FINISHED,
    // Submitted, waiting for GPU to finish
    COMMAND_BUFFER_STATE_SUBMITTED,
} CommandBufferState;

typedef struct CommandBuffer {
    VkCommandBuffer handle;
    CommandBufferState state;
} CommandBuffer;

typedef struct Color {
    f32 r;
    f32 g;
    f32 b;
    f32 a;
} Color;

typedef struct RenderArea {
    f32 x;
    f32 y;
    f32 width;
    f32 height;
} RenderArea;

typedef struct RenderPass {
    VkRenderPass handle;
    f32 depth;
    f32 stencil;
    Color clear_color;
    RenderArea render_area;
} RenderPass;

typedef struct Image {
    VkImage handle;
    VkImageView view;
    // Allocated memory for the image
    VkDeviceMemory memory;
    u32 width;
    u32 height;
} Image;

typedef struct Swapchain {
    VkSurfaceFormatKHR format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    VkImage* images;
    VkImageView* views;
    u32 image_count;
    Image depth_image;
} Swapchain;

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
    VkCommandPool graphics_command_pool;
} Device;

typedef struct VulkanBackend {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkAllocationCallbacks* allocator;
    Device device;
    Swapchain swapchain;
    RenderPass main_pass;

    Vector(CommandBuffer) graphics_command_buffers;
    Vector(VkFramebuffer) framebuffers;

    Vector(VkSemaphore) image_available_semaphores;
    Vector(VkSemaphore) queue_complete_semaphores;
    Vector(VkFence) in_flight_fences;
    u32 in_flight_fence_count;

    u32 framebuffer_width;
    u32 framebuffer_height;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugger;
#endif
    i32 (*find_memory_type)(u32 type_filter, VkMemoryPropertyFlags properties);

} VulkanBackend;

#define VK_FN_CHECK(fn)                                                                            \
    {                                                                                              \
        VkResult result = fn;                                                                      \
        if (result != VK_SUCCESS) {                                                                \
            ERROR("Vulkan Function Error: %d", result);                                            \
        }                                                                                          \
    }

#endif
