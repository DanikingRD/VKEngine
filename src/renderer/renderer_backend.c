#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"

bool renderer_backend_setup(const char* app_name, Window* window, RendererBackend* backend) {
    if (backend->type == RENDER_BACKEND_VULKAN) {
        backend->create = vulkan_backend_create;
        backend->resize = vulkan_backend_resize;
        backend->begin_frame = vulkan_backend_begin_frame;
        backend->end_frame = vulkan_backend_end_frame;
        backend->destroy = vulkan_backend_destroy;
        return true;
    }
    return false;
}

void renderer_backend_reset(RendererBackend* backend) {
    backend->create = 0;
    backend->resize = 0;
    backend->begin_frame = 0;
    backend->end_frame = 0;
    backend->destroy = 0;
}
