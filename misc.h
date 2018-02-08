/*
 * misc.h
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#ifndef MISC_H_
#define MISC_H_

#include <stdbool.h>

#include "legislation.h"
#include "sections.h"

bool get_sections_from_legislation(struct sections* result,
		struct leg_id legislation);

#endif /* MISC_H_ */
