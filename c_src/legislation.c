/*
 * legislation.c
 *
 *  Created on: 13. jaan 2018
 *      Author: raigo
 */

#include "legislation.h"

#include <stdbool.h>

#include "strings.h"

struct leg_id leg_init_c(const char* type, const char* year, const char* number)
{
    struct leg_id leg =
    {
        .type = str_c(type),
        .year = str_c(year),
        .number = str_c(number)
    };
    return leg;
}

void leg_free(struct leg_id* legislation_p)
{
    struct leg_id legislation = *legislation_p;
    str_free(&legislation.type);
    str_free(&legislation.year);
    str_free(&legislation.number);
    *legislation_p = legislation;
}
