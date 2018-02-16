/*
 * dyn_arrays.h
 *
 *  Created on: 18. jaan 2018
 *      Author: raigo
 */

#ifndef VECTORS_H_
#define VECTORS_H_

#include <stdbool.h>
#include <stdint.h>

#define VECTOR_STRUCT(type, name) \
		struct name { type* content, size_t vector_size, size_t vector_length };

#define VECTOR_DECLARE(modifier, type, name) \
	struct name { \
		struct vector parent; \
	}; \
	modifier struct name name ## _init(); \
	modifier void name ## _free(struct name* name); \
	modifier type name ## _get(struct name name, int32_t index); \
	modifier void name ## _set(struct name* name, int32_t index, type element); \
	modifier void name ## _append(struct name* name, type element); \
	modifier void name ## _remove(struct name* name, int32_t index); \
	modifier int32_t name ## _length(struct name name); \
	modifier bool name ## _contains(struct name name, type element, \
			int (*comparator)(type element1, type element2));


#define VECTOR_DEFINE(modifier, type, name) \
	static int _vector_ ## name ## _compare(void* element1, void* element2, \
			int (*type_comparator)(type element1, type element2)) \
	{ \
		return type_comparator(*(type*)element1, *(type*)element2); \
	} \
	modifier struct name name ## _init() \
	{ \
		return (struct name) { vector_init(sizeof(type)) }; \
	} \
	modifier void name ## _free(struct name* name) { \
		vector_free(&name->parent); \
	} \
	modifier type name ## _get(struct name name, int32_t index) \
	{ \
		return * (type*) vector_get_element_p(name.parent, index); \
	} \
	modifier void name ## _set(struct name* name, int32_t index, type element) \
	{ \
		vector_set(&name->parent, index, &element); \
	} \
	modifier void name ## _append(struct name* name, type element) \
	{ \
		vector_append(&name->parent, &element); \
	} \
	modifier void name ## _remove(struct name* name, int32_t index) \
	{ \
		vector_remove(&name->parent, index); \
	} \
	modifier int32_t name ## _length(struct name name) \
	{ \
		return vector_length(name.parent); \
	} \
	modifier bool name ## _contains(struct name name, type element, \
			int (*comparator)(type element1, type element2)) \
	{ \
		return vector_contains(name.parent, &element, comparator, \
				_vector_ ## name ## _compare); \
	}

struct vector {
	void* content;
	int32_t vector_size;
	int32_t length;
	int32_t elem_size;
};

struct vector vector_init_size(int32_t size, int32_t elem_size);
struct vector vector_init_size2(int32_t elem_size, int32_t size);
struct vector vector_init(int32_t elem_size);
void vector_free(struct vector* vector_p);
void* vector_get_element_p(struct vector vector, int32_t index);
void vector_set(struct vector* vector_p, int32_t index, void* element);
void vector_append(struct vector* vector_p, void* element);
void vector_remove(struct vector* vector_p, int32_t index);
int32_t vector_length(struct vector vector);
bool vector_contains(struct vector vector, void* element, int (*type_comparator)(),
		int (*comparator)(void* element1, void* element2, int (*type_comparator)()));

#endif /* VECTORS_H_ */
