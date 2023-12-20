#include "instant.h"
#include "platform/platform.h"

void instant_now(Instant* instant) { instant->start = platform_system_time(); }

f64 instant_elapsed(Instant* instant) { return platform_system_time() - instant->start; }
