/*
 * convert_csv.h
 *
 *  Created on: 13. märts 2018
 *      Author: raigo
 */

#ifndef CONVERT_CSV_H_
#define CONVERT_CSV_H_

#include <stdbool.h>

#include "arg_parsing.h"
#include "error.h"

bool convert_csv(
		const char* prog, const char* command, struct arp_parser parser,
		struct error*);

#endif /* CONVERT_CSV_H_ */
