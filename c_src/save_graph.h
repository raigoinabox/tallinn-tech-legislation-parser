/*
 * calc_graph.h
 *
 *  Created on: 28. sept 2018
 *      Author: raigo
 */

#ifndef SAVE_GRAPH_H_
#define SAVE_GRAPH_H_

#include <stdbool.h>

#include "arg_parsing.h"
#include "error.h"

bool save_graph(
		const char* program_name, const char* command_name, struct arp_parser,
		struct error*);

#endif /* SAVE_GRAPH_H_ */
