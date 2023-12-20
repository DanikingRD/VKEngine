#ifndef INSTANT_H
#define INSTANT_H

#include "types.h"

/**
 * Represents a system instant which allows for tracking delta times.
 */
typedef struct Instant {
    f64 start;
} Instant;

/**
 *  Returns the current platform `instant`.
 */
void instant_now(Instant* instant);

/**
 *  Returns the amount of time elapsed since the given instant
 */
f64 instant_elapsed(Instant* instant);

#endif
