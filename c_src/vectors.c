/*
 * vectors.c
 *
 *  Created on: 26. mai 2018
 *      Author: raigo
 */

#include "vectors.h"

#include <string.h>

void vec_init_size2(struct vector2* vec_p, int size, int elem_size)
{
    assert(0 < size);
    assert(0 < elem_size);
    struct vector2 vec = *vec_p;
    vec.capacity = size;
    vec.elem_size = elem_size;
    vec.length = 0;
    vec.content = malloc_a(size, elem_size);
    *vec_p = vec;
}

void vec_init_c2(struct vector2* vec_p, const void* content, int length, int elem_size)
{
    assert(content != NULL);
    assert(0 <= length);
    assert(0 < elem_size);
    struct vector2 vec = *vec_p;
    vec.capacity = length;
    vec.content = (void*) content;
    vec.elem_size = elem_size;
    vec.length = length;
    *vec_p = vec;
}

void _vec_free(void* pointer)
{
    struct vector2* vector_p = pointer;
    struct vector2 vector = *vector_p;
    vec_free(vector);
    *vector_p = vector;
}

void vec_reserve(struct vector2* vector_p, vec_size elem_count)
{
    struct vector2 vector = *vector_p;
    int new_capacity = vector.capacity;
    while (new_capacity - vector.length < elem_count)
    {
        new_capacity *= 2;
    }

    if (vector.capacity < new_capacity)
    {
        vector.capacity = new_capacity;
        vector.content = realloc_a(vector.content, vector.capacity,
                vector.elem_size);
    }
    *vector_p = vector;
}

void vec_set2(struct vector2* vector_p, int index, void* elem)
{
    assert(0 <= index);
    struct vector2 vector = *vector_p;
    assert(index < vector.length);
    void* dest = (char*) vector.content + (index * vector.elem_size);
    memmove(dest, elem, vector.elem_size);
}

void vec_append2(struct vector2* vec_p, void* elem)
{
    vec_reserve(vec_p, 1);
    vec_p->length += 1;
    vec_set2(vec_p, vec_p->length - 1, elem);
}

int vec_len(struct vector2 vec)
{
    return vec.length;
}

int vec_cap(struct vector2 vec)
{
    return vec.capacity;
}
