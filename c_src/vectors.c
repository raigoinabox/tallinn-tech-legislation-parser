/*
 * dyn_arrays.c
 *
 *  Created on: 18. jaan 2018
 *      Author: raigo
 */

#include "vectors.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

struct vector vector_init_size(int32_t array_size, int32_t elem_size) {
	assert(0 < array_size);
	assert(0 < elem_size);

	return (struct vector)
	{
		.content = malloc_a(array_size, elem_size),
		.vector_size = array_size,
		.length = 0,
		.elem_size = elem_size
	};
}

struct vector vector_init(int32_t elem_size)
{
	assert(0 < elem_size);

	int32_t default_array_size = 16;
	return (struct vector)
	{
		.content = malloc_a(default_array_size, elem_size),
		.vector_size = default_array_size,
		.length = 0,
		.elem_size = elem_size
	};
}

void vector_free(struct vector* vector_p)
{
	struct vector vector = *vector_p;
	free(vector.content);
	vector.content = NULL;
	vector.length = 0;
	vector.vector_size = 0;
	vector.elem_size = 0;
	*vector_p = vector;
}

void* vector_get_element_p(struct vector vector, int32_t index)
{
	assert(index < vector.length);
	assert(0 <= index);
	return (uint8_t*)vector.content + (index * vector.elem_size);
}

void vector_set(struct vector* vector_p, int32_t index, void* element)
{
	assert(vector_p != NULL);
	assert(element != NULL);
	assert(0 <= index);
	struct vector vector = *vector_p;
	assert(index < vector.length);
	memmove(vector_get_element_p(vector, index), element, vector.elem_size);
}

void vector_append(struct vector* vector_p, void* element)
{
	assert(vector_p != NULL);
	assert(element != NULL);

	struct vector vector = *vector_p;
	if (vector.vector_size <= vector.length) {
		vector.vector_size *= 2;
		vector.content =
				realloc_a(vector.content, vector.vector_size, vector.elem_size);
	}
	vector.length += 1;
	vector_set(&vector, vector.length - 1, element);
	*vector_p = vector;
}

void vector_remove(struct vector* vector_p, int32_t index)
{
	assert(vector_p != NULL);
	assert (0 <= index);
	struct vector vector = *vector_p;
	assert(index < vector.length);

	if (index < vector.length - 1)
	{
		// cannot be memcpy
		memmove(vector_get_element_p(vector, index),
				vector_get_element_p(vector, index + 1),
				vector.elem_size * (vector.length - (index + 1)));
	}
	vector.length -= 1;
	*vector_p = vector;
}

int32_t vector_length(struct vector vector)
{
	return vector.length;
}

bool vector_contains(struct vector vector, void* element, int (*type_comparator)(),
		int (*comparator)(void* element1, void* element2, int (*type_comparator)()))
{
	for (int i = 0; i < vector_length(vector); i++)
	{
		void* elem_p = vector_get_element_p(vector, i);
		if (comparator(elem_p, element, type_comparator) == 0)
		{
			return true;
		}
	}

	return false;
}
