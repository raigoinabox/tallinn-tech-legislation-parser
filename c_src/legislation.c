/*
 * legislation.c
 *
 *  Created on: 13. jaan 2018
 *      Author: raigo
 */

#include "legislation.h"

#include <stdbool.h>

#include "strings.h"

struct leg_id leg_init(char* type, char* year, char* number)
{
    struct leg_id leg =
    {
        .type = str_c(type),
        .year = str_c(year),
        .number = str_c(number)
    };
    return leg;
}

