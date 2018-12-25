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

static bool map_get_index(int* result, struct map map, void* key)
{
    for (int i = 0; i < vec_length(map.keys); i++)
    {
        if (map.comparator(vec_elem(map.keys, i), key) == 0)
        {
            *result = i;
            return true;
        }
    }

    return false;
}

struct map map_init(int32_t key_size, int32_t value_size,
                    int (*comparator)(void* key1, void* key2))
{
    struct map map;
    map.keys = vec_init(key_size);
    map.values = vec_init(value_size);
    map.comparator = comparator;
    return map;
}

void map_free(struct map* map_p)
{
    struct map map = *map_p;
    vec_destroy(&map.keys);
    vec_destroy(&map.values);
    *map_p = map;
}

bool map_get(void* result, struct map map, void* key)
{
    int index;
    if (map_get_index(&index, map, key))
    {
        memmove(result, vec_elem(map.values, index), map.values.elem_size);
        return true;
    }
    else
    {
        return false;
    }
}

void map_set(struct map* map_p, void* key, void* value)
{
    assert(map_p != NULL);
    struct map map = *map_p;

    int index;
    if (map_get_index(&index, map, key))
    {
        vec_set(&map.values, index, value);
    }
    else
    {
        vec_append(&map.keys, key);
        vec_append(&map.values, value);
        *map_p = map;
    }
}

struct map_iterator map_iterator(struct map map)
{
    struct map_iterator iterator =
    {
        .map = map,
        .vector_index = -1
    };
    return iterator;
}

bool map_iterator_next(struct map_entry* result,
                       struct map_iterator* iterator_p)
{
    iterator_p->vector_index++;
    if (vec_length(iterator_p->map.keys) <= iterator_p->vector_index)
    {
        return false;
    }

    struct map_entry entry;
    entry.key = vec_elem(iterator_p->map.keys, iterator_p->vector_index);
    entry.value = vec_elem(iterator_p->map.values, iterator_p->vector_index);
    *result = entry;
    return true;
}
