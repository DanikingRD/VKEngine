#ifndef RENDERER_BACKEND_H
#define RENDERER_BACKEND_H

#include "math/lineal_types.h"
#include "types.h"
#include "window_types.h"

typedef enum RenderBackend {
    RENDER_BACKEND_VULKAN,
    RENDER_BACKEND_OPENGL,
} RenderBackend;

typedef struct GlobalsUBO {
    Mat4 proj;  // 64 bytes
    Mat4 view;  // 64 bytes
    Mat4 pad_0; // 64 bytes, reserved
    Mat4 pad_1; // 64 bytes, reserved
} GlobalsUBO;

typedef struct RendererBackend {
    RenderBackend type;
    bool (*create)(const char* app_name, Window* window);
    void (*resize)(u16 width, u16 height);
    bool (*begin_frame)(f32 dt);
    void (*update_globals)(Mat4 proj, Mat4 view);
    bool (*end_frame)(f32 dt);
    void (*destroy)(void);
} RendererBackend;

bool renderer_backend_setup(const char* app_name, Window* window, RendererBackend* backend);
void renderer_backend_reset(RendererBackend* backend);

#endif