/*
 * arg_parsing.c
 *
 *  Created on: 10. veebr 2018
 *      Author: raigo
 */

#include "arg_parsing.h"

#include <stdint.h>
#include <string.h>

#include "util.h"

static bool find_option(struct arp_option* result,
		struct arp_option_vec options, const char short_form) {
	for (int32_t option_index = 0;
			option_index < arp_option_vec_length(options); option_index++) {
		struct arp_option option = arp_option_vec_get(options, option_index);
		if (short_form == option.short_form) {
			*result = option;
			return true;
		}
	}

	return false;
}

static bool next_short_option(struct arp_iterator* iterator_p) {
	struct arp_iterator iterator = *iterator_p;
	struct _arp_ret ret = { 0 };

	struct arp_option option;
	char option_key = *iterator.ret.sho_opt_iter;
	if (!find_option(&option, iterator.options, option_key)) {
		printf_ea("Unknown option -%c\n", option_key);
		return false;
	} else {
		ret.option = option;
		if (option.argument_name == NULL) {
			ret.mode = ret_sho_opt;
			ret.sho_opt_iter = iterator.ret.sho_opt_iter + 1;
			goto success;
		} else if (iterator.ret.sho_opt_iter[1] != '\0') {
			ret.mode = ret_option_argument;
			ret.option_arg = iterator.ret.sho_opt_iter + 1;
			goto success;
		} else {
			iterator.arg_index++;
			if (iterator.argc <= iterator.arg_index) {
				printf_ea("Option -%c requires an argument.\n", option_key);
				return false;
			} else {
				ret.mode = ret_option_argument;
				ret.option_arg = iterator.argv[iterator.arg_index];
				goto success;
			}
		}
	}

	return false;

	success: iterator.ret = ret;
	*iterator_p = iterator;
	return true;
}

static bool next_long_option(struct arp_iterator* iterator_p,
		const char* argument) {
	struct arp_iterator iterator = *iterator_p;

	for (int32_t option_index = 0;
			option_index < arp_option_vec_length(iterator.options);
			option_index++) {
		struct arp_option option = arp_option_vec_get(iterator.options,
				option_index);

		char split_argument[strlen(argument) + 10];
		strcpy(split_argument, argument + 2);
		int32_t option_argument_index = str_tokenize(split_argument, '=');
		if (strcmp(option.long_form, split_argument) == 0) {
			struct _arp_ret ret = { 0 };
			ret.option = option;
			if (option_argument_index < 0) {
				iterator.arg_index++;
				if (iterator.argc <= iterator.arg_index) {
					printf_ea("Option %s requires an argument.\n", argument);
					return false;
				}
				ret.option_arg = iterator.argv[iterator.arg_index];
			} else {
				ret.option_arg = argument + option_argument_index + 2;
			}
			iterator.ret = ret;
			*iterator_p = iterator;
			return true;
		}
	}

	printf_ea("Unkown option %s\n", argument);
	return false;
}

struct arp_iterator get_arg_parsing_iterator(int argc, const char** argv,
		int offset, struct arp_option_vec options) {
	struct arp_iterator iterator = { .argc = argc, .argv = argv, .options =
			options, .arg_index = offset, .cur_arg_count = 0, .ret = { .mode =
			ret_begin } };
	return iterator;
}

bool arp_next(struct arp_iterator* iterator_p) {
	struct arp_iterator iterator = *iterator_p;
	struct _arp_ret ret = { 0 };
	bool is_success;

	// handle any remaining short options
	if (iterator.ret.mode == ret_sho_opt) {
		char option_key = *ret.sho_opt_iter;
		if (option_key == '\0') {
			ret.mode = ret_begin;
			iterator.ret = ret;
			is_success = arp_next(&iterator);
		} else {
			is_success = next_short_option(&iterator);
		}
	} else {
		// move on to next argument
		iterator.arg_index++;
		if (iterator.argc <= iterator.arg_index) {
			ret.mode = ret_end;
			is_success = true;
		} else {
			const char* argument = iterator.argv[iterator.arg_index];
			if (argument[0] != '-' || !iterator.keep_parsing_options
					|| argument[1] == '\0') {
				ret.mode = ret_argument;
				ret.argument = argument;
				iterator.cur_arg_count++;
				is_success = true;
			} else if (argument[1] != '-') {
				ret.sho_opt_iter = argument + 1;
				iterator.ret = ret;
				is_success = next_short_option(&iterator);
			} else if (argument[2] == '\0') {
				iterator.keep_parsing_options = false;
				is_success = arp_next(&iterator);
			} else if (argument[2] != '-') {
				is_success = !next_long_option(&iterator, argument);
			} else {
				printf_ea("Three dashes is not supported.\n", argument);
				is_success = false;
			}
		}
	}

	if (is_success) {
		iterator.ret = ret;
		*iterator_p = iterator;
	}
	return is_success;
}

bool arp_has(struct arp_iterator iterator) {
	return iterator.ret.mode != ret_end;
}

bool arp_has_option(struct arp_iterator iterator) {
	switch (iterator.ret.mode) {
	case ret_long_opt:
	case ret_sho_opt:
	case ret_option_argument:
		return true;
	default:
		return false;
	}
}

struct arp_option arp_get_option(struct arp_iterator iterator) {
	return iterator.ret.option;
}

const char* arp_get_option_arg(struct arp_iterator iterator) {
	return iterator.ret.option_arg;
}

int32_t arp_get_arg_count(struct arp_iterator iterator) {
	return iterator.cur_arg_count;
}

const char* arp_get_arg(struct arp_iterator iterator) {
	return iterator.ret.argument;
}
