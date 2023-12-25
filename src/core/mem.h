#ifndef MEM_H
#define MEM_H

#include "types.h"

void* mem_alloc(u64 bytes);
void mem_free(void* block);
void mem_copy(void* dest, void* src, u64 bytes);
#endif
