/*
 * maps.h
 *
 *  Created on: 27. jaan 2018
 *      Author: raigo
 */

#ifndef MAPS_H_
#define MAPS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "vectors.h"

struct map_entry
{
	void* key;
	void* value;
};

VECTOR_DECLARE(, struct map_entry, _map)

struct map {
	struct _map map;
	size_t key_size;
	size_t value_size;
	int (*comparator)(void* elem1, void* elem2, int (*type_comparator)());
	int (*type_comparator)();
};

struct map_iterator {
	struct map map;
	int32_t vector_index;
};

#define MAP_DECLARE(modifier, key_type, value_type, name) \
	struct name { \
		struct map parent; \
	}; \
	struct name ## _iterator { \
		struct map_iterator parent; \
	}; \
	modifier struct name name ## _init(int (*comparator)(key_type key1, key_type key2)); \
	modifier void name ## _free(struct name* name); \
	modifier bool name ## _get(value_type* result, struct name name, key_type key); \
	modifier void name ## _set(struct name* name, key_type key, value_type value); \
	struct name ## _iterator name ## _iterator(struct name name); \
	modifier void name ## _iterator_next(struct name ## _iterator* iterator); \
	modifier bool name ## _iterator_has_next(struct name ## _iterator iterator); \
	modifier key_type name ## _iterator_get_key(struct name ## _iterator iterator); \
	modifier value_type name ## _iterator_get_value(struct name ## _iterator iterator);


#define MAP_DEFINE(modifier, key_type, value_type, name) \
	static int _map_ ## name ## _comparator(void* key1, void* key2, \
			int (*type_comparator)(key_type key1, key_type key2)) \
	{ \
		return type_comparator(*(key_type*)key1, *(key_type*)key2); \
	} \
	modifier struct name name ## _init(int (*comparator)(key_type key1, key_type key2)) \
	{ \
		struct name name = { \
				map_init(sizeof(key_type), sizeof(value_type), \
						_map_ ## name ## _comparator, comparator) \
		}; \
		return name; \
	} \
	modifier void name ## _free(struct name* name) \
	{ \
		map_free(&name->parent); \
	} \
	modifier bool name ## _get(value_type* result, struct name name, key_type key) \
	{ \
		void* _result = result; \
		bool success = map_get_value_p(&_result, name.parent, &key); \
		if (success) { \
			*result = *(value_type*)_result; \
			return true; \
		} else { \
			return false; \
		} \
	} \
	modifier void name ## _set(struct name* name, key_type key, value_type value) \
	{ \
		map_set(&name->parent, &key, &value); \
	} \
	struct name ## _iterator name ## _iterator(struct name name) \
	{ \
		struct name ## _iterator iterator = { \
			.parent = map_iterator(name.parent) \
		}; \
		return iterator; \
	} \
	modifier void name ## _iterator_next(struct name ## _iterator* iterator) \
	{ \
		map_iterator_next(&iterator->parent); \
	} \
	modifier bool name ## _iterator_has_next(struct name ## _iterator iterator) \
	{ \
		return map_iterator_has_next(iterator.parent); \
	} \
	modifier key_type name ## _iterator_get_key(struct name ## _iterator iterator) \
	{ \
		return * (key_type*) map_iterator_get_key_p(iterator.parent); \
	} \
	modifier value_type name ## _iterator_get_value(struct name ## _iterator iterator) \
	{ \
		return * (value_type*) map_iterator_get_value_p(iterator.parent); \
	}

#define MAP_ITERATOR_SET(iterator, value) map_iterator_set(iterator.parent, &value)

struct map map_init(int32_t key_size, int32_t value_size,
		int (*comparator)(void* key1, void* key2, int (*type_comp)()),
		int (*type_comparator)());
void map_free(struct map* map_p);
bool map_get_value_p(void** result, struct map map, void* key);
void map_set(struct map* map_p, void* key, void* value);
struct map_iterator map_iterator(struct map map);
void map_iterator_next(struct map_iterator* iterator);
bool map_iterator_has_next(struct map_iterator iterator);
void* map_iterator_get_key_p(struct map_iterator iterator);
void* map_iterator_get_value_p(struct map_iterator iterator);
void map_iterator_set(struct map_iterator* iterator, void* value);

#endif /* MAPS_H_ */
