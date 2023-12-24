#include "window.h"
#include "core/event.h"
#include "core/input.h"
#include "core/log.h"
#include "defines.h"

static void key_callback(Window* window, int key, int scancode, int action, int mods);
static void cursor_callback(Window* window, double xpos, double ypos);
static void mouse_button_callback(Window* window, int button, int action, int mods);
static void scroll_callback(Window* window, double xoffset, double yoffset);
static void window_close_callback(Window* window);
static void window_error_callback(int code, const char* description);

Window* create_window(u32 width, u32 height, const char* name) {
    INFO("Creating window");
    if (!glfwInit()) {
        ERROR("Failed to initialize GLFW.");
        return 0;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwSetErrorCallback(window_error_callback);
    Window* window = glfwCreateWindow(width, height, name, 0, 0);
    if (!window) {
        ERROR("Failed to create window.");
        glfwTerminate();
        return 0;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    return window;
}

void window_create_vulkan_surface(Window* window, VulkanBackend* backend) {
    VK_FN_CHECK(
        glfwCreateWindowSurface(backend->instance, window, backend->allocator, &backend->surface));
}

void window_required_vulkan_extensions(Vector(const char*) * extensions) {
    u32 extension_count = 0;
    const char** out = glfwGetRequiredInstanceExtensions(&extension_count);
    if (!out) {
        ERROR("[Window] -> Failed to get required extensions");
        return;
    }
    for (u8 i = 0; i < extension_count; i++) {
        vector_push(*extensions, out[i]);
    }
}

void window_get_framebuffer_size(Window* window, i32* width, i32* height) {
    glfwGetFramebufferSize(window, width, height);
}

static void window_error_callback(int code, const char* description) {
    ERROR("[GLFW] -> %s", description);
    UNUSED(code);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    input_manager_on_key_press(key, action != GLFW_RELEASE);
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);
}
static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    UNUSED(window);
    UNUSED(xpos);
    UNUSED(ypos);
}
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    UNUSED(window);
    UNUSED(xoffset);
    UNUSED(yoffset);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    UNUSED(window);
    UNUSED(button);
    UNUSED(action);
    UNUSED(mods);
}
static void window_close_callback(Window* window) {
    UNUSED(window);
    event_manager_trigger(EVENT_CODE_GAME_EXIT, EVENT_MESSAGE_NULL);
}

bool window_should_close(Window* window) { return glfwWindowShouldClose(window); }
void window_poll_events(void) { glfwPollEvents(); }

void window_destroy(Window* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
    INFO("Window destroyed.");
}
