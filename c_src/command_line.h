#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "arg_parsing.h"
#include "error.h"

struct command
{
    const char* command_text;
    const char* description;
    bool (*command)(const char*, const char*, struct arp_parser, struct error*);
};
vec_struct(command_vec, struct command);

bool col_print_init_help(const char* program_name,
                         struct arp_option_vec options,
                         struct command_vec commands);
bool col_print_command_help(const char* program_name, const char* command,
                            struct arp_option_vec options, const char* argument_string);
