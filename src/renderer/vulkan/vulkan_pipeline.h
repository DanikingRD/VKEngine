#ifndef VULKAN_PIPELINE_H
#define VULKAN_PIPELINE_H

#include "vulkan_types.h"

bool vulkan_render_pipeline_create(VulkanBackend* backend, RenderPass* pass, u32 attribute_count,
                                   VkVertexInputAttributeDescription* vertex_attributes,
                                   u32 descriptor_set_layout_count,
                                   VkDescriptorSetLayout* descriptor_set_layouts, u32 stage_count,
                                   VkPipelineShaderStageCreateInfo* create_infos,
                                   VkViewport viewport, VkRect2D scissor, bool wireframe,
                                   Pipeline* pipeline);

void vulkan_pipeline_bind(VulkanBackend* backend, CommandBuffer cmdbuf, Pipeline* pipeline);

void vulkan_pipeline_destroy(VulkanBackend* backend, Pipeline* pipeline);

#endif
