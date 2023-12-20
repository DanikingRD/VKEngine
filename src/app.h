#include <types.h>
typedef struct AppConfig {
    const char* title;
    u32 initial_window_width;
    u32 initial_window_height;
} AppConfig;

bool application_initialize(AppConfig* config);
bool application_run(void);
