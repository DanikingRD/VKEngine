#ifndef VECTOR_H
#define VECTOR_H

#include "types.h"

#define VECTOR_RESIZE_FACTOR 2
#define Vector(type) type*
#define T void

/**
 * @brief Creates a new vector of the given length and stride.
 * Note that this performs a dynamic memory allocation.
 * @note Avoid using this directly; use the vector_new macro instead.
 * @param length The default number of elements in the array.
 * @param stride The size of each array element.
 * @returns A pointer representing the block of memory containing the array.
 */
Vector(T) _vector_new(u64 length, u64 stride);
Vector(T) _vector_resize(Vector(T) array);
Vector(T) _vector_push(Vector(T) array, const T* value_ptr);

/* void* _vector_insert_at(void* array, u64 index, void* value_ptr);
 */
/**
 * @brief Creates a new vector of the given type with the provided capacity.
 * Performs a dynamic memory allocation.
 * @param type The type to be used to create the vector.
 * @param capacity The number of elements the vector can initially hold (can be resized).
 * @returns A pointer to the array's memory block.
 */
#define vector_with_capacity(type, capacity) _vector_new(capacity, sizeof(type))

/**
 * @brief Creates a new vector of the given type with the default capacity.
 * Performs a dynamic memory allocation.
 * @param type The type to be used to create the vector.
 * @returns A pointer to the array's memory block.
 */
#define vector_new(type) vector_with_capacity(type, 1)

/**
 * @brief Destroys the provided array, freeing any memory allocated by it.
 * @param array The array to be destroyed.
 */
void _vector_free(void* array);

#define vector_free(array)                                                                         \
    {                                                                                              \
        _vector_free(array);                                                                       \
        array = 0;                                                                                 \
    }

#define vector_push(array, value)                                                                  \
    {                                                                                              \
        __typeof__(value) temp = value;                                                            \
        array = _vector_push(array, &temp);                                                        \
    }

void vector_pop(Vector(T) array, T* value_ptr);

#define vector_insert_at(array, index, value)                                                      \
    {                                                                                              \
        typeof(value) temp = value;                                                                \
        array = _vector_insert_at(array, index, &temp);                                            \
    }

Vector(T) vector_remove(Vector(T) array, u64 index, T* value_ptr);

/**
 * @brief Clears all entries from the array. Does not release any internally-allocated memory.
 * @param array The array to be cleared.
 */
void vector_clear(void* array);

/**
 * @brief Gets the given array's capacity.
 * @param array The array whose capacity to retrieve.
 * @returns The capacity of the given array.
 */
u64 vector_capacity(void* array);

/**
 * @brief Gets the length (number of elements) in the given array.
 * @param array The array to obtain the length of.
 * @returns The length of the given array.
 */
u64 vector_length(void* array);

/**
 * @brief Gets the stride (element size) of the given array.
 * @param array The array to obtain the stride of.
 * @returns The stride of the given array.
 */
u64 vector_stride(void* array);

/**
 * @brief Sets the length of the given array. This ensures the array has the
 * required capacity to be able to set entries directly, for instance. Can trigger
 * an internal reallocation.
 * @param array The array to set the length of.
 * @param value The length to set the array to.
 */
void vector_length_set(void* array, u64 value);

#endif
