#ifndef VULKAN_RENDERPASS
#define VULKAN_RENDERPASS

#include "vulkan_types.h"

void vulkan_renderpass_create(VulkanBackend* backend, Vec4 render_area, Vec4 clear, f32 depth,
                              f32 stencil, RenderPass* pass);
void vulkan_renderpass_begin(VulkanBackend* backend, RenderPass* pass,
                             CommandBuffer* command_buffer, VkFramebuffer framebuffer);
void vulkan_renderpass_end(RenderPass* pass, CommandBuffer* command_buffer);
void vulkan_renderpass_destroy(VulkanBackend* backend, RenderPass* pass);

#endif
