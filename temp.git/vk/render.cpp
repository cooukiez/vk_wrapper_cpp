//
// Created by Ludw on 4/25/2024.
//

#include "../app.h"

void App::create_cmd_pool() {
    VkCommandPoolCreateInfo cmd_pool_info{};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_info.queueFamilyIndex = qf_indices.qf_graph.value();

    if (vkCreateCommandPool(dev, &cmd_pool_info, nullptr, &cmd_pool) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics command pool.");
}

VkCommandBuffer App::begin_single_time_cmd() {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = cmd_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd_buf;
    vkAllocateCommandBuffers(dev, &alloc_info, &cmd_buf);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd_buf, &beginInfo);

    return cmd_buf;
}

void App::end_single_time_cmd(VkCommandBuffer cmd_buf) {
    vkEndCommandBuffer(cmd_buf);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd_buf;

    vkQueueSubmit(q_graph, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(q_graph);

    vkFreeCommandBuffers(dev, cmd_pool, 1, &cmd_buf);
}

void App::create_cmd_bufs() {
    cmd_bufs.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t) cmd_bufs.size();

    if (vkAllocateCommandBuffers(dev, &alloc_info, cmd_bufs.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers.");
    }
}

void App::create_sync() {
    img_avl_semps.resize(MAX_FRAMES_IN_FLIGHT);
    rend_fin_semps.resize(MAX_FRAMES_IN_FLIGHT);
    fens.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semp_info{};
    semp_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fen_info{};
    fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(dev, &semp_info, nullptr, &img_avl_semps[i]) != VK_SUCCESS ||
            vkCreateSemaphore(dev, &semp_info, nullptr, &rend_fin_semps[i]) != VK_SUCCESS ||
            vkCreateFence(dev, &fen_info, nullptr, &fens[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects.");
        }
    }
}

void App::create_query_pool(uint32_t loc_frame_query_count) {
    VkQueryPoolCreateInfo query_pool_info{};
    query_pool_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_info.queryCount = loc_frame_query_count * static_cast<uint32_t>(swap_imgs.size());

    if (vkCreateQueryPool(dev, &query_pool_info, nullptr, &query_pool) != VK_SUCCESS)
        throw std::runtime_error("failed to create query pool.");

    frame_query_count = loc_frame_query_count;
}


void App::render() {
    vkWaitForFences(dev, 1, &fens[cur_frame], VK_TRUE, UINT64_MAX);

    uint32_t img_index;
    VkResult result = vkAcquireNextImageKHR(dev, swap, UINT64_MAX, img_avl_semps[cur_frame],
                                            VK_NULL_HANDLE, &img_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    update_bufs(cur_frame);

    vkResetFences(dev, 1, &fens[cur_frame]);

    vkResetCommandBuffer(cmd_bufs[cur_frame], /*VkCommandBufferResetFlagBits*/ 0);
    record_cmd_buf(cmd_bufs[cur_frame], img_index);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semps[] = {img_avl_semps[cur_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = wait_semps;
    submit.pWaitDstStageMask = wait_stages;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd_bufs[cur_frame];

    VkSemaphore signal_semps[] = {rend_fin_semps[cur_frame]};
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = signal_semps;

    if (vkQueueSubmit(q_graph, 1, &submit, fens[cur_frame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit render command buffer.");

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = signal_semps;

    VkSwapchainKHR swaps[] = {swap};
    present.swapchainCount = 1;
    present.pSwapchains = swaps;

    present.pImageIndices = &img_index;

    result = vkQueuePresentKHR(q_pres, &present);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
        resized = false;
        recreate_swap();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image.");
    }

    fetch_queries(img_index);

    cur_frame = (cur_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::clean_up_sync() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(dev, rend_fin_semps[i], nullptr);
        vkDestroySemaphore(dev, img_avl_semps[i], nullptr);
        vkDestroyFence(dev, fens[i], nullptr);
    }
}
