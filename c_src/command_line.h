#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "safe_string.h"

struct option_parameter {
	char short_form;
	const char* long_form;
	const char* help_text;
	const char* argument_name;
	struct print_args (*set_option_data)(struct print_args, const char*);
};

struct print_args {
	const char* url;
	const char* output_file_name;
	const char* format;
	bool print_help;
	bool debug;
};

void mod_init_command_line();
bool print_help(const char* program_name);
bool print_print_help(const char* program_name);
bool parse_print_args(struct print_args* result, int argc, char const* argv[],
		int32_t offset);
