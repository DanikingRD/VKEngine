#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"
#include "window.h"

bool renderer_create(const char* app_name, Window* window);
void renderer_destroy(void);
bool renderer_render(f32 dt);
void renderer_resize(u16 width, u16 height);

#endif
