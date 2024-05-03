//
// Created by Ludw on 4/25/2024.
//

#include "../../app.h"

void App::frame_buf_resized(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    app->resized = true;
}

void App::cursor_callback(GLFWwindow *window, double nx, double ny) {
    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    glm::vec2 new_pos = {nx, ny};
#ifdef USE_CAMERA
    if (!app->cursor_enabled) {
        glm::vec2 delta = new_pos - app->cursor_pos;
        app->cam.update_cam_rotation(delta.x, delta.y);
    }
#endif
    app->cursor_pos = new_pos;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        app->cursor_enabled = true;
    } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        app->cursor_enabled = false;
        glfwSetCursorPos(window, app->cursor_pos.x, app->cursor_pos.y);
    } else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        if (glfwGetWindowMonitor(window) == nullptr) {
            glfwGetWindowPos(window, &app->window_pos.x, &app->window_pos.y);
            glfwGetWindowSize(window, &app->window_dim.x, &app->window_dim.y);

            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            int actual_width = mode->width / FULLSCREEN_RES_DIV;
            int actual_height = mode->height / FULLSCREEN_RES_DIV;

            glfwSetWindowMonitor(window, monitor, 0, 0, actual_width, actual_height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(window, nullptr, app->window_pos.x, app->window_pos.y, app->window_dim.x, app->window_dim.y, GLFW_DONT_CARE);
        }
    }
}

void App::init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, frame_buf_resized);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetKeyCallback(window, key_callback);
}

std::vector<const char *> App::get_required_exts() {
    uint32_t glfw_ext_count = 0;
    const char **glfw_exts;
    glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    std::vector<const char *> exts(glfw_exts, glfw_exts + glfw_ext_count);

#ifdef VALIDATION
    exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return exts;
}

void App::create_inst() {
#ifdef VALIDATION
    if (!check_validation_support())
        throw std::runtime_error("validation layers requested, but not available.");
#endif

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = APP_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = ENGINE_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo inst_info{};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pApplicationInfo = &app_info;

    auto exts = get_required_exts();
    inst_info.enabledExtensionCount = static_cast<uint32_t>(exts.size());
    inst_info.ppEnabledExtensionNames = exts.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_info{};
#ifdef VALIDATION
    inst_info.enabledLayerCount = static_cast<uint32_t>(val_layers.size());
    inst_info.ppEnabledLayerNames = val_layers.data();

    get_debug_msg_info(debug_info);
    inst_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debug_info;
#else
    inst_info.enabledLayerCount = 0;
    inst_info.pNext = nullptr;
#endif

    if (vkCreateInstance(&inst_info, nullptr, &inst) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance.");
}

void App::setup_debug_msg() {
#ifdef VALIDATION
    VkDebugUtilsMessengerCreateInfoEXT debug_info;
    get_debug_msg_info(debug_info);

    if (create_debug_callback(inst, &debug_info, nullptr, &debug_msg) != VK_SUCCESS)
        throw std::runtime_error("failed to set up debug messenger.");
#endif
}

void App::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
#ifdef IMGUI_SCALE_OVERLAY
    ImGui::GetIO().FontGlobalScale = 2.0f;
#else
    ImGui::GetIO().FontGlobalScale = 1.0f;
#endif

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo imgui_init_info{};
    imgui_init_info.Instance = inst;
    imgui_init_info.PhysicalDevice = phy_dev;
    imgui_init_info.Device = dev;
    imgui_init_info.QueueFamily = qf_indices.qf_graph.value();
    imgui_init_info.Queue = q_graph;
    imgui_init_info.DescriptorPool = desc_pool;
    imgui_init_info.RenderPass = rendp;
    imgui_init_info.Subpass = 0;
    imgui_init_info.MinImageCount = static_cast<uint32_t>(swap_imgs.size());
    imgui_init_info.ImageCount = static_cast<uint32_t>(swap_imgs.size());
    imgui_init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&imgui_init_info);

    ImGui_ImplVulkan_CreateFontsTexture();
}