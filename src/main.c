#include "app.h"
#include "core/log.h"

int main(void) {

    AppConfig config;
    config.initial_window_width = 1280;
    config.initial_window_height = 720;
    config.title = "VoxelGame";

    if (!application_initialize(&config)) {
        ERROR("Failed to initialize application.");
        return 1;
    }
    if (!application_run()) {
        ERROR("Failed to run application.");
        return 1;
    }
    return 0;
}
