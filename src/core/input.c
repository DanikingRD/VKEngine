#include "input.h"
#include "log.h"

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
void input_manager_on_key_press(u32 key_code, bool is_pressed) {}
void input_manager_on_cursor_move(i32 x, i32 y) {}
void input_manager_is_key_pressed(u32 key_code, bool* is_pressed) {}
void input_manager_cursor_position(i32* x, i32* y) {}