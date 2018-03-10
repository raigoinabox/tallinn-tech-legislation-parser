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

struct arp_option
{
    char short_form;
    const char* long_form;
    const char* help_text;
    const char* argument_name;
};

vec_struct(arp_option_vec, struct arp_option);

struct arp_parser
{
    int argc;
    const char** argv;
    struct arp_option_vec options;

    bool keep_parsing_options;
    int arg_index;
    int cur_arg_count;
    bool is_end;

    struct _arp_ret
    {
        struct arp_option option;
        const char* sho_opt_iter;
        const char* option_arg;
        const char* argument;
    } ret;
};

struct arp_parser arp_get_parser(int argc, const char** argv, int offset,
                                 struct arp_option_vec options);
struct arp_parser arp_get_parser_from_parser(struct arp_parser parser,
        struct arp_option_vec options);
bool arp_next(struct arp_parser* parser);

bool arp_has(struct arp_parser parser);
bool arp_has_option(struct arp_parser parser);
char arp_get_option_key(struct arp_parser parser);
const char* arp_get_option_arg(struct arp_parser parser);
int32_t arp_get_arg_count(struct arp_parser parser);
const char* arp_get_arg(struct arp_parser parser);

void arp_print_options_help(struct arp_option_vec options);

#endif /* ARG_PARSING_H_ */
