/*
 * print_leg.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "print_leg.h"

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "command_line.h"
#include "legislation.h"
#include "misc.h"
#include "printing.h"
#include "safe_string.h"
#include "sections.h"
#include "web.h"

struct run_info {
	struct leg_id legislation;
	FILE* output_file;
	const char* format;
	bool debug;
};

static bool get_default_file(FILE** result, struct leg_id legislation,
		const char* format) {
	struct string file_name = str_init_ds(
			str_length(legislation.type) + str_length(legislation.year)
					+ str_length(legislation.number) + strlen(format) + 10);

	str_appends(&file_name, legislation.type);
	str_append(&file_name, "_");
	str_appends(&file_name, legislation.year);
	str_append(&file_name, "_");
	str_appends(&file_name, legislation.number);
	str_append(&file_name, ".");
	str_append(&file_name, format);

	int32_t file_number = 1;
	FILE* output_file = fopen(str_content(file_name), "r");
	while (output_file != NULL) {
		fclose(output_file);

		char file_number_str[100];
		sprintf(file_number_str, "%d", file_number);
		str_clear(&file_name);
		str_appends(&file_name, legislation.type);
		str_append(&file_name, "_");
		str_appends(&file_name, legislation.year);
		str_append(&file_name, "_");
		str_appends(&file_name, legislation.number);
		str_append(&file_name, "-");
		str_append(&file_name, file_number_str);
		str_append(&file_name, ".");
		str_append(&file_name, format);

		output_file = fopen(str_content(file_name), "r");
		file_number += 1;
	}
	if (errno != ENOENT) {
		perror("Unknown error while searching for the default file name ");
		return false;
	}

	output_file = fopen(str_content(file_name), "w");
	if (output_file == NULL) {
		perror(str_content(file_name));
		return false;
	}

	str_free(&file_name);
	*result = output_file;
	return true;
}

static bool process_args(struct run_info* result, struct print_args args) {
	struct run_info run_info;

	run_info.debug = args.debug;

	struct leg_id legislation;
	if (!parse_url(&legislation, args.url)) {
		fprintf(stderr, "Url is malformed.\n");
		return false;
	}
	run_info.legislation = legislation;

	run_info.format = args.format;
	if (run_info.format == NULL || strcmp(run_info.format, "") == 0) {
		run_info.format = "pdf";
	}

	if (args.output_file_name == NULL) {
		if (!get_default_file(&run_info.output_file, legislation,
				run_info.format)) {
			return false;
		}

	} else if (strcmp(args.output_file_name, "-") == 0) {
		run_info.output_file = stdout;
	} else {
		run_info.output_file = fopen(args.output_file_name, "w");
		if (run_info.output_file == NULL) {
			perror(args.output_file_name);
			return false;
		}
	}

	*result = run_info;
	return true;
}

bool print_leg(int argc, const char* argv[], int offset) {
	struct print_args args;
	bool success = parse_print_args(&args, argc, argv, offset);
	if (!success) {
		return false;
	}

	if (args.print_help) {
		print_print_help(argv[0]);
	} else {
		struct run_info run_info;
		if (!process_args(&run_info, args)) {
			return false;
		}

		struct sections sections;
		success = get_sections_from_legislation(&sections, run_info.legislation);
		if (!success)
		{
			return false;
		}
		if (!args.debug) {
			remove_foreign_sections(sections);
		}
		remove_single_sections(&sections);

		print_from_sections(run_info.output_file, sections, run_info.format);

		sections_free_deep(&sections);
	}

	return true;
}
