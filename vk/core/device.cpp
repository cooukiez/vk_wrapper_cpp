//
// Created by Ludw on 4/25/2024.
//

#include "../../app.h"

std::vector<VkQueueFamilyProperties> App::get_qf_props(VkPhysicalDevice loc_phy_dev) {
    uint32_t qf_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(loc_phy_dev, &qf_count, nullptr);

    std::vector<VkQueueFamilyProperties> loc_qf_props(qf_count);
    vkGetPhysicalDeviceQueueFamilyProperties(loc_phy_dev, &qf_count, loc_qf_props.data());

    return loc_qf_props;
}

VCW_QueueFamilyIndices App::find_qf(VkPhysicalDevice loc_phy_dev) {
    VCW_QueueFamilyIndices loc_qf_indices;

    std::vector<VkQueueFamilyProperties> loc_qf_props = get_qf_props(loc_phy_dev);

    int i = 0;
    for (const auto &qf: loc_qf_props) {
        if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT && qf.timestampValidBits)
            loc_qf_indices.qf_graph = i;

        VkBool32 pres_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(loc_phy_dev, i, surf, &pres_support);

        if (pres_support)
            loc_qf_indices.qf_pres = i;

        if (loc_qf_indices.is_complete())
            break;

        i++;
    }

    return loc_qf_indices;
}

bool App::check_phy_dev_ext_support(VkPhysicalDevice loc_phy_dev) {
    uint32_t ext_count;
    vkEnumerateDeviceExtensionProperties(loc_phy_dev, nullptr, &ext_count, nullptr);

    std::vector<VkExtensionProperties> available_exts(ext_count);
    vkEnumerateDeviceExtensionProperties(loc_phy_dev, nullptr, &ext_count, available_exts.data());

    std::set<std::string> required_exts(dev_exts.begin(), dev_exts.end());

    for (const auto &ext: available_exts)
        required_exts.erase(ext.extensionName);

    return required_exts.empty();
}

VCW_SwapSupport App::query_swap_support(VkPhysicalDevice loc_phy_dev) {
    VCW_SwapSupport support;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(loc_phy_dev, surf, &support.caps);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(loc_phy_dev, surf, &format_count, nullptr);

    if (format_count != 0) {
        support.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(loc_phy_dev, surf, &format_count, support.formats.data());
    }

    uint32_t pres_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(loc_phy_dev, surf, &pres_mode_count, nullptr);

    if (pres_mode_count != 0) {
        support.pres_modes.resize(pres_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(loc_phy_dev, surf, &pres_mode_count, support.pres_modes.data());
    }

    return support;
}

bool App::is_phy_dev_suitable(VkPhysicalDevice loc_phy_dev) {
    VCW_QueueFamilyIndices loc_qf_indices = find_qf(loc_phy_dev);

    bool exts_supported = check_phy_dev_ext_support(loc_phy_dev);

    bool swap_adequate = false;
    if (exts_supported) {
        VCW_SwapSupport swap_support = query_swap_support(loc_phy_dev);
        swap_adequate = !swap_support.formats.empty() && !swap_support.pres_modes.empty();
    }

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(loc_phy_dev, &features);

    return loc_qf_indices.is_complete() && exts_supported && swap_adequate && features.samplerAnisotropy;
}

void App::pick_phy_dev() {
    uint32_t phy_dev_count = 0;
    vkEnumeratePhysicalDevices(inst, &phy_dev_count, nullptr);

    if (phy_dev_count == 0)
        throw std::runtime_error("failed to find GPU with Vulkan support.");

    std::vector<VkPhysicalDevice> phy_devs(phy_dev_count);
    vkEnumeratePhysicalDevices(inst, &phy_dev_count, phy_devs.data());

    for (const auto &possible_dev: phy_devs) {
        if (is_phy_dev_suitable(possible_dev)) {
            phy_dev = possible_dev;
            break;
        }
    }

    if (phy_dev == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU.");

    vkGetPhysicalDeviceMemoryProperties(phy_dev, &phy_dev_mem_props);
    vkGetPhysicalDeviceProperties(phy_dev, &phy_dev_props);
}

void App::create_dev() {
    qf_indices = find_qf(phy_dev);

    std::vector<VkDeviceQueueCreateInfo> queue_infos;
    std::set<uint32_t> q_families = {qf_indices.qf_graph.value(), qf_indices.qf_pres.value()};

    float q_prior = 1.0f;
    for (uint32_t family: q_families) {
        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = family;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &q_prior;
        queue_infos.push_back(queue_info);
    }

    VkPhysicalDeviceFeatures dev_features{};
    dev_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo dev_info{};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    dev_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
    dev_info.pQueueCreateInfos = queue_infos.data();

    dev_info.pEnabledFeatures = &dev_features;

    dev_info.enabledExtensionCount = static_cast<uint32_t>(dev_exts.size());
    dev_info.ppEnabledExtensionNames = dev_exts.data();

#ifdef VALIDATION
    dev_info.enabledLayerCount = static_cast<uint32_t>(val_layers.size());
    dev_info.ppEnabledLayerNames = val_layers.data();
#else
    dev_info.enabledLayerCount = 0;
#endif

    if (vkCreateDevice(phy_dev, &dev_info, nullptr, &dev) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device.");

    qf_props = get_qf_props(phy_dev);
    vkGetDeviceQueue(dev, qf_indices.qf_graph.value(), 0, &q_graph);
    vkGetDeviceQueue(dev, qf_indices.qf_pres.value(), 0, &q_pres);
}