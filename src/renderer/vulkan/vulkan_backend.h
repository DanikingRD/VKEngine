#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H

#include "types.h"
#include "window.h"

bool vulkan_backend_create(const char* app_name, Window* window);
void vulkan_backend_resize(u16 width, u16 height);
bool vulkan_backend_begin_frame(f32 dt);
bool vulkan_backend_end_frame(f32 dt);
void vulkan_backend_destroy(void);

#endif
