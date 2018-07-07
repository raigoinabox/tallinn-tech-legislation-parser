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

struct map
{
    struct vector keys;
    struct vector values;
    int (*comparator)(void* key1, void* key2);
};

struct map_iterator
{
    struct map map;
    int32_t vector_index;
};

struct map map_init(int32_t key_size, int32_t value_size,
                    int (*comparator)(void* key1, void* key2));
void map_free(struct map* map_p);
bool map_get(void* result, struct map map, void* key);
void map_set(struct map* map_p, void* key, void* value);
struct map_iterator map_iterator(struct map map);
bool map_iterator_next(struct map_entry* result,
                       struct map_iterator* iterator_p);

#endif /* MAPS_H_ */
