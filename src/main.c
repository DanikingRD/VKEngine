#include "core/log.h"
#include "window.h"

int main(void) {
    INFO("Info log");
    WARN("Warn log");
    ERROR("Error log");
    DEBUG("Debug log");
    TRACE("Trace log");

    Window* window = create_window(800, 600, "Vulkan");
    while (!window_should_close(window)) {
        window_poll_events();
    }
    return 0;
}
