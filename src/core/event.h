#ifndef EVENT_H
#define EVENT_H
#include "types.h"

typedef struct EventMessage {
    // Send up to a Vec3
    union {
        u32 u32[3];
        i32 i32[3];
        f32 f32[3];

        u16 u16[6];
        i16 i16[6];

        u8 u8[12];
        i8 i8[12];
    } data;
} EventMessage;

#define EVENT_MESSAGE_NULL ((EventMessage){0})

typedef enum EventCode {
    /**
     * @brief Window close event.
     */
    EVENT_CODE_GAME_EXIT,
    /**
     * @brief Key press event.
     *
     * Message details:
     * data[0] = key code
     * data[1] = whether the key was pressed or released
     */
    EVENT_CODE_KEY_PRESS,

    EVENT_CODE_KEY_RELEASE,
    EVENT_CODE_CURSOR_MOVE,
} EventCode;

typedef void (*EventCallback)(EventCode code, EventMessage message);

bool event_manager_create(void);
void event_manager_register(EventCode code, EventCallback callback);
void event_manager_destroy(void);
void event_manager_trigger(EventCode code, EventMessage message);

#endif // EVENT_H
