//
// Created by Ludw on 4/25/2024.
//

#include "inc.h"
#include "prop.h"
#include "util.h"

#include "render/camera.h"

#ifndef VCW_APP_H
#define VCW_APP_H

//
// vulkan debug
//
VkResult create_debug_callback(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT *p_create_inf,
                               const VkAllocationCallbacks *p_alloc, VkDebugUtilsMessengerEXT *p_debug_msg);

void destroy_debug_callback(VkInstance inst, VkDebugUtilsMessengerEXT debug_msg, const VkAllocationCallbacks *p_alloc);

void get_debug_msg_info(VkDebugUtilsMessengerCreateInfoEXT &debug_info);

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
               const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);

bool check_validation_support();

struct VCW_QueueFamilyIndices {
    std::optional<uint32_t> qf_graph;
    std::optional<uint32_t> qf_pres;

    bool is_complete() {
        return qf_graph.has_value() && qf_pres.has_value();
    }
};

struct VCW_SwapSupport {
    VkSurfaceCapabilitiesKHR caps;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> pres_modes;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;

    static VkVertexInputBindingDescription get_binding_desc();

    static std::array<VkVertexInputAttributeDescription, 2> get_attrib_descs();
};

struct VCW_PushConstants {
    alignas(16) glm::mat4 view_proj;
    alignas(8) glm::vec2 res;
    alignas(4) uint32_t time;
};

struct VCW_Uniform {
    alignas(16) glm::mat4 data;
};

struct VCW_Buffer {
    VkDeviceSize size;
    VkBuffer buf;
    VkDeviceMemory mem;
    void *p_mapped_mem = nullptr;
};

struct VCW_Image {
    VkImage img;
    VkDeviceMemory mem;

    VkImageView view;

    VkSampler sampler;
    bool has_sampler = false;

    VkExtent3D extent;
    VkFormat format;
    VkImageLayout cur_layout;
};

struct VCW_RenderStats {
    double frame_time;
    double gpu_frame_time;
    double blit_img_time;
    uint32_t frame_count;
};

class App {
public:
    void run() {
        init_window();
        init_app();
        render_loop();
        clean_up();
    }

private:
    GLFWwindow *window;
    bool resized = false;
    glm::vec2 cursor_pos;

    VkInstance inst;
    VkDebugUtilsMessengerEXT debug_msg;

    VkSurfaceKHR surf;

    VkPhysicalDevice phy_dev = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties phy_dev_mem_props;
    VkPhysicalDeviceProperties phy_dev_props;

    VCW_QueueFamilyIndices qf_indices;
    VkDevice dev;
    VkQueue q_graph;
    VkQueue q_pres;

    VkSwapchainKHR swap;
    std::vector<VCW_Image> swap_imgs;
    VkFormat swap_img_format;
    VkExtent2D swap_extent;
    VkExtent2D render_extent;

    VkRenderPass rendp;
    VkPipelineLayout pipe_layout;
    VkPipeline pipe;
    std::vector<VkFramebuffer> frame_bufs;
    std::vector<VCW_Image> render_targets;

    std::vector<VkDescriptorSetLayout> desc_set_layouts;
    std::vector<VkDescriptorPoolSize> desc_pool_sizes;
    VkDescriptorPool desc_pool;
    std::vector<VkDescriptorSet> desc_sets;

    VkCommandPool cmd_pool;
    std::vector<VkCommandBuffer> cmd_bufs;

    VCW_Image depth_img;
    VCW_Image tex_img;

    std::vector<Vertex> vertices;
    VCW_Buffer vert_buf;
    std::vector<uint16_t> indices;
    VCW_Buffer index_buf;

    VkQueryPool query_pool;
    uint32_t frame_query_count;

    VCW_PushConstants push_const;

    VCW_Uniform ubo;
    std::vector<VCW_Buffer> unif_bufs;

    std::vector<VkSemaphore> img_avl_semps;
    std::vector<VkSemaphore> rend_fin_semps;
    std::vector<VkFence> fens;

    uint32_t cur_frame = 0;
    VCW_RenderStats stats;

    VCW_Camera cam;

    //
    //
    //
    // function definitions
    //
    //

    void init_app();

    void render_loop();

    void clean_up();

    //
    // window
    //
    static void frame_buf_resized(GLFWwindow *window, int width, int height);

    static void cursor_callback(GLFWwindow *window, double nx, double ny);

    void init_window();

    //
    // vulkan instance
    //
    static std::vector<const char *> get_required_exts();

    void create_inst();

    void setup_debug_msg();

    //
    //
    // vulkan primitives
    //
    // surface
    //
    void create_surf();

    //
    // physical device
    //
    VCW_QueueFamilyIndices find_qf(VkPhysicalDevice loc_phy_dev);

    static bool check_phy_dev_ext_support(VkPhysicalDevice loc_phy_dev);

    VCW_SwapSupport query_swap_support(VkPhysicalDevice loc_phy_dev);

    bool is_phy_dev_suitable(VkPhysicalDevice loc_phy_dev);

    void pick_phy_dev();

    //
    // logical device
    //
    void create_dev();

    //
    // swapchain
    //
    static VkSurfaceFormatKHR choose_surf_format(const std::vector<VkSurfaceFormatKHR> &available);

    static VkPresentModeKHR choose_pres_mode(const std::vector<VkPresentModeKHR> &available);

    VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR &caps);

    void create_swap();

    void recreate_swap();

    void clean_up_swap();

    //
    // buffers
    //
    uint32_t find_mem_type(uint32_t type_filter, VkMemoryPropertyFlags mem_flags);

    VCW_Buffer create_buf(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_props);

    void map_buf(VCW_Buffer *p_buf);

    void unmap_buf(VCW_Buffer *p_buf);

    void cp_data_to_buf(VCW_Buffer *p_buf, void *p_data);

    void cp_buf(VCW_Buffer src_buf, VCW_Buffer dst_buf);

    void clean_up_buf(VCW_Buffer buf);

    //
    // images
    //
    VkFormat find_supported_format(const std::vector<VkFormat> &possible_formats, VkImageTiling tiling,
                                   VkFormatFeatureFlags features);

    VkFormat find_depth_format();

    bool has_stencil_component(VkFormat format);

    VCW_Image create_img(VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags mem_props);

    void create_img_view(VCW_Image *p_img, VkImageAspectFlags aspect_flags);

    void create_sampler(VCW_Image *p_img, VkFilter filter, VkSamplerAddressMode address_mode);

    static VkAccessFlags get_access_mask(VkImageLayout layout);

    static void transition_img_layout(VkCommandBuffer cmd_buf, VCW_Image *p_img, VkImageLayout layout,
                                      VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage);

    static void cp_buf_to_img(VkCommandBuffer cmd_buf, VCW_Buffer buf, VCW_Image img, VkExtent2D extent);

    static  void blit_img(VkCommandBuffer cmd_buf, VCW_Image src, VkExtent3D src_extent, VCW_Image dst, VkExtent3D dst_extent, VkFilter filter);

    static void blit_img(VkCommandBuffer cmd_buf, VCW_Image src, VCW_Image dst, VkFilter filter);

    static void copy_img(VkCommandBuffer cmd_buf, VCW_Image src, VCW_Image dst);

    void clean_up_img(VCW_Image img);

    //
    // descriptor pool
    //
    void add_desc_set_layout(uint32_t binding_count, VkDescriptorSetLayoutBinding *p_bindings);

    void add_pool_size(uint32_t desc_count, VkDescriptorType desc_type);

    void create_desc_pool(uint32_t max_sets);

    void write_buf_desc_binding(VCW_Buffer buf, uint32_t dst_set, uint32_t dst_binding, VkDescriptorType desc_type);

    void write_img_desc_binding(VCW_Image img, uint32_t dst_set, uint32_t dst_binding, VkDescriptorType desc_type);

    void clean_up_desc();

    //
    // pipeline prerequisites
    //
    void create_rendp();

    VkShaderModule create_shader_mod(const std::vector<char> &code);

    void create_render_targets();

    void create_frame_bufs(std::vector<VCW_Image> img_targets);

    void clean_up_pipe();

    //
    // render prerequisites
    //
    void create_cmd_pool();

    VkCommandBuffer begin_single_time_cmd();

    void end_single_time_cmd(VkCommandBuffer cmd_buf);

    void create_cmd_bufs();

    void create_sync();

    void create_query_pool(uint32_t loc_frame_query_count);

    void render();

    void clean_up_sync();

    //
    //
    // personalized vulkan initialization
    //
    void create_vert_buf(const std::vector<Vertex> &vertices_dataset);

    void create_index_buf(const std::vector<uint16_t> &indices_dataset);

    void create_unif_bufs();

    void create_tex_img();

    void create_depth_resources();

    void create_desc_pool_layout();

    void create_pipe();

    void write_desc_pool();

    void update_bufs(uint32_t index_inflight_frame);

    void record_cmd_buf(VkCommandBuffer cmd_buf, uint32_t img_index);

    void fetch_queries(uint32_t img_index);
};

const std::vector<Vertex> PLATE_SAMPLE_VERTICES = {{{-0.5f, -0.5f, 0.0f},  {1.0f, 0.0f}},
                                                   {{0.5f,  -0.5f, 0.0f},  {0.0f, 0.0f}},
                                                   {{0.5f,  0.5f,  0.0f},  {0.0f, 1.0f}},
                                                   {{-0.5f, 0.5f,  0.0f},  {1.0f, 1.0f}},

                                                   {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
                                                   {{0.5f,  -0.5f, -0.5f}, {0.0f, 0.0f}},
                                                   {{0.5f,  0.5f,  -0.5f}, {0.0f, 1.0f}},
                                                   {{-0.5f, 0.5f,  -0.5f}, {1.0f, 1.0f}}};

const std::vector<uint16_t> PLATE_SAMPLE_INDICES = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

const std::vector<Vertex> CUBE_VERTICES = {{{-0.5f, 0.5f,  -0.5f}, {0.0f, 0.0f}},
                                           {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
                                           {{0.5f,  -0.5f, -0.5f}, {1.0f, 1.0f}},
                                           {{0.5f,  0.5f,  -0.5f}, {1.0f, 0.0f}},

                                           {{-0.5f, 0.5f,  0.5f},  {0.0f, 0.0f}},
                                           {{-0.5f, -0.5f, 0.5f},  {0.0f, 1.0f}},
                                           {{0.5f,  -0.5f, 0.5f},  {1.0f, 1.0f}},
                                           {{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},

                                           {{0.5f,  0.5f,  -0.5f}, {0.0f, 0.0f}},
                                           {{0.5f,  -0.5f, -0.5f}, {0.0f, 1.0f}},
                                           {{0.5f,  -0.5f, 0.5f},  {1.0f, 1.0f}},
                                           {{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},

                                           {{-0.5f, 0.5f,  -0.5f}, {0.0f, 0.0f}},
                                           {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
                                           {{-0.5f, -0.5f, 0.5f},  {1.0f, 1.0f}},
                                           {{-0.5f, 0.5f,  0.5f},  {1.0f, 0.0f}},

                                           {{-0.5f, 0.5f,  0.5f},  {0.0f, 0.0f}},
                                           {{-0.5f, 0.5f,  -0.5f}, {0.0f, 1.0f}},
                                           {{0.5f,  0.5f,  -0.5f}, {1.0f, 1.0f}},
                                           {{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},

                                           {{-0.5f, -0.5f, 0.5f},  {0.0f, 0.0f}},
                                           {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
                                           {{0.5f,  -0.5f, -0.5f}, {1.0f, 1.0f}},
                                           {{0.5f,  -0.5f, 0.5f},  {1.0f, 0.0f}}};

const std::vector<uint16_t> CUBE_INDICES = {0, 1, 3, 3, 1, 2, 4, 5, 7, 7, 5, 6, 8, 9, 11, 11, 9, 10, 12, 13, 15, 15, 13,
                                            14, 16, 17, 19, 19, 17, 18, 20, 21, 23, 23, 21, 22};

const std::vector<Vertex> TRIANGLE_VERTICES = {{{0.0f,  -0.5f, 1.0f}, {1.0f, 0.0f}},
                                               {{0.5f,  0.5f,  1.0f}, {0.0f, 1.0f}},
                                               {{-0.5f, 0.5f,  1.0f}, {0.0f, 0.0f}}};

const std::vector<uint16_t> TRIANGLE_INDICES = {0, 1, 2, 0, 3, 1};

const std::vector<Vertex> SCREEN_QUAD_VERTICES = {{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                                  {{1.0f,  -1.0f, 0.0f}, {1.0f, 0.0f}},
                                                  {{1.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
                                                  {{-1.0f, 1.0f,  0.0f}, {0.0f, 1.0f}}};

const std::vector<uint16_t> SCREEN_QUAD_INDICES = {0, 1, 2, 2, 3, 0};

#endif //VCW_APP_H
