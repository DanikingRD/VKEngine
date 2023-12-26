#ifndef WINDOW_H
#define WINDOW_H
#include "renderer/vulkan/vulkan_types.h"
#include "types.h"
#include "window_types.h"
#include <collections/vector.h>

Window* create_window(u32 width, u32 height, const char* name);
bool window_should_close(Window* window);
void window_poll_events(void);
void window_required_vulkan_extensions(Vector(const char*) * extensions);
void window_create_vulkan_surface(Window* window, VulkanBackend* backend);
void window_get_framebuffer_size(Window* window, u32* width, u32* height);
void window_destroy(Window* window);

#endif
