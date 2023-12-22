#include "input.h"
#include "log.h"
#include "event.h"

typedef struct KeyboardState {
    bool keys[INPUT_KEY_LAST];
} KeyboardState;

typedef struct MouseState {
    i32 x;
    i32 y;
    bool buttons[INPUT_MOUSE_BUTTON_LAST];
} MouseState;

typedef struct InputManager {
    KeyboardState keyboard;
    MouseState mouse;
} InputManager;

static InputManager input;

void input_manager_create(void) { INFO("Input Manager created."); }
void input_manager_destroy(void) { INFO("Input Manager destroyed."); }

void input_manager_on_key_press(u32 key_code, bool is_pressed) {
    input.keyboard.keys[key_code] = is_pressed;
    EventMessage packet;
    packet.data.u32[0] = key_code;
    event_manager_trigger(is_pressed ? EVENT_CODE_KEY_PRESS : EVENT_CODE_KEY_RELEASE, packet);
}

void input_manager_on_cursor_move(i32 x, i32 y) {
    input.mouse.x = x;
    input.mouse.y = y;

    EventMessage packet;
    packet.data.i16[0] = x;
    packet.data.i16[1] = y;
    event_manager_trigger(EVENT_CODE_CURSOR_MOVE, packet);
}

bool input_manager_is_key_pressed(u32 key_code) {
    return input.keyboard.keys[key_code];
}

void input_manager_cursor_position(Vec2* pos) {
    *pos = (Vec2){input.mouse.x, input.mouse.y};
}
