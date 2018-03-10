/*
 * legislation.c
 *
 *  Created on: 13. jaan 2018
 *      Author: raigo
 */

#include "legislation.h"

#include <stdbool.h>

#include "safe_string.h"

struct leg_id leg_init(const char* type, char* year, char* number) {
	struct leg_id leg = {
		.type = str_init_c(type),
		.year = str_init_c(year),
		.number = str_init_c(number)
	};
	return leg;
}
