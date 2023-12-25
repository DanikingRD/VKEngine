#include "vulkan_pipeline.h"

bool vulkan_render_pipeline_create(VulkanBackend* backend, RenderPass* pass, u32 attribute_count,
                                   VkVertexInputAttributeDescription* vertex_attributes,
                                   u32 descriptor_set_layout_count,
                                   VkDescriptorSetLayout* descriptor_set_layouts, u32 stage_count,
                                   VkPipelineShaderStageCreateInfo* create_infos,
                                   VkViewport viewport, VkRect2D scissor, bool wireframe,
                                   Pipeline* pipeline) {
    VkPipelineViewportStateCreateInfo viewport_state = {0};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {0};
    rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.depthClampEnable = VK_FALSE;
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_create_info.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer_create_info.lineWidth = 1.0f;
    rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_create_info.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisample_state = {0};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.sampleShadingEnable = VK_FALSE;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state.minSampleShading = 1.0f;

    // Depth
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {0};
    depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.depthTestEnable = VK_TRUE;
    depth_stencil_state.depthWriteEnable = VK_TRUE;
    depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend = {0};
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blend.attachmentCount = 1;
    color_blend.pAttachments = &color_blend_attachment;

    const u32 dynamic_state_count = 3;
    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dyn_state_create_info = {0};
    dyn_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_state_create_info.dynamicStateCount = dynamic_state_count;
    dyn_state_create_info.pDynamicStates = dynamic_states;

    VkVertexInputBindingDescription binding_description = {0};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {0};
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.vertexBindingDescriptionCount = 1;
    vertex_input_state.pVertexBindingDescriptions = &binding_description;
    vertex_input_state.vertexAttributeDescriptionCount = attribute_count;
    vertex_input_state.pVertexAttributeDescriptions = vertex_attributes;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;

    VK_FN_CHECK(vkCreatePipelineLayout(backend->device.logical, &pipeline_layout_create_info,
                                       backend->allocator, &pipeline->layout));

    VkGraphicsPipelineCreateInfo pipeline_create_info = {0};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = stage_count;
    pipeline_create_info.pStages = create_infos;
    pipeline_create_info.pVertexInputState = &vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &input_assembly;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pColorBlendState = &color_blend;
    pipeline_create_info.pDynamicState = &dyn_state_create_info;
    pipeline_create_info.layout = pipeline->layout;
    pipeline_create_info.renderPass = pass->handle;

    VK_FN_CHECK(vkCreateGraphicsPipelines(backend->device.logical, VK_NULL_HANDLE, 1,
                                          &pipeline_create_info, backend->allocator,
                                          &pipeline->pipeline));

    return true;
}

void vulkan_pipeline_bind(VulkanBackend* backend, CommandBuffer cmdbuf, Pipeline* pipeline) {
    vkCmdBindPipeline(cmdbuf.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
}

void vulkan_pipeline_destroy(VulkanBackend* backend, Pipeline* pipeline) {
    if (pipeline) {
        vkDestroyPipeline(backend->device.logical, pipeline->pipeline, backend->allocator);
        vkDestroyPipelineLayout(backend->device.logical, pipeline->layout, backend->allocator);
        pipeline->pipeline = 0;
        pipeline->layout = 0;
    }
}
