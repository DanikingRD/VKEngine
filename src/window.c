#include "window.h"
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
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    if (!window) {
        ERROR("Failed to create window.");
        glfwTerminate();
        return 0;
    }
    return window;
}

void window_get_framebuffer_size(Window* window, i32* width, i32* height) {
    glfwGetFramebufferSize(window, width, height);
}

static void window_error_callback(int code, const char* description) {
    ERROR("[GLFW] -> %s", description);
    UNUSED(code);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    UNUSED(window);
    UNUSED(key);
    UNUSED(scancode);
    UNUSED(action);
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
static void window_close_callback(Window* window) { UNUSED(window); }

bool window_should_close(Window* window) { return glfwWindowShouldClose(window); }
void window_poll_events(void) { glfwPollEvents(); }

void destroy_window(Window* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
    INFO("Window destroyed.");
}
