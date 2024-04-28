//
// Created by Ludw on 4/25/2024.
//

#include "app.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

void App::init_app() {
    //
    // vulkan core initialization
    //
    create_inst();
    setup_debug_msg();
    create_surf();

    pick_phy_dev();
    create_dev();

    create_swap();

    //
    // pipeline creation
    //
    create_rendp();
    create_desc_pool_layout();
    create_pipe();

    create_cmd_pool();

#ifdef ENABLE_DEPTH_TESTING
    create_depth_resources();
#endif
#ifdef BIND_SAMPLE_TEXTURE
    create_tex_img();
#endif

#ifdef CUBE_DATA
    create_vert_buf(CUBE_VERTICES);
    create_index_buf(CUBE_INDICES);
#elifdef PLATE_DATA
    create_vert_buf(PLATE_VERTICES);
    create_index_buf(PLATE_INDICES);
#elifdef SCREEN_QUAD_DATA
    create_vert_buf(SCREEN_QUAD_VERTICES);
    create_index_buf(SCREEN_QUAD_INDICES);
#else
    create_vert_buf(TRIANGLE_VERTICES);
    create_index_buf(TRIANGLE_INDICES);
#endif
#ifdef ENABLE_UNIFORM
    create_unif_bufs();
#endif

#ifdef INTERMEDIATE_RENDER_TARGET
    create_render_targets();
    create_frame_bufs(render_targets);
#else
    create_frame_bufs(swap_imgs);
#endif
    create_desc_pool(MAX_FRAMES_IN_FLIGHT);
    write_desc_pool();

    create_cmd_bufs();
    create_sync();

    create_query_pool(3);

#ifdef USE_CAMERA
    cam.create_default_cam(render_extent);
#endif
}

void App::create_vert_buf(const std::vector<Vertex> &vertices_dataset) {
    vertices = vertices_dataset;
    VkDeviceSize buf_size = sizeof(vertices[0]) * vertices.size();

    VCW_Buffer staging_buf = create_buf(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    cp_data_to_buf(&staging_buf, (void *) vertices.data());

    vert_buf = create_buf(buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    cp_buf(staging_buf, vert_buf);

    clean_up_buf(staging_buf);
}

void App::create_index_buf(const std::vector<uint16_t> &indices_dataset) {
    indices = indices_dataset;
    VkDeviceSize buf_size = sizeof(indices[0]) * indices.size();

    VCW_Buffer staging_buf = create_buf(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    cp_data_to_buf(&staging_buf, (void *) indices.data());

    index_buf = create_buf(buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    cp_buf(staging_buf, index_buf);

    clean_up_buf(staging_buf);
}

void App::create_unif_bufs() {
    VkDeviceSize buf_size = sizeof(VCW_Uniform);
    unif_bufs.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        unif_bufs[i] = create_buf(buf_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        map_buf(&unif_bufs[i]);
    }
}

void App::create_tex_img() {
    int tex_width, tex_height, tex_channels;
    stbi_uc *pixels = stbi_load("textures/texture.jpg", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    VkDeviceSize img_size = tex_width * tex_height * 4;

    if (!pixels)
        throw std::runtime_error("failed to load texture image.");

    VCW_Buffer staging_buf = create_buf(img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    cp_data_to_buf(&staging_buf, pixels);

    stbi_image_free(pixels);

    VkExtent2D extent = {(uint32_t) tex_width, (uint32_t) tex_height};
    tex_img = create_img(extent, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer cmd_buf = begin_single_time_cmd();
    transition_img_layout(cmd_buf, &tex_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT);
    cp_buf_to_img(cmd_buf, staging_buf, tex_img, extent);
    transition_img_layout(cmd_buf, &tex_img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    end_single_time_cmd(cmd_buf);

    create_img_view(&tex_img, VK_IMAGE_ASPECT_COLOR_BIT);
    create_sampler(&tex_img, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);

    clean_up_buf(staging_buf);
}

void App::create_depth_resources() {
    VkFormat depth_format = find_depth_format();

    depth_img = create_img(swap_extent, depth_format, VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    create_img_view(&depth_img, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void App::create_desc_pool_layout() {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    uint32_t last_binding = 0;

#ifdef ENABLE_UNIFORM
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = last_binding;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.pImmutableSamplers = nullptr;
    ubo_layout_binding.stageFlags = UNIFORM_STAGE;
    bindings.push_back(ubo_layout_binding);
    last_binding++;
#endif
#ifdef BIND_SAMPLE_TEXTURE
    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = last_binding;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(sampler_layout_binding);
    last_binding++;
#endif

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        add_desc_set_layout(static_cast<uint32_t>(bindings.size()), bindings.data());
    }

#ifdef ENABLE_UNIFORM
    add_pool_size(MAX_FRAMES_IN_FLIGHT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
#endif
#ifdef BIND_SAMPLE_TEXTURE
    add_pool_size(MAX_FRAMES_IN_FLIGHT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
#endif
}

void App::create_pipe() {
    auto vert_code = read_file("vert.spv");
    auto frag_code = read_file("frag.spv");

    VkShaderModule vert_module = create_shader_mod(vert_code);
    VkShaderModule frag_module = create_shader_mod(frag_code);

    VkPipelineShaderStageCreateInfo vert_stage_info{};
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info{};
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {vert_stage_info, frag_stage_info};

    VkPipelineVertexInputStateCreateInfo vert_input_info{};
    vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto binding_desc = Vertex::get_binding_desc();
    auto attrib_descs = Vertex::get_attrib_descs();

    vert_input_info.vertexBindingDescriptionCount = 1;
    vert_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrib_descs.size());
    vert_input_info.pVertexBindingDescriptions = &binding_desc;
    vert_input_info.pVertexAttributeDescriptions = attrib_descs.data();

    VkPipelineInputAssemblyStateCreateInfo input_asm_info{};
    input_asm_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_asm_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_asm_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_info{};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster_info{};
    raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_info.depthClampEnable = DEPTH_CLAMP_ENABLE;
    raster_info.rasterizerDiscardEnable = RASTERIZER_DISCARD_ENABLE;
    raster_info.polygonMode = POLYGON_MODE;
    raster_info.lineWidth = 1.0f;
    raster_info.cullMode = CULL_MODE;
    raster_info.frontFace = FRONT_FACE;
    raster_info.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisample_info{};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

#ifdef ENABLE_DEPTH_TESTING
    VkPipelineDepthStencilStateCreateInfo depth_info{};
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_info.depthTestEnable = VK_TRUE;
    depth_info.depthWriteEnable = VK_TRUE;
    depth_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_info.depthBoundsTestEnable = VK_FALSE;
    depth_info.stencilTestEnable = VK_FALSE;
#endif

    VkPipelineColorBlendAttachmentState blend_attach{};
    blend_attach.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    blend_attach.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo blend_info{};
    blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_info.logicOpEnable = VK_FALSE;
    blend_info.logicOp = VK_LOGIC_OP_COPY;
    blend_info.attachmentCount = 1;
    blend_info.pAttachments = &blend_attach;
    blend_info.blendConstants[0] = 0.0f;
    blend_info.blendConstants[1] = 0.0f;
    blend_info.blendConstants[2] = 0.0f;
    blend_info.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_info.pDynamicStates = dynamic_states.data();

#ifdef ENABLE_PUSH_CONSTANTS
    VkPushConstantRange push_const_range{};
    push_const_range.stageFlags = PUSH_CONSTANTS_STAGE;
    push_const_range.offset = 0;
    push_const_range.size = sizeof(VCW_PushConstants);
#endif

    VkPipelineLayoutCreateInfo pipe_layout_info{};
    pipe_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipe_layout_info.setLayoutCount = desc_set_layouts.size();
    pipe_layout_info.pSetLayouts = desc_set_layouts.data();
#ifdef ENABLE_PUSH_CONSTANTS
    pipe_layout_info.pushConstantRangeCount = 1;
    pipe_layout_info.pPushConstantRanges = &push_const_range;
#endif

    if (vkCreatePipelineLayout(dev, &pipe_layout_info, nullptr, &pipe_layout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout.");

    VkGraphicsPipelineCreateInfo pipe_info{};
    pipe_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipe_info.stageCount = 2;
    pipe_info.pStages = stages;
    pipe_info.pVertexInputState = &vert_input_info;
    pipe_info.pInputAssemblyState = &input_asm_info;
    pipe_info.pViewportState = &viewport_info;
    pipe_info.pRasterizationState = &raster_info;
    pipe_info.pMultisampleState = &multisample_info;
#ifdef ENABLE_DEPTH_TESTING
    pipe_info.pDepthStencilState = &depth_info;
#endif
    pipe_info.pColorBlendState = &blend_info;
    pipe_info.pDynamicState = &dynamic_state_info;
    pipe_info.layout = pipe_layout;
    pipe_info.renderPass = rendp;
    pipe_info.subpass = 0;
    pipe_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipe_info, nullptr, &pipe) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline.");

    vkDestroyShaderModule(dev, frag_module, nullptr);
    vkDestroyShaderModule(dev, vert_module, nullptr);
}

void App::write_desc_pool() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uint32_t last_binding = 0;
#ifdef ENABLE_UNIFORM
        write_buf_desc_binding(unif_bufs[i], i, last_binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        last_binding++;
#endif
#ifdef BIND_SAMPLE_TEXTURE
        write_img_desc_binding(tex_img, i, last_binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
#endif
    }
}

void App::update_bufs(uint32_t index_inflight_frame) {
#ifdef ENABLE_PUSH_CONSTANTS
    push_const.view_proj = cam.get_view_proj();
    push_const.res = {render_extent.width, render_extent.height};
    push_const.time = stats.frame_count;
#endif
#ifdef ENABLE_UNIFORM
    ubo.data = cam.get_view_proj();
    memcpy(unif_bufs[index_inflight_frame].p_mapped_mem, &ubo, sizeof(ubo));
#endif
}

void App::record_cmd_buf(VkCommandBuffer cmd_buf, uint32_t img_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmd_buf, &begin_info) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer.");

    VkRenderPassBeginInfo rendp_begin_info{};
    rendp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rendp_begin_info.renderPass = rendp;
    rendp_begin_info.framebuffer = frame_bufs[img_index];
    rendp_begin_info.renderArea.offset = {0, 0};
    rendp_begin_info.renderArea.extent = render_extent;


#ifdef ENABLE_DEPTH_TESTING
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};
#else
    std::array<VkClearValue, 1> clear_values{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
#endif

    rendp_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    rendp_begin_info.pClearValues = clear_values.data();

    vkCmdResetQueryPool(cmd_buf, query_pool, img_index * 3, 3);

    vkCmdWriteTimestamp(cmd_buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, img_index * frame_query_count);
    vkCmdBeginRenderPass(cmd_buf, &rendp_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) render_extent.width;
    viewport.height = (float) render_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = render_extent;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    VkBuffer vert_bufs[] = {vert_buf.buf};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd_buf, 0, 1, vert_bufs, offsets);

    vkCmdBindIndexBuffer(cmd_buf, index_buf.buf, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout, 0, 1,
                            &desc_sets[cur_frame], 0, nullptr);
#ifdef ENABLE_PUSH_CONSTANTS
    vkCmdPushConstants(cmd_buf, pipe_layout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(VCW_PushConstants),
                       &push_const);
#endif

    vkCmdDrawIndexed(cmd_buf, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(cmd_buf);

    vkCmdWriteTimestamp(cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool, img_index * frame_query_count + 1);

#ifdef INTERMEDIATE_RENDER_TARGET
    // set by renderpass
    render_targets[img_index].cur_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    swap_imgs[img_index].cur_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    transition_img_layout(cmd_buf, &swap_imgs[img_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

#ifdef TESTING_COPY_INSTEAD_BLIT_IMG
    copy_img(cmd_buf, render_targets[img_index], swap_imgs[img_index]);
#else
    VkExtent3D src_extent = {render_extent.width, render_extent.height, 0};
    blit_img(cmd_buf, render_targets[img_index], src_extent, swap_imgs[img_index], swap_imgs[img_index].extent,
             VK_FILTER_LINEAR);
#endif

    transition_img_layout(cmd_buf, &swap_imgs[img_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                          VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
#endif

    vkCmdWriteTimestamp(cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pool, img_index * frame_query_count + 2);

    if (vkEndCommandBuffer(cmd_buf) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer.");
}

void App::fetch_queries(uint32_t img_index) {
    uint64_t buffer[frame_query_count];

    VkResult result = vkGetQueryPoolResults(dev, query_pool, img_index * frame_query_count, frame_query_count,
                                            sizeof(uint64_t) * frame_query_count, buffer, sizeof(uint64_t),
                                            VK_QUERY_RESULT_64_BIT);
    if (result == VK_NOT_READY) {
        return;
    } else if (result == VK_SUCCESS) {
        stats.gpu_frame_time = (float) (buffer[2] - buffer[0]) * phy_dev_props.limits.timestampPeriod;
        stats.blit_img_time = (float) (buffer[2] - buffer[1]) * phy_dev_props.limits.timestampPeriod;
    } else {
        throw std::runtime_error("failed to receive query results.");
    }
}

void App::render_loop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        render();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam.pos += cam.mov_lin;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam.pos -= cam.mov_lin;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam.pos -= cam.mov_lat;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam.pos += cam.mov_lat;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cam.speed = CAM_FAST;
        else
            cam.speed = CAM_SLOW;

        stats.frame_count++;
    }

    vkDeviceWaitIdle(dev);
}

void App::clean_up() {
    clean_up_swap();
    clean_up_pipe();

    clean_up_desc();

#ifdef ENABLE_UNIFORM
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        clean_up_buf(unif_bufs[i]);
#endif
#ifdef BIND_SAMPLE_TEXTURE
    clean_up_img(tex_img);
#endif

    clean_up_buf(vert_buf);
    clean_up_buf(index_buf);

    vkDestroyQueryPool(dev, query_pool, nullptr);

    clean_up_sync();

    vkDestroyCommandPool(dev, cmd_pool, nullptr);
    vkDestroyDevice(dev, nullptr);

#ifdef VALIDATION
    destroy_debug_callback(inst, debug_msg, nullptr);
#endif

    vkDestroySurfaceKHR(inst, surf, nullptr);
    vkDestroyInstance(inst, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}