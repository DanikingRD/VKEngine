#ifndef RENDERER_BACKEND_H
#define RENDERER_BACKEND_H

#include "types.h"
#include "window.h"

typedef enum RenderBackend {
    RENDER_BACKEND_VULKAN,
    RENDER_BACKEND_OPENGL,
} RenderBackend;

typedef struct RendererBackend {
    RenderBackend type;
    bool (*create)(const char* app_name, Window* window);
    void (*resize)(u16 width, u16 height);
    bool (*begin_frame)(f32 dt);
    bool (*end_frame)(f32 dt);
    void (*destroy)(void);
} RendererBackend;

bool renderer_backend_setup(const char* app_name, Window* window, RendererBackend* backend);
void renderer_backend_reset(RendererBackend* backend);

#endif