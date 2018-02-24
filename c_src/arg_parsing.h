/*
 * arg_parse.h
 *
 *  Created on: 10. veebr 2018
 *      Author: raigo
 */

#ifndef ARG_PARSING_H_
#define ARG_PARSING_H_

#include <stdbool.h>

#include "vectors.h"

struct arp_option {
	char short_form;
	const char* long_form;
	const char* help_text;
	const char* argument_name;
	void (*set_option)(void* args_p, const char* argument);
};

VECTOR_DECLARE(static, struct arp_option, arp_option_vec)
VECTOR_DEFINE(static, struct arp_option, arp_option_vec)

struct arp_iterator {
	int argc;
	const char** argv;
	struct arp_option_vec options;

	bool keep_parsing_options;
	int arg_index;
	int cur_arg_count;

	struct _arp_ret {
		struct arp_option option;
		const char* sho_opt_iter;
		const char* option_arg;
		const char* argument;
		enum {
			ret_begin,
			ret_end,
			ret_argument,
			ret_option_argument,
			ret_sho_opt,
			ret_long_opt
		} mode;
	} ret;
};

struct arp_iterator get_arg_parsing_iterator(int argc, const char** argv,
		int offset, struct arp_option_vec options);
bool arp_next(struct arp_iterator* iterator);
bool arp_has(struct arp_iterator iterator);
bool arp_has_option(struct arp_iterator iterator);
struct arp_option arp_get_option(struct arp_iterator iterator);
const char* arp_get_option_arg(struct arp_iterator iterator);
int32_t arp_get_arg_count(struct arp_iterator iterator);
const char* arp_get_arg(struct arp_iterator iterator);

#endif /* ARG_PARSING_H_ */
