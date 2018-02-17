/*
 * arg_parsing.c
 *
 *  Created on: 10. veebr 2018
 *      Author: raigo
 */

#include "arg_parsing.h"

#include <stdint.h>
#include <stdlib.h>

#include "command_line.h"

struct arp_iterator get_arg_parsing_iterator(int argc, const char** argv,
		int offset) {
	struct arp_iterator iterator = { .argc = argc, .argv = argv,
			.arg_index = offset - 1, .cur_arg_count = 0 };
	return iterator;
}

bool arp_next(struct arp_iterator* iterator_p) {
	struct arp_iterator iterator = *iterator_p;
	int32_t argument_count = 0;
	struct print_args args = { 0 };
	bool keep_parsing_options = true;
	struct option_parameter option = { 0 };
	const char* option_argument;
	for (int32_t i = iterator.arg_index + 1; i < iterator.argc; i++) {
		const char* argument = iterator.argv[i];
		if (!is_null_option(option) && option.argument_name != NULL
				&& *option_argument == '\0') {
			option_argument = argument;

			iterator.arg_index = i;
			iterator.cur_opt_arg = argument;
			iterator.cur_arg = NULL;
			iterator.return_mode = ret_option;
			*iterator_p = iterator;
			return true;
		} else if (argument[0] != '-' || !keep_parsing_options
				|| argument[1] == '\0') {
			argument_count++;
			iterator.arg_index = i;
			iterator.cur_opt_arg = NULL;
			iterator.cur_arg = argument;
			iterator.cur_arg_count++;
			iterator.return_mode = ret_argument;
			*iterator_p = iterator;
			return true;
		} else if (argument[1] != '-') {
			const char* iter_buffer = begin_parse_short_options(argument);
			bool success = parse_next_short_option(&option, &option_argument,
					&iter_buffer);
			while (success && !is_null_option(option)
					&& option.argument_name == NULL) {
				args = option.set_option_data(args, "");
				success = parse_next_short_option(&option, &option_argument,
						&iter_buffer);
			}

			if (!success) {
				goto error;
			}
		} else if (argument[2] == '\0') {
			keep_parsing_options = false;
		} else if (!parse_long_option(&option, &option_argument, argument)) {
			goto error;
		}

		if (!is_null_option(option)
				&& (option.argument_name == NULL || *option_argument != '\0')) {
			args = option.set_option_data(args, option_argument);
			option = (struct option_parameter )
					{ 0 };
			option_argument = "";
		}
	}

	if (argument_count < 1 && !args.print_help) {
		goto error;
	}

	error: return false;
}
