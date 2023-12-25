#include "vector.h"
#include "core/log.h"
#include "core/mem.h"
#include <core/log.h>
#include <string.h>

typedef struct VectorHeader {
    u64 capacity;
    u64 length;
    u64 stride;
} VectorHeader;

Vector(T) _vector_new(u64 length, u64 stride) {
    if (length == 0) {
        WARN("Attempted to allocate a vector with 0 length. This will do nothing.");
        return 0;
    }
    u64 header_size = sizeof(VectorHeader);
    u64 array_size = length * stride;
    VectorHeader* header = mem_alloc(header_size + array_size);
    header->length = 0;
    header->capacity = length;
    header->stride = stride;
    return (void*)((u8*)header + header_size);
}

void _vector_free(Vector(T) array) {
    if (array) {
        u64 header_size = sizeof(VectorHeader);
        VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
        mem_free(header);
    }
}

Vector(T) _vector_resize(Vector(T) array) {
    u64 header_size = sizeof(VectorHeader);
    VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
    if (header->capacity == 0) {
        WARN("Attempted to resize vector with 0 capacity. This will do nothing.");
        return 0;
    }
    Vector(T) vector = _vector_new((VECTOR_RESIZE_FACTOR * header->capacity), header->stride);

    header = (VectorHeader*)((u8*)array - header_size);
    memcpy(vector, array, header->length * header->stride);

    vector_length_set(vector, header->length);
    vector_free(array);
    return vector;
}

void* _vector_push(void* array, const void* value_ptr) {
    u64 header_size = sizeof(VectorHeader);
    VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
    if (header->length >= header->capacity) {
        array = _vector_resize(array);
    }
    header = (VectorHeader*)((u8*)array - header_size);

    u64 addr = (u64)array;
    addr += (header->length * header->stride);
    memcpy((void*)addr, value_ptr, header->stride);
    vector_length_set(array, header->length + 1);
    return array;
}

void vector_pop(Vector(T) array, T* dest) {
    u64 length = vector_length(array);
    u64 stride = vector_stride(array);
    if (length < 1) {
        WARN("Attempted to pop an empty vector. This will do nothing.");
        return;
    }
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    memcpy(dest, (void*)addr, stride);
    vector_length_set(array, length - 1);
}

Vector(T) vector_remove(Vector(T) array, u64 index, T* dest) {
    u64 length = vector_length(array);
    u64 stride = vector_stride(array);
    if (index >= length) {
        ERROR("Index out of bounds. Length: %d, index: %d", length, index);
        return array;
    }
    u64 addr = (u64)array;
    memcpy(dest, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if (index != length - 1) {
        memcpy((void*)(addr + (index * stride)), (void*)(addr + ((index + 1) * stride)),
               stride * (length - (index - 1)));
    }
    vector_length_set(array, length - 1);
    return array;
}
// TODO: is this necessary?
// void* _vector_insert_at(void* array, u64 index, void* value_ptr) {
//     u64 length = vector_length(array);
//     u64 stride = vector_stride(array);
//     if (index >= length) {
//         return array;
//     }
//     if (length >= vector_capacity(array)) {
//         array = _vector_resize(array);
//     }

//     u64 addr = (u64)array;

//     // Push element(s) from index forward out by one. This should
//     // even happen if inserted at the last index.
//     memcpy((void*)(addr + ((index + 1) * stride)), (void*)(addr + (index * stride)),
//            stride * (length - index));

//     // Set the value at the index
//     memcpy((void*)(addr + (index * stride)), value_ptr, stride);

//     vector_length_set(array, length + 1);
//     return array;
// }

void vector_clear(void* array) { vector_length_set(array, 0); }

u64 vector_capacity(void* array) {
    u64 header_size = sizeof(VectorHeader);
    VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
    return header->capacity;
}

u64 vector_length(void* array) {
    if (array) {
        u64 header_size = sizeof(VectorHeader);
        VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
        return header->length;
    }
    return 0;
}

u64 vector_stride(void* array) {
    if (array) {
        u64 header_size = sizeof(VectorHeader);
        VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
        return header->stride;
    }
    return 0;
}

void vector_length_set(void* array, u64 value) {
    if (array) {
        u64 header_size = sizeof(VectorHeader);
        VectorHeader* header = (VectorHeader*)((u8*)array - header_size);
        header->length = value;
    }
}
