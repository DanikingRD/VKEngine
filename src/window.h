#ifndef WINDOW_H
#define WINDOW_H
// #define GLFW_INCLUDE_VULKAN
#include "renderer/vulkan/vulkan_types.h"
#include "types.h"
#include <GLFW/glfw3.h>
#include <collections/vector.h>
typedef GLFWwindow Window;

Window* create_window(u32 width, u32 height, const char* name);
bool window_should_close(Window* window);
void window_poll_events(void);
void window_required_vulkan_extensions(Vector(const char*) * extensions);
void window_create_vulkan_surface(Window* window, VulkanBackend* backend);
void window_get_framebuffer_size(Window* window, i32* width, i32* height);
void window_destroy(Window* window);

#endif
