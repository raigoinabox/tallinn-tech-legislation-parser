/*
 * dyn_arrays.h
 *
 *  Created on: 18. jaan 2018
 *      Author: raigo
 */

#ifndef VECTORS_H_
#define VECTORS_H_

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "util.h"

typedef ptrdiff_t vec_size;

#define _vec_def_array_size 16

#define vec_capacity(vector) \
	(vector).capacity

#define _vec_elem_size(vector) \
	sizeof(*vec_content(vector))

#define _vec_elem(vector, index) \
	vec_content(vector)[index]

#define _vec_init(vector, content, length, size) \
	(assert(0 < (size)), vec_content(vector) = (content), \
			vec_length(vector) = (length), \
			vec_capacity(vector) = (size))

#define _vec_expand(vector) \
	(vec_capacity(vector) *= 2, \
			vec_content(vector) = realloc_a(vec_content(vector), \
					vec_capacity(vector), _vec_elem_size(vector)))

#define _assert_index(vector, index) \
	(assert((index) < vec_length(vector)))


#define vector(type) \
    vec_struct(, type)

#define vec_struct(name, type) \
	struct name { \
		type* content; \
		vec_size capacity; \
		vec_size length; \
	}

#define vec_init(vector) \
        vec_init_size(vector, _vec_def_array_size)

#define vec_init_size(vector, size) \
	_vec_init(vector, \
			malloc_a((size), _vec_elem_size(vector)), \
			0, \
			size)

#define vec_init_sta(vector, buffer, size) \
	_vec_init(vector, buffer, 0, size)

#define vec_init_c(vector, content, length) \
	_vec_init(vector, content, length, length)


#define vec_free(vector) \
	(free(vec_content(vector)), \
	vec_content(vector) = NULL, \
	vec_length(vector) = 0, \
	vec_capacity(vector) = 0)

#define vec_length(vector) \
	(vector).length

#define vec_content(vector) \
	(vector).content

#define vec_elem(vector, index) \
	(_assert_index(vector, index), \
			_vec_elem(vector, index))

#define vec_set(vector, index, element) \
	(_assert_index(vector, index), \
			_vec_elem(vector, index) = element)

#define vec_append(vector, element) \
	vec_capacity(vector) <= vec_length(vector) \
	? _vec_expand(vector) \
	: 0, \
	vec_length(vector) += 1, \
	_vec_elem(vector, vec_length(vector) - 1) = element

#define vec_remove(vector, index) \
	(_assert_index(vector, index), \
			(index) < vec_length(vector) - 1 \
			? memmove(vec_content(vector) + (index), \
					vec_content(vector) + (index) + 1, \
					sizeof(*vec_content(vector)) \
					* (vec_length(vector) - (index) - 1)) \
			: 0, \
			vec_length(vector)--)

struct vector2 {
    void* content;
    vec_size capacity;
    vec_size length;
    int elem_size;
};

void vec_init_size2(struct vector2* vec_p, int size, int elem_size);
void vec_init_c2(struct vector2* vec_p, const void* content, int length, int elem_size);
void _vec_free(void* vector_p);
void vec_reserve(struct vector2* vector_p, vec_size elem_count);
void vec_set2(struct vector2* vector_p, int index, void* elem);
void vec_append2(struct vector2* vec_p, void* elem);
int vec_len(struct vector2 vec);
int vec_cap(struct vector2 vec);

#endif /* VECTORS_H_ */
