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

#define vec_capacity_old(vector) \
	(vector).capacity

#define _vec_elem_size(vector) \
	sizeof(*vec_content(vector))

#define _vec_elem(vector, index) \
	vec_content(vector)[index]

#define _vec_init(vector, content, length, size) \
	(assert(0 < (size)), vec_content(vector) = (content), \
			vec_length_old(vector) = (length), \
			vec_capacity_old(vector) = (size))

#define _vec_expand(vector) \
	(vec_capacity_old(vector) *= 2, \
			vec_content(vector) = realloc_a(vec_content(vector), \
					vec_capacity_old(vector), _vec_elem_size(vector)))

#define _assert_index(vector, index) \
	(assert((index) < vec_length_old(vector)))


#define vector_old(type) \
    vec_struct(, type)

#define vec_struct(name, type) \
	struct name { \
		type* content; \
		vec_size capacity; \
		vec_size length; \
	}

#define vec_init_old(vector) \
        _vec_init_size(vector, _vec_def_array_size)

#define _vec_init_size(vector, size) \
	_vec_init(vector, \
			malloc_a((size), _vec_elem_size(vector)), \
			0, \
			size)

#define vec_init_sta(vector, buffer, size) \
	_vec_init(vector, buffer, 0, size)

#define vec_free(vector) \
	(free(vec_content(vector)), \
	vec_content(vector) = NULL, \
	vec_length_old(vector) = 0, \
	vec_capacity_old(vector) = 0)

#define vec_length_old(vector) \
	(vector).length

#define vec_content(vector) \
	(vector).content

#define vec_elem_old(vector, index) \
	(_assert_index(vector, index), \
			_vec_elem(vector, index))

#define vec_set_old(vector, index, element) \
	(_assert_index(vector, index), \
			_vec_elem(vector, index) = element)

#define vec_append_old(vector, element) \
	vec_capacity_old(vector) <= vec_length_old(vector) \
	? _vec_expand(vector) \
	: 0, \
	vec_length_old(vector) += 1, \
	_vec_elem(vector, vec_length_old(vector) - 1) = element

#define vec_remove(vector, index) \
	(_assert_index(vector, index), \
			(index) < vec_length_old(vector) - 1 \
			? memmove(vec_content(vector) + (index), \
					vec_content(vector) + (index) + 1, \
					sizeof(*vec_content(vector)) \
					* (vec_length_old(vector) - (index) - 1)) \
			: 0, \
			vec_length_old(vector)--)

struct vector
{
    void* content;
    vec_size capacity;
    vec_size length;
    int elem_size;
};

struct vec_iterator
{
    struct vector vector;
    int index;
};

void vec_init(struct vector* vector_old, int elem_size);
void vec_init_size(struct vector* vec_p, int elem_size, int size);
void vec_c(struct vector* vec_p, const void* content, int length,
           int elem_size);
void vec_destroy(struct vector* vector_old);
void vec_reserve(struct vector* vector_p, vec_size elem_count);
void* vec_elem(struct vector vector_old, int index);
void vec_set(struct vector* vector_p, int index, void* elem);
void vec_append(struct vector* vec_p, void* elem);
int vec_length(struct vector vec);
int vec_capacity(struct vector vec);
struct vector vec_subvector(struct vector vector_old, int begin_index);
struct vector vec_subvector_end(struct vector vector_old, int begin_index,
                                int end_index);
struct vec_iterator vec_iterator(struct vector vector);
bool vec_iter_next(void* result, struct vec_iterator* iterator);

#endif /* VECTORS_H_ */
