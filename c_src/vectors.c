/*
 * vectors.c
 *
 *  Created on: 26. mai 2018
 *      Author: raigo
 */

#include "vectors.h"

void _vec_free(void* pointer)
{
    struct _void_vec* vector_p = pointer;
    struct _void_vec vector = *vector_p;
    vec_free(vector);
    *vector_p = vector;
}
