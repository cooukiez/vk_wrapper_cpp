//
// Created by Ludw on 4/25/2024.
//

#ifndef VCW_PROP_H
#define VCW_PROP_H

#define WINDOW_TITLE "Vulkan App"

#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600

#define MAX_FRAMES_IN_FLIGHT 2

#define APP_NAME "Vulkan App"
#define ENGINE_NAME "No Engine"

// #define VERBOSE
// #define VALIDATION

const std::vector<const char *> val_layers = {
        "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char *> dev_exts = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
        // VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME // not required anymore
};

//
// scaling resolution
//
// #define INTERMEDIATE_RENDER_TARGET
#define RESOLUTION_DIV 4
//
// for testing blit vs. copy performance for RESOLUTION_DIV of 1
//
// #define TESTING_COPY_INSTEAD_BLIT_IMG

const VkFormat PREFERRED_FORMAT = VK_FORMAT_B8G8R8A8_SRGB;
const VkColorSpaceKHR PREFERRED_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
const VkPresentModeKHR PREFERRED_PRES_MODE = VK_PRESENT_MODE_IMMEDIATE_KHR;
const VkCompositeAlphaFlagBitsKHR PREFERRED_COMPOSITE_ALPHA = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

#ifdef INTERMEDIATE_RENDER_TARGET
const VkImageUsageFlags SWAP_IMG_USAGE = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
#else
const VkImageUsageFlags SWAP_IMG_USAGE = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
#endif

const VkComponentMapping DEFAULT_COMPONENT_MAPPING = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY};
const VkImageSubresourceRange DEFAULT_SUBRESOURCE_RANGE = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};
const VkImageSubresourceLayers DEFAULT_SUBRESOURCE_LAYERS = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};

const VkBorderColor DEFAULT_SAMPLER_BORDER_COLOR = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

// Rasterization Stage
const VkBool32 DEPTH_CLAMP_ENABLE = VK_FALSE;
const VkBool32 RASTERIZER_DISCARD_ENABLE = VK_FALSE;
const VkPolygonMode POLYGON_MODE = VK_POLYGON_MODE_FILL;
// const VkCullModeFlags CULL_MODE = VK_CULL_MODE_NONE;
const VkCullModeFlags CULL_MODE = VK_CULL_MODE_BACK_BIT;
// const VkFrontFace FRONT_FACE = VK_FRONT_FACE_COUNTER_CLOCKWISE;
const VkFrontFace FRONT_FACE = VK_FRONT_FACE_CLOCKWISE;

// #define BIND_SAMPLE_TEXTURE
// #define ENABLE_DEPTH_TESTING

// #define ENABLE_UNIFORM
const VkShaderStageFlags UNIFORM_STAGE = VK_SHADER_STAGE_VERTEX_BIT;

// #define ENABLE_PUSH_CONSTANTS
const VkShaderStageFlags PUSH_CONSTANTS_STAGE = VK_SHADER_STAGE_ALL_GRAPHICS;

//
// select which vertex set you want to use
// just comment out the sets you do not want
// (if none selected default is triangle)
//
// #define CUBE_DATA
// #define PLATE_DATA
#define TRIANGLE_DATA

// #define USE_CAMERA

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