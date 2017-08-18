#pragma once

#include <stdbool.h>
#include <stdint.h>

struct run_info
{
	const char* url;
	const char* output_file_name;
	const char* format;
	bool print_help;
	bool debug;
};

bool print_help(const char* program_name);
bool parse_args(struct run_info* result, int argc, const char* argv[]);
