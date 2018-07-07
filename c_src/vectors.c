/*
 * vectors.c
 *
 *  Created on: 26. mai 2018
 *      Author: raigo
 */

#include "vectors.h"

#include <string.h>

void vec_init(struct vector* vector, int elem_size)
{
    vec_init_size(vector, elem_size, 16);
}

void vec_init_size(struct vector* vec_p, int elem_size, int size)
{
    assert(0 < size);
    assert(0 < elem_size);
    struct vector vec = *vec_p;
    vec.capacity = size;
    vec.elem_size = elem_size;
    vec.length = 0;
    vec.content = malloc_a(size, elem_size);
    *vec_p = vec;
}

void vec_c(struct vector* vec_p, const void* content, int length, int elem_size)
{
    assert(content != NULL);
    assert(0 <= length);
    assert(0 < elem_size);
    struct vector vec = *vec_p;
    vec.capacity = length;
    vec.content = (void*) content;
    vec.elem_size = elem_size;
    vec.length = length;
    *vec_p = vec;
}

void vec_destroy(struct vector* vector_p)
{
    struct vector vector = *vector_p;
    free(vector.content);
    vector.capacity = 0;
    vector.content = NULL;
    vector.length = 0;
    *vector_p = vector;
}

void vec_reserve(struct vector* vector_p, vec_size elem_count)
{
    struct vector vector = *vector_p;
    int necessary_capacity = vector.length + elem_count;
    if (vector.capacity < necessary_capacity)
    {
        vector.capacity = necessary_capacity * 2;
        vector.content = realloc_a(vector.content, vector.capacity,
                                   vector.elem_size);
    }
    *vector_p = vector;
}

void* vec_elem(struct vector vector, int index)
{
    assert(0 <= index);
    assert(index < vector.length);
    return (char*) vector.content + (index * vector.elem_size);
}

void vec_set(struct vector* vector_p, int index, void* elem)
{
    struct vector vector = *vector_p;
    void* dest = vec_elem(vector, index);
    memmove(dest, elem, vector.elem_size);
}

void vec_append(struct vector* vec_p, void* elem)
{
    vec_reserve(vec_p, 1);
    vec_p->length += 1;
    vec_set(vec_p, vec_p->length - 1, elem);
}

int vec_length(struct vector vec)
{
    return vec.length;
}

int vec_capacity(struct vector vec)
{
    return vec.capacity;
}

struct vector vec_subvector(struct vector vector, int begin_index)
{
    return vec_subvector_end(vector, begin_index, vector.length);
}

struct vector vec_subvector_end(struct vector vector, int begin_index,
                                int end_index)
{
    assert(0 <= begin_index);
    assert(0 <= end_index);
    assert(begin_index < vector.length);
    assert(end_index <= vector.length);

    struct vector subvector = vector;
    subvector.content = vec_elem(vector, begin_index);
    subvector.length = end_index - begin_index;
    return subvector;
}

struct vec_iterator vec_iterator(struct vector vector)
{
    struct vec_iterator iterator;
    iterator.vector = vector;
    iterator.index = -1;
    return iterator;
}

bool vec_iter_next(void* result, struct vec_iterator* iterator)
{
    iterator->index++;
    if (vec_length(iterator->vector) <= iterator->index)
    {
        return false;
    }

    memmove(result, vec_elem(iterator->vector, iterator->index),
            iterator->vector.elem_size);
    return true;
}
