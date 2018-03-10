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
#include "sections.h"
#include "strings.h"
#include "web.h"

struct run_info {
	struct leg_id legislation;
	FILE* output_file;
	const char* format;
	bool debug;
};

struct print_args {
	const char* url;
	const char* output_file_name;
	const char* format;
	bool print_help;
	bool debug;
};

static bool get_default_file(FILE** result, struct leg_id legislation,
		const char* format) {
	struct string file_name = str_init_ds(
			str_length(legislation.type) + str_length(legislation.year)
					+ str_length(legislation.number) + strlen(format) + 10);

	str_append(&file_name, legislation.type);
	str_appends(&file_name, "_");
	str_append(&file_name, legislation.year);
	str_appends(&file_name, "_");
	str_append(&file_name, legislation.number);
	str_appends(&file_name, ".");
	str_appends(&file_name, format);

	int32_t file_number = 1;
	FILE* output_file = fopen(str_content(file_name), "r");
	while (output_file != NULL) {
		fclose(output_file);

		char file_number_str[100];
		sprintf(file_number_str, "%d", file_number);
		str_clear(&file_name);
		str_append(&file_name, legislation.type);
		str_appends(&file_name, "_");
		str_append(&file_name, legislation.year);
		str_appends(&file_name, "_");
		str_append(&file_name, legislation.number);
		str_appends(&file_name, "-");
		str_appends(&file_name, file_number_str);
		str_appends(&file_name, ".");
		str_appends(&file_name, format);

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

static struct arp_option_vec get_options() {
	struct arp_option_vec options;
	vec_init(options);
	struct arp_option option = { .short_form = 'h', .long_form = "help",
			.help_text = "Print this help message.", .argument_name = NULL };
	vec_append(options, option);
	option.short_form = 'o';
	option.long_form = "output";
	option.help_text = "Print output to FILE instead."
			" If instead of a file there is a -,"
			" the program will write to standard output."
			" By default calculates a reasonable file name"
			" and doesn't overwrite any existing file.";
	option.argument_name = "FILE";
	vec_append(options, option);
	option.short_form = 'f';
	option.long_form = "format";
	option.help_text = "Write output in FORMAT instead."
			" By default prints output in pdf."
			" The possible formats are the same"
			" as for graphviz:"
			" http://www.graphviz.org/doc/info/output.html.";
	option.argument_name = "FORMAT";
	vec_append(options, option);
	option.short_form = 'g';
	option.long_form = "debug";
	option.help_text = "Debug mode. Shows false-positives.";
	option.argument_name = NULL;
	vec_append(options, option);
	return options;
}

static bool parse_args(struct print_args* result_p, const char* prog,
		const char* command, struct arp_parser parser,
		struct arp_option_vec options, const char* argument_text) {
	struct print_args result = { 0 };
	bool success = arp_next(&parser);
	while (success && arp_has(parser)) {
		if (arp_has_option(parser)) {
			switch (arp_get_option_key(parser)) {
			case 'h':
				result.print_help = true;
				break;
			case 'o':
				result.output_file_name = arp_get_option_arg(parser);
				break;
			case 'f':
				result.format = arp_get_option_arg(parser);
				break;
			case 'g':
				result.debug = true;
				break;
			default:
				col_print_command_help(prog, command, options, argument_text);
				return false;
			}
		} else if (arp_get_arg_count(parser) == 1) {
			result.url = arp_get_arg(parser);
		} else {
			printf_ea("%s: too many arguments\n", prog);
			col_print_command_help(prog, command, options, argument_text);
			return false;
		}
		success = arp_next(&parser);
	}

	if (!result.print_help && arp_get_arg_count(parser) < 1) {
		printf_ea("%s: arguments required\n", prog);
		return false;
	}

	if (!success) {
		return false;
	}

	*result_p = result;
	return true;
}

bool print_leg(const char* prog, const char* command,
		struct arp_parser arg_parser) {
	struct print_args args;
	struct arp_option_vec options = get_options();
	const char* argument_text = "legislation_act_url";
	if (!parse_args(&args, prog, command,
			arp_get_parser_from_parser(arg_parser, options), options,
			argument_text)) {
		return false;
	}

	if (args.print_help) {
		if (!col_print_command_help(prog, command, options, argument_text)) {
			return false;
		}
	} else {
		struct run_info run_info;
		if (!process_args(&run_info, args)) {
			return false;
		}

		struct sections sections;
		if (!get_sections_from_legislation(&sections,
				run_info.legislation)) {
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
