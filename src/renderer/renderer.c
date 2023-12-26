#include "renderer.h"
#include "core/log.h"
#include "math/lineal.h"
#include "renderer_backend.h"
static RendererBackend backend = {0};

bool renderer_create(const char* app_name, Window* window) {
    // TODO: make this configurable
    backend.type = RENDER_BACKEND_VULKAN;
    renderer_backend_setup(app_name, window, &backend);
    if (!backend.create(app_name, window)) {
        ERROR("Failed to create render system");
        return false;
    }
    return true;
}

bool renderer_render(f32 dt) {
    if (backend.begin_frame(dt)) {
        backend.update_globals(mat4_identity(), mat4_identity());
        bool is_ok = backend.end_frame(dt);
        if (!is_ok) {
            ERROR("Could not finish frame.");
            return false;
        }
    }
    return true;
}

void renderer_resize(u16 width, u16 height) { backend.resize(width, height); }

void renderer_destroy(void) {
    backend.destroy();
    renderer_backend_reset(&backend);
}
