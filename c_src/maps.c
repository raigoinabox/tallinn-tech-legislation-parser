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

struct map map_init(int32_t key_size, int32_t value_size,
                    int (*comparator)(void* key1, void* key2, int (*type_comp)()),
                    int (*type_comparator)())
{
    struct map map =
    {
        .key_size = key_size,
        .value_size = value_size,
        .comparator = comparator,
        .type_comparator = type_comparator
    };
    vec_init(map.entries);
    return map;
}

void map_free(struct map* map_p)
{
    struct map map = *map_p;
    for (int32_t i = 0; i < vec_length(map.entries); i++)
    {
        struct map_entry map_entry = vec_elem(map.entries, i);
        free(map_entry.key);
        map_entry.key = NULL;
        free(map_entry.value);
        map_entry.value = NULL;
    }
    vec_free(map.entries);
    *map_p = map;
}

bool map_get_value_p(void** result, struct map map, void* key)
{
    for (int32_t i = 0; i < vec_length(map.entries); i++)
    {
        struct map_entry map_entr = vec_elem(map.entries, i);
        if (map.comparator(map_entr.key, key, map.type_comparator) == 0)
        {
            *result = map_entr.value;
            return true;
        }
    }

    return false;
}

bool map_get_value(void* result, struct map map, void* key)
{
    void* value_p;
    if (map_get_value_p(&value_p, map, key))
    {
        memmove(result, value_p, map.value_size);
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

    void* old_value_p;
    bool success = map_get_value_p(&old_value_p, map, key);
    if (success)
    {
        memmove(old_value_p, value, map.value_size);
    }
    else
    {
        struct map_entry map_entry =
        {
            .key = malloc_a(1, map.key_size),
            .value = malloc_a(1, map.value_size)
        };
        memmove(map_entry.key, key, map.key_size);
        memmove(map_entry.value, value, map.value_size);
        vec_append(map.entries, map_entry);
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
    return iterator.vector_index < vec_length(iterator.map.entries) - 1;
}

void* map_iterator_get_key_p(struct map_iterator iterator)
{
    return vec_elem(iterator.map.entries, iterator.vector_index).key;
}

void* map_iterator_get_value_p(struct map_iterator iterator)
{
    return vec_elem(iterator.map.entries, iterator.vector_index).value;
}

void map_iterator_set(struct map_iterator* iterator_p, void* value)
{
    struct map_iterator iterator = *iterator_p;
    map_set(&iterator.map, map_iterator_get_key_p(iterator), value);
    *iterator_p = iterator;
}
