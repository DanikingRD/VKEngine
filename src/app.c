#include "app.h"
#include "core/event.h"
#include "core/input.h"
#include "core/log.h"
#include "renderer/renderer.h"
#include "window.h"

typedef struct App {
    Window* window;
} App;

App app = {0};

static void window_close_callback(EventCode code, EventMessage message);
static void key_press_callback(EventCode code, EventMessage message);
static void application_on_resized(EventCode code, EventMessage message);

bool application_initialize(AppConfig* config) {
    logger_create();
    INFO("Initializing...");
    Window* window =
        create_window(config->initial_window_width, config->initial_window_height, config->title);
    if (!window) {
        return false;
    }
    app.window = window;
    INFO("Window created");
    event_manager_create();
    event_manager_register(EVENT_CODE_GAME_EXIT, window_close_callback);
    event_manager_register(EVENT_CODE_KEY_PRESS, key_press_callback);
    event_manager_register(EVENT_CODE_KEY_RELEASE, key_press_callback);
    event_manager_register(EVENT_CODE_WINDOW_RESIZE, application_on_resized);
    input_manager_create();

    if (!renderer_create(config->title, app.window)) {
        ERROR("Failed to create renderer.");
        return false;
    }
    return true;
}

bool application_run(void) {
    INFO("Game running...");
    f32 dt = 0.0f;
    while (!window_should_close(app.window)) {
        window_poll_events();
        renderer_render(dt);
    }
    event_manager_destroy();
    input_manager_destroy();
    renderer_destroy();
    window_destroy(app.window);
    logger_destroy();
    return true;
}

static void application_on_resized(EventCode code, EventMessage message) {
    u16 w = message.data.u16[0];
    u16 h = message.data.u16[1];
    renderer_resize(w, h);
    INFO("Application resized.");
}

static void window_close_callback(EventCode code, EventMessage message) {
    DEBUG("Application close requested");
}

static void key_press_callback(EventCode code, EventMessage message) {
    u32 key = message.data.u32[0];
    DEBUG("[%c] pressed.", key);
}
