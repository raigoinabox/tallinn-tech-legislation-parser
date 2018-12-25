/*
 * print_leg.h
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#ifndef PRINT_LEG_H_
#define PRINT_LEG_H_

#include <stdbool.h>

#include "arg_parsing.h"
#include "error.h"

bool print_leg(
		const char* prog, const char* command, struct arp_parser arg_parser,
		struct error*);

#endif /* PRINT_LEG_H_ */
