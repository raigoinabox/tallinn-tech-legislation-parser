/*
 * maps.c
 *
 *  Created on: 27. jaan 2018
 *      Author: raigo
 */

#include "maps.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "util.h"

VECTOR_DEFINE(, struct map_entry, _map)

struct map map_init(int32_t key_size, int32_t value_size,
		int (*comparator)(void* key1, void* key2, int (*type_comp)()),
		int (*type_comparator)())
{
	struct map map = {
			.map = _map_init(),
			.key_size = key_size,
			.value_size = value_size,
			.comparator = comparator,
			.type_comparator = type_comparator
	};
	return map;
}

void map_free(struct map* map_p)
{
	struct map map = *map_p;
	for (int i = 0; i < _map_length(map.map); i++)
	{
		struct map_entry map_entry = _map_get(map.map, i);
		free(map_entry.key);
		map_entry.key = NULL;
		free(map_entry.value);
		map_entry.value = NULL;
	}
	_map_free(&map.map);
	*map_p = map;
}

bool map_get_value_p(void** result, struct map map, void* key)
{
	for (int i = 0; i < _map_length(map.map); i++)
	{
		struct map_entry map_entr = _map_get(map.map, i);
		if (map.comparator(map_entr.key, key, map.type_comparator) == 0)
		{
			*result = map_entr.value;
			return true;
		}
	}

	return false;
}

void map_set(struct map* map_p, void* key, void* value)
{
	assert(map_p != NULL);
	struct map map = *map_p;

	void* old_value_p;
	bool success = map_get_value_p(&old_value_p, map, key);
	if (success) {
		memmove(old_value_p, value, map.value_size);
	} else {
		struct map_entry map_entry = {
				.key = malloc_a(1, map.key_size),
				.value = malloc_a(1, map.value_size)
		};
		memmove(map_entry.key, key, map.key_size);
		memmove(map_entry.value, value, map.value_size);
		_map_append(&map.map, map_entry);
		*map_p = map;
	}
}

struct map_iterator map_iterator(struct map map)
{
	struct map_iterator iterator = {
			.map = map,
			.vector_index = -1
	};
	return iterator;
}

void map_iterator_next(struct map_iterator* iterator_p)
{
	assert(iterator_p != NULL);
	struct map_iterator iterator = *iterator_p;
	assert(map_iterator_has_next(iterator));
	iterator.vector_index += 1;
	*iterator_p = iterator;
}

bool map_iterator_has_next(struct map_iterator iterator)
{
	return iterator.vector_index < _map_length(iterator.map.map) - 1;
}

void* map_iterator_get_key_p(struct map_iterator iterator)
{
	return _map_get(iterator.map.map, iterator.vector_index).key;
}

void* map_iterator_get_value_p(struct map_iterator iterator)
{
	return _map_get(iterator.map.map, iterator.vector_index).value;
}

void map_iterator_set(struct map_iterator* iterator_p, void* value)
{
	struct map_iterator iterator = *iterator_p;
	map_set(&iterator.map, map_iterator_get_key_p(iterator), value);
	*iterator_p = iterator;
}
