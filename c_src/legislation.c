/*
 * legislation.c
 *
 *  Created on: 13. jaan 2018
 *      Author: raigo
 */

#include "legislation.h"

#include <stdbool.h>

#include "safe_string.h"

struct leg_id leg_init(char* type, char* year, char* number) {
	struct leg_id leg = {
		.type = str_init_c(type),
		.year = str_init_c(year),
		.number = str_init_c(number)
	};
	return leg;
}

struct cleg_id cleg_init(struct leg_id leg) {
	struct cleg_id cleg;
	cleg.type = cst_from_str(leg.type);
	cleg.year = cst_from_str(leg.year);
	cleg.number = cst_from_str(leg.number);
	return cleg;
}
