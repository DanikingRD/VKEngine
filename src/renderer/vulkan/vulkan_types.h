#ifndef VULKAN_TYPES
#define VULKAN_TYPES

#include "collections/vector.h"
#include "core/log.h"
#include "math/lineal_types.h"
#include "renderer/renderer_backend.h"
#include "vulkan/vulkan.h"
#include "vulkan_utils.h"

typedef struct Vertex {
    Vec3 pos;
} Vertex;

typedef struct Buffer {
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 mem_flags;
    u64 size;
} Buffer;

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

typedef struct RenderPass {
    VkRenderPass handle;
    f32 depth;
    f32 stencil;
    Vec4 clear_color;
    Vec4 render_area;
} RenderPass;

typedef struct Image {
    VkImage handle;
    VkImageView view;
    // Allocated memory for the image
    VkDeviceMemory memory;
    u32 width;
    u32 height;
} Image;

typedef struct ShaderModule {
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo stage_info;
} ShaderModule;

typedef struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout layout;
} Pipeline;

#define AVAILABLE_SHADER_STAGES 2

typedef struct Shader {
    VkDescriptorPool descriptor_pool;
    // 1 per swapchain image
    VkDescriptorSet* descriptor_sets;
    // no need to create 4 descriptor set layouts
    // since we are using the same layout for each frame.
    VkDescriptorSetLayout descriptor_layout;
    ShaderModule modules[AVAILABLE_SHADER_STAGES];
    Pipeline pipeline;
    GlobalsUBO globals;
    Buffer* globals_buffer;

} Shader;

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
    VkFence** images_in_flight;
    Shader basic_shader;

    Buffer vertex_buffer;
    Buffer index_buffer;

    u32 in_flight_fence_count;

    u32 framebuffer_width;
    u32 framebuffer_height;
    bool recreating_swapchain;
    bool swapchain_needs_resize;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugger;
#endif
    i32 (*find_memory_type)(u32 type_filter, VkMemoryPropertyFlags properties);
    u32 image_index;
    u32 current_frame;
} VulkanBackend;

#define VK_FN_CHECK(fn)                                                                            \
    {                                                                                              \
        VkResult result = fn;                                                                      \
        if (result != VK_SUCCESS) {                                                                \
            ERROR("Vulkan Function Error: %s", vulkan_result_to_str(result, true));                \
        }                                                                                          \
    }

#endif
