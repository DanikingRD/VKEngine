#include "vulkan_renderpass.h"
#include <vulkan/vulkan_core.h>

void vulkan_renderpass_create(
    VulkanBackend* backend, RenderArea render_area, Color clear, f32 depth, f32 stencil,
    RenderPass* pass
) {

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentDescription attachments[2] = {0};

    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = backend->swapchain.format.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment = {0};
    depth_attachment.format = backend->device.depth_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {0};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency subpass_dependency = {0};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Maintain a list of attachments to be used by the render pass
    attachments[0] = color_attachment;
    attachments[1] = depth_attachment;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &subpass_dependency;

    VK_FN_CHECK(vkCreateRenderPass(backend->device.logical, &render_pass_info, 0, &pass->handle));

    pass->clear_color = clear;
    pass->depth = depth;
    pass->stencil = stencil;
    pass->render_area = render_area;
}

void vulkan_renderpass_begin(
    VulkanBackend* backend, RenderPass* pass, CommandBuffer* command_buffer,
    VkFramebuffer framebuffer
) {
    VkRenderPassBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = pass->handle;
    begin_info.framebuffer = framebuffer;
    begin_info.renderArea.offset.x = pass->render_area.x;
    begin_info.renderArea.offset.y = pass->render_area.y;
    begin_info.renderArea.extent.width = pass->render_area.width;
    begin_info.renderArea.extent.height = pass->render_area.height;

    VkClearValue clear_values[2] = {0};
    clear_values[0].color.float32[0] = pass->clear_color.r;
    clear_values[0].color.float32[1] = pass->clear_color.g;
    clear_values[0].color.float32[2] = pass->clear_color.b;
    clear_values[0].color.float32[3] = pass->clear_color.a;
    clear_values[1].depthStencil.depth = pass->depth;
    clear_values[1].depthStencil.stencil = pass->stencil;

    begin_info.clearValueCount = 2;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan_renderpass_end(RenderPass* pass, CommandBuffer* command_buffer) {
    vkCmdEndRenderPass(command_buffer->handle);
}

void vulkan_renderpass_destroy(VulkanBackend* backend, RenderPass* pass) {
    if (pass->handle) {
        vkDestroyRenderPass(backend->device.logical, pass->handle, 0);
        pass->handle = 0;
    }
}
