//
// Created by Ludw on 4/25/2024.
//

#ifndef VCW_PROP_H
#define VCW_PROP_H

// #define VERBOSE

#define WINDOW_TITLE "Vulkan App"

#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600

#define MAX_FRAMES_IN_FLIGHT 2

#define APP_NAME "Vulkan App"
#define ENGINE_NAME "No Engine"

#define VALIDATION

const std::vector<const char *> val_layers = {
        "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char *> dev_exts = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const VkPresentModeKHR PREFERRED_PRES_MODE = VK_PRESENT_MODE_IMMEDIATE_KHR;

const VkComponentMapping DEFAULT_COMPONENT_MAPPING = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY};
const VkImageSubresourceRange DEFAULT_SUBRESOURCE_RANGE = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};
const VkImageSubresourceLayers DEFAULT_SUBRESOURCE_LAYERS = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};

//
// default values for camera
//
#define CAM_FOV 60.0f
#define CAM_SENSITIVITY 0.1f

#define CAM_NEAR 0.1f
#define CAM_FAR 100.0f

#define CAM_MIN_PITCH (-89.0f)
#define CAM_MAX_PITCH 89.0f
#define CAM_MIN_YAW (-180.0f)
#define CAM_MAX_YAW 180.0f

#define CAM_SLOW 0.05f
#define CAM_FAST 0.1f

#endif //VCW_PROP_H

// scaling resolution
// #define RESOLUTION_SCALE
#define RESOLUTION_DIV 2
