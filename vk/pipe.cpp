//
// Created by Ludw on 4/25/2024.
//

#include "../app.h"

void App::create_rendp() {
    VkAttachmentDescription color_attach{};
    color_attach.format = swap_img_format;
    color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
#ifdef RESOLUTION_SCALE
    color_attach.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
#else
    color_attach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#endif

    VkAttachmentDescription depth_attach{};
    depth_attach.format = find_depth_format();
    depth_attach.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attach_ref{};
    color_attach_ref.attachment = 0;
    color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attach_ref{};
    depth_attach_ref.attachment = 1;
    depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attach_ref;
    subpass.pDepthStencilAttachment = &depth_attach_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {color_attach, depth_attach};
    VkRenderPassCreateInfo rendp_info{};
    rendp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rendp_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    rendp_info.pAttachments = attachments.data();
    rendp_info.subpassCount = 1;
    rendp_info.pSubpasses = &subpass;
    rendp_info.dependencyCount = 1;
    rendp_info.pDependencies = &dependency;

    if (vkCreateRenderPass(dev, &rendp_info, nullptr, &rendp) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass.");
    }
}

VkShaderModule App::create_shader_mod(const std::vector<char> &code) {
    VkShaderModuleCreateInfo mod_info{};
    mod_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    mod_info.codeSize = code.size();
    mod_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule mod;
    if (vkCreateShaderModule(dev, &mod_info, nullptr, &mod) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module.");
    }

    return mod;
}

void App::create_render_targets() {
    render_targets.resize(swap_imgs.size());

    for (auto &render_target: render_targets) {
        render_target = create_img(swap_extent, swap_img_format, VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        create_img_view(&render_target, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void App::create_frame_bufs(std::vector<VCW_Image> img_targets) {
    frame_bufs.resize(swap_imgs.size());

    for (size_t i = 0; i < swap_imgs.size(); i++) {
        std::array<VkImageView, 2> attachments = {
                img_targets[i].view,
                depth_img.view
        };

        VkFramebufferCreateInfo frame_buf_info{};
        frame_buf_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buf_info.renderPass = rendp;
        frame_buf_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        frame_buf_info.pAttachments = attachments.data();
        frame_buf_info.width = swap_extent.width;
        frame_buf_info.height = swap_extent.height;
        frame_buf_info.layers = 1;

        if (vkCreateFramebuffer(dev, &frame_buf_info, nullptr, &frame_bufs[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer.");
    }
}

void App::clean_up_pipe() {
    vkDestroyPipeline(dev, pipe, nullptr);
    vkDestroyPipelineLayout(dev, pipe_layout, nullptr);
    vkDestroyRenderPass(dev, rendp, nullptr);
}