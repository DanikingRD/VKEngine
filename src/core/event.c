#include "event.h"
#include "collections/vector.h"
#include "core/log.h"

#define MAX_CALLBACKS 32

typedef struct EventManager {
    Vector(EventCallback) callbacks[MAX_CALLBACKS];
} EventManager;

static EventManager manager = {0};

bool event_manager_create(void) {
    // Keep track of initialization order
    INFO("Creating event manager");
    return true;
}

void event_manager_register(EventCode code, EventCallback callback) {
    if (!manager.callbacks[code]) {
        manager.callbacks[code] = vector_new(EventCallback);
    }
    vector_push(manager.callbacks[code], callback);
}

void event_manager_trigger(EventCode code, EventMessage message) {
    Vector(EventCallback) callbacks = manager.callbacks[code];
    if (!callbacks) {
        WARN("No callbacks registered for event code %d", code);
        return;
    }
    for (u64 i = 0; i < vector_length(callbacks); i++) {
        EventCallback callback = callbacks[i];
        callback(code, message);
    }
}

void event_manager_destroy(void) {
    INFO("Destroying event manager");
    for (u64 i = 0; i < MAX_CALLBACKS; i++) {
        if (manager.callbacks[i]) {
            vector_free(manager.callbacks[i]);
        }
    }
}
