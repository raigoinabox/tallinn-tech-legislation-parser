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

#define _vec_def_array_size 16

#define vec_size(vector) \
	(vector).vector_size

#define _vec_elem_size(vector) \
	sizeof(*vec_content(vector))

#define _vec_elem(vector, index) \
	(vector).content[index]

#define _vec_init(vector, content, length, size) \
	(assert(0 < (size)), vec_content(vector) = (content), \
			vec_length(vector) = (length), \
			vec_size(vector) = (size))

#define _vec_expand(vector) \
	(vec_size(vector) *= 2, \
			vec_content(vector) = realloc_a(vec_content(vector), \
					vec_size(vector), _vec_elem_size(vector)))

#define _assert_index(vector, index) \
	(assert(0 <= (index)), \
			assert((index) < vec_length(vector)))



#define vec_struct(name, type) \
	struct name { \
		type* content; \
		int32_t vector_size; \
		int32_t length; \
	}

#define vec_init(vector) \
	_vec_init(vector, \
			malloc_a((_vec_def_array_size), _vec_elem_size(vector)), \
			0, \
			_vec_def_array_size)

#define vec_init_siz(vector, size) \
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
	vec_size(vector) = 0)

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
	vec_size(vector) <= vec_length(vector) \
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

#endif /* VECTORS_H_ */
