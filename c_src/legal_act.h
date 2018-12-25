/*
 * misc.h
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#ifndef LEGAL_ACT_H_
#define LEGAL_ACT_H_

#include <stdbool.h>

#include "sections.h"
#include "error.h"

struct legal_act_id
{
	char* type;
	char* year;
	char* number;
	const char* version_date;
};

struct legal_act_id leg_init_c(char* type, char* year, char* number);
void leg_free(struct legal_act_id* legislation_p);


bool get_sections_from_legislation(
		struct section_vec* result, struct legal_act_id, struct error*);

char* fit_text(const char* text, int32_t prefix_length);

#endif /* LEGAL_ACT_H_ */
