#pragma once

#include <stdbool.h>
#include <stdint.h>

struct print_args
{
	const char* url;
	const char* output_file_name;
	const char* format;
	bool print_help;
	bool debug;
};

struct dbu_args {
	int32_t hello;
};

bool print_help(const char* program_name);
bool parse_print_args(struct print_args* result, int argc, char const* argv[], int32_t offset);
