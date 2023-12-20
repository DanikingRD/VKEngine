#include "app.h"
#include "core/event.h"
#include "core/input.h"
#include "core/log.h"
#include "window.h"

typedef struct App {
    Window* window;
} App;

App app = {0};

static void window_close_callback(EventCode code, EventMessage message) {
    INFO("Window close callback");
    //
}

bool application_initialize(AppConfig* config) {
    logger_create();
    INFO("Initializing...");
    Window* window = create_window(config->initial_window_width, config->initial_window_height, config->title);
    if (!window) {
        return false;
    }
    app.window = window;
    INFO("Window created");
    event_manager_create();
    event_manager_register(EVENT_CODE_GAME_EXIT, window_close_callback);
    input_manager_create();
    return true;
}

bool application_run(void) {
    INFO("Game running...");
    while (!window_should_close(app.window)) {
        window_poll_events();
    }
    event_manager_destroy();
    input_manager_destroy();
    window_destroy(app.window);
    logger_destroy();
    return true;
}
