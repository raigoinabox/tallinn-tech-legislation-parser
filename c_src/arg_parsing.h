/*
 * arg_parse.h
 *
 *  Created on: 10. veebr 2018
 *      Author: raigo
 */

#ifndef ARG_PARSING_H_
#define ARG_PARSING_H_

#include <stdbool.h>

#include "command_line.h"

struct arp_iterator {
	int argc;
	const char** argv;

	int arg_index;

	enum { par_normal, par_sho_opt, par_opt_arg, par_only_args} mode;

	struct option_parameter option;
	bool keep_parsing_options;
	int cur_arg_count;
	const char* cur_opt_arg;
	const char* cur_arg;
	enum { ret_argument, ret_option } return_mode;
};

struct arp_iterator get_arg_parsing_iterator(int argc, const char** argv,
		int offset);
bool arp_next(struct arp_iterator* iterator);

#endif /* ARG_PARSING_H_ */
