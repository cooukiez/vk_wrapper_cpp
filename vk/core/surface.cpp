//
// Created by Ludw on 4/25/2024.
//

#include "../../app.h"

void App::create_surf() {
    if (glfwCreateWindowSurface(inst, window, nullptr, &surf) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface.");
}

VkSurfaceFormatKHR App::choose_surf_format(const std::vector<VkSurfaceFormatKHR> &available) {
    for (const auto &format: available)
        if (format.format == PREFERRED_FORMAT && format.colorSpace == PREFERRED_COLOR_SPACE)
            return format;

    return available[0];
}

VkPresentModeKHR App::choose_pres_mode(const std::vector<VkPresentModeKHR> &available) {
    for (const auto &pres_mode: available)
        if (pres_mode == PREFERRED_PRES_MODE)
            return pres_mode;

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D App::choose_extent(const VkSurfaceCapabilitiesKHR &caps) {
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return caps.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actual_extent.width = std::clamp(actual_extent.width, caps.minImageExtent.width,
                                         caps.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, caps.minImageExtent.height,
                                          caps.maxImageExtent.height);

        return actual_extent;
    }
}

void App::create_swap() {
    VCW_SwapSupport swap_support = query_swap_support(phy_dev);

    VkSurfaceFormatKHR surf_format = choose_surf_format(swap_support.formats);
    VkPresentModeKHR pres_mode = choose_pres_mode(swap_support.pres_modes);
    VkExtent2D extent = choose_extent(swap_support.caps);

    uint32_t img_count = swap_support.caps.minImageCount + 1;
    if (swap_support.caps.maxImageCount > 0 && img_count > swap_support.caps.maxImageCount)
        img_count = swap_support.caps.maxImageCount;

    VkSwapchainCreateInfoKHR swap_info{};
    swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_info.surface = surf;

    swap_info.minImageCount = img_count;
    swap_info.imageFormat = surf_format.format;
    swap_info.imageColorSpace = surf_format.colorSpace;
    swap_info.imageExtent = extent;
    swap_info.imageArrayLayers = 1;
    swap_info.imageUsage = SWAP_IMG_USAGE;

    uint32_t families[] = {qf_indices.qf_graph.value(), qf_indices.qf_pres.value()};

    if (qf_indices.qf_graph != qf_indices.qf_pres) {
        swap_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_info.queueFamilyIndexCount = 2;
        swap_info.pQueueFamilyIndices = families;
    } else {
        swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swap_info.preTransform = swap_support.caps.currentTransform;
    swap_info.compositeAlpha = PREFERRED_COMPOSITE_ALPHA;
    swap_info.presentMode = pres_mode;
    swap_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(dev, &swap_info, nullptr, &swap) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain.");

    vkGetSwapchainImagesKHR(dev, swap, &img_count, nullptr);
    std::vector<VkImage> swap_base_imgs(img_count);
    vkGetSwapchainImagesKHR(dev, swap, &img_count, swap_base_imgs.data());

    for (auto &base_img: swap_base_imgs) {
        VCW_Image img{};
        img.img = base_img;
        img.format = surf_format.format;
        img.extent.width = extent.width;
        img.extent.height = extent.height;
        img.extent.depth = 1;
        img.cur_layout = VK_IMAGE_LAYOUT_UNDEFINED;

        create_img_view(&img, VK_IMAGE_ASPECT_COLOR_BIT);

        swap_imgs.push_back(img);
    }
    swap_img_format = surf_format.format;
    swap_extent = extent;

#ifdef INTERMEDIATE_RENDER_TARGET
    render_extent = {extent.width / RESOLUTION_DIV,
                     extent.height / RESOLUTION_DIV};
#else
    render_extent = swap_extent;
#endif
}

void App::recreate_swap() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(dev);

    clean_up_swap();

    create_swap();
    create_depth_resources();
#ifdef INTERMEDIATE_RENDER_TARGET
    create_render_targets();
    create_frame_bufs(render_targets);
#else
    create_frame_bufs(swap_imgs);
#endif

#ifdef USE_CAMERA
    cam.update_proj(render_extent);
#endif
}

void App::clean_up_swap() {
    clean_up_img(depth_img);

    for (auto framebuffer: frame_bufs)
        vkDestroyFramebuffer(dev, framebuffer, nullptr);

    for (auto img: swap_imgs)
        vkDestroyImageView(dev, img.view, nullptr);
    swap_imgs.clear();

#ifdef INTERMEDIATE_RENDER_TARGET
    for (auto img: render_targets)
        clean_up_img(img);
    render_targets.clear();
#endif

    vkDestroySwapchainKHR(dev, swap, nullptr);
}
