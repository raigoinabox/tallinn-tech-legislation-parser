/*
 * arg_parsing.c
 *
 *  Created on: 10. veebr 2018
 *      Author: raigo
 */

#include "arg_parsing.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "printing.h"
#include "safe_string.h"
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

static bool next_short_option(struct arp_parser* iterator_p,
		struct _arp_ret* ret_p) {
	struct arp_parser iterator = *iterator_p;
	struct _arp_ret ret = *ret_p;

	struct arp_option option;
	char option_key = *iterator.ret.sho_opt_iter;
	if (!find_option(&option, iterator.options, option_key)) {
		printf_ea("Unknown option -%c\n", option_key);
		return false;
	} else {
		ret.option = option;
		if (option.argument_name == NULL) {
			ret.sho_opt_iter = iterator.ret.sho_opt_iter + 1;
		} else if (iterator.ret.sho_opt_iter[1] != '\0') {
			ret.option_arg = iterator.ret.sho_opt_iter + 1;
		} else {
			iterator.arg_index++;
			if (iterator.argc <= iterator.arg_index) {
				printf_ea("Option -%c requires an argument.\n", option_key);
				return false;
			} else {
				ret.option_arg = iterator.argv[iterator.arg_index];
			}
		}
	}

	*ret_p = ret;
	*iterator_p = iterator;
	return true;
}

static bool next_long_option(struct arp_parser* iterator_p,
		struct _arp_ret* ret_p, const char* argument) {
	struct arp_parser iterator = *iterator_p;

	for (int32_t option_index = 0;
			option_index < arp_option_vec_length(iterator.options);
			option_index++) {
		struct arp_option option = arp_option_vec_get(iterator.options,
				option_index);

		char split_argument[strlen(argument) + 10];
		strcpy(split_argument, argument + 2);
		int32_t option_argument_index = str_tokenize(split_argument, '=');
		if (strcmp(option.long_form, split_argument) == 0) {
			struct _arp_ret ret = *ret_p;
			ret.option = option;
			if (option.argument_name == NULL) {
				// empty
			} else if (option_argument_index < 0) {
				iterator.arg_index++;
				if (iterator.argc <= iterator.arg_index) {
					printf_ea("Option %s requires an argument.\n", argument);
					return false;
				}
				ret.option_arg = iterator.argv[iterator.arg_index];
			} else {
				ret.option_arg = argument + option_argument_index + 2;
			}
			*ret_p = ret;
			*iterator_p = iterator;
			return true;
		}
	}

	printf_ea("Unkown option %s\n", argument);
	return false;
}

struct arp_parser arp_get_parser(int argc, const char** argv,
		int offset, struct arp_option_vec options) {
	struct arp_parser iterator = { .argc = argc, .argv = argv, .options =
			options, .arg_index = offset, .cur_arg_count = 0,
			.keep_parsing_options = true, .is_end = false };
	return iterator;
}

struct arp_parser arp_get_parser_from_parser(struct arp_parser parser,
		struct arp_option_vec options) {
	parser.options = options;
	return parser;
}

bool arp_next(struct arp_parser* iterator_p) {
	struct arp_parser iterator = *iterator_p;
	struct _arp_ret ret = { 0 };
	bool is_success;

	// handle any remaining short options
	if (iterator.ret.sho_opt_iter != NULL) {
		char option_key = *iterator.ret.sho_opt_iter;
		if (option_key == '\0') {
			iterator.ret = ret;
			is_success = arp_next(&iterator);
		} else {
			is_success = next_short_option(&iterator, &ret);
		}
	} else {
		// move on to next argument
		iterator.arg_index++;
		if (iterator.argc <= iterator.arg_index) {
			iterator.is_end = true;
			is_success = true;
		} else {
			const char* argument = iterator.argv[iterator.arg_index];
			if (argument[0] != '-' || !iterator.keep_parsing_options
					|| argument[1] == '\0') {
				ret.argument = argument;
				iterator.cur_arg_count++;
				is_success = true;
			} else if (argument[1] != '-') {
				iterator.ret.sho_opt_iter = argument + 1;
				is_success = next_short_option(&iterator, &ret);
			} else if (argument[2] == '\0') {
				iterator.keep_parsing_options = false;
				is_success = arp_next(&iterator);
			} else if (argument[2] != '-') {
				is_success = next_long_option(&iterator, &ret, argument);
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

bool arp_has(struct arp_parser iterator) {
	return !iterator.is_end;
}

bool arp_has_option(struct arp_parser iterator) {
	return iterator.ret.option.short_form != 0;
}

char arp_get_option_key(struct arp_parser iterator) {
	return iterator.ret.option.short_form;
}

const char* arp_get_option_arg(struct arp_parser iterator) {
	return iterator.ret.option_arg;
}

int32_t arp_get_arg_count(struct arp_parser iterator) {
	return iterator.cur_arg_count;
}

const char* arp_get_arg(struct arp_parser iterator) {
	return iterator.ret.argument;
}

void arp_print_options_help(struct arp_option_vec options) {

	for (int i = 0; i < arp_option_vec_length(options); i++) {
		struct arp_option option = arp_option_vec_get(options, i);
		char mod_long_form[strlen(option.long_form)
				+ (option.argument_name == NULL ?
						0 : strlen(option.argument_name)) + 10];
		int print_result;
		if (option.argument_name == NULL) {
			strcpy(mod_long_form, option.long_form);
		} else {
			print_result = sprintf(mod_long_form, "%s=%s", option.long_form,
					option.argument_name);
			assert(print_result >= 0);
		}

		struct string prefix = str_init();
		str_appendf(&prefix, "  -%c, --%-10s", option.short_form,
				mod_long_form);

		struct string help_text = fit_text(option.help_text,
				str_length(prefix));

		printf_a("%s%s\n", str_content(prefix), str_content(help_text));
		str_free(&prefix);
		str_free(&help_text);
	}

}
