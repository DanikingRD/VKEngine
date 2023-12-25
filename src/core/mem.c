#include "mem.h"
#include <stdlib.h>
#include <string.h>

void* mem_alloc(u64 bytes) {
    return malloc(bytes);
    //
}

void mem_free(void* block) {
    free(block);
    //
}

void mem_copy(void* dest, void* src, u64 bytes) {
    memcpy(dest, src, bytes);
    //
}
