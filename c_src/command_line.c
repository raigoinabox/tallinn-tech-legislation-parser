#include "command_line.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include <assert.h>
#include <string.h>

#include "util.h"
#include "safe_string.h"
#include "vectors.h"
#include "arg_parsing.h"

struct global_args {
	bool print_help;
	const char* command;
};

struct command {
	const char* command;
	const char* description;
};

VECTOR_DECLARE(static, struct command, command_vec)
VECTOR_DEFINE(static, struct command, command_vec)

static struct command_vec commands;
static struct arp_option_vec global_options;

static void set_global_help_option(void* args_p, const char* argument) {
	(void) argument;
	struct global_args* global_args_p = args_p;
	global_args_p->print_help = true;
}

static struct print_args set_print_help_data(struct print_args result,
		const char* argument) {
	(void) argument;
	result.print_help = true;
	return result;
}
static struct print_args set_output_data(struct print_args result,
		const char* argument) {
	result.output_file_name = argument;
	return result;
}
static struct print_args set_format_data(struct print_args result,
		const char* argument) {
	result.format = argument;
	return result;
}
static struct print_args set_debug_data(struct print_args result,
		const char* argument) {
	(void) argument;
	result.debug = true;
	return result;
}

static const struct option_parameter print_options[] =
		{ { .short_form = 'h', .long_form = "help", .help_text =
				"Print this help message.", .argument_name = NULL,
				.set_option_data = set_print_help_data }, { .short_form = 'o',
				.long_form = "output", .help_text =
						"Print output to FILE instead."
								" If instead of a file there is a -,"
								" the program will write to standard output."
								" By default calculates a reasonable file name"
								" and doesn't overwrite any existing file.",
				.argument_name = "FILE", .set_option_data = set_output_data },
				{ .short_form = 'f', .long_form = "format",
						.help_text =
								"Write output in FORMAT instead."
										" By default prints output in pdf."
										" The possible formats are the same"
										" as for graphviz: http://www.graphviz.org/doc/info/output.html.",
						.argument_name = "FORMAT", .set_option_data =
								set_format_data }, { .short_form = 'g',
						.long_form = "debug", .help_text =
								"Debug mode. Shows false-positives.",
						.argument_name =
						NULL, .set_option_data = set_debug_data }, { 0 } };

static int32_t find_space(const char* string) {
	int32_t space_idx = 0;

	while (string[space_idx] != '\0') {
		if (string[space_idx] == ' ') {
			return space_idx;
		}

		space_idx++;
	}

	return -1;
}

static struct string fit_text(const char* text, int32_t prefix_length) {
	char help_indent[100];
	if (prefix_length > 18) {
		int print_result = sprintf(help_indent, "\n%18c", ' ');
		assert(print_result >= 0);
	} else {
		int i = 0;
		for (; i < 18 - prefix_length; i++) {
			help_indent[i] = ' ';
		}
		help_indent[i] = '\0';
	}

	int help_text_len = strlen(text);
	int32_t text_read = 0;
	char split_text_buf[help_text_len * 2 + 10];
	struct string split_text = str_init_s(split_text_buf,
			sizeof(split_text_buf));
	while (text_read < help_text_len) {
		int32_t line_length = 0;
		while (text_read < help_text_len) {
			int32_t chars_to_space_w = find_space(text + text_read + 1);
			int32_t chars_to_space = chars_to_space_w + 1;
			if (chars_to_space_w == -1) {
				chars_to_space = help_text_len - text_read;
			}
			if (chars_to_space + line_length >= 60 && line_length > 0) {
				break;
			}

			str_appendn(&split_text, text + text_read, chars_to_space);
			line_length += chars_to_space;
			text_read += chars_to_space;
		}
		if (text_read >= help_text_len) {
			break;
		}

		str_append(&split_text, "\n                    ");
		text_read += 1;
	}

	struct string help_text = str_init();
	str_appendf(&help_text, "%s  %s", help_indent, split_text.content);
	return help_text;
}

static bool print_option_help(struct option_parameter option) {
	char mod_long_form[strlen(option.long_form)
			+ (option.argument_name == NULL ? 0 : strlen(option.argument_name))
			+ 10];
	int print_result;
	if (option.argument_name == NULL) {
		strcpy(mod_long_form, option.long_form);
	} else {
		print_result = sprintf(mod_long_form, "%s=%s", option.long_form,
				option.argument_name);
		assert(print_result >= 0);
	}

	struct string prefix = str_init();
	str_appendf(&prefix, "  -%c, --%-10s", option.short_form, mod_long_form);

	struct string help_text = fit_text(option.help_text, str_length(prefix));

	printf_a("%s%s\n", str_content(prefix), str_content(help_text));
	str_free(&prefix);
	str_free(&help_text);
	return true;
}

static bool is_null_option(struct option_parameter option) {
	return option.short_form == '\0';
}

static bool parse_long_option(struct option_parameter* result,
		const char** option_argument, const char* argument) {
	for (int32_t option_index = 0;; option_index++) {
		struct option_parameter option = print_options[option_index];
		if (is_null_option(option)) {
			break;
		}

		char split_argument[strlen(argument) + 10];
		strcpy(split_argument, argument + 2);
		int32_t option_argument_index = str_tokenize(split_argument, '=');
		if (strcmp(option.long_form, split_argument) == 0) {
			*result = option;
			if (option_argument_index < 0) {
				*option_argument = argument + strlen(argument);
			} else {
				*option_argument = argument + option_argument_index + 2;
			}
			return true;
		}
	}

	printf_ea("Unkown option %s\n", argument);
	return false;
}

static const char* begin_parse_short_options(const char* argument) {
	return argument + 1;
}

static bool parse_next_short_option(struct option_parameter* result,
		const char** option_argument, const char** iterator) {
	char option_key = **iterator;
	if (option_key == '\0') {
		*result = (struct option_parameter )
				{ 0 };
		return true;
	}

	struct option_parameter option;
	for (int32_t option_index = 0;; option_index++) {
		option = print_options[option_index];
		if (is_null_option(option)) {
			printf_ea("Unknown option -%c\n", option.short_form);
			return false;
		}

		if (option_key == option.short_form) {
			break;
		}
	}

	const char* next_str = *iterator + 1;
	if (option.argument_name != NULL) {
		*option_argument = next_str;

		next_str += 1;
		while (*next_str != '\0') {
			next_str += 1;
		}
	}

	*iterator = next_str;
	*result = option;

	return true;
}

void mod_init_command_line() {
	commands = command_vec_init();
	struct command command = { .command = "print", .description =
			"Print legislation url as PDF." };
	command_vec_append(&commands, command);
	command.command = "comp-dbu";
	command.description =
			"Compare my parser results with those of Doing Business report.";
	command_vec_append(&commands, command);

	global_options = arp_option_vec_init();
	struct arp_option global_option = { .short_form = 'h', .long_form = "help",
			.help_text = "Print this help message.", .argument_name =
			NULL, .set_option = set_global_help_option };
	arp_option_vec_append(&global_options, global_option);
}

bool print_help(const char* program_name) {
	int result = printf("Usage: %s <command> [<args>] \n", program_name);
	if (result < 0) {
		return false;
	}
	printf_a("Possible commands are:\n");
	for (int i = 0; i < command_vec_length(commands); i++) {
		struct command command = command_vec_get(commands, i);
		struct string description = fit_text(command.description,
				strlen(command.command) + 3);
		printf_a("   %s%s\n", command.command, str_content(description));
		str_free(&description);
	}
	printf_a("\n");
	printf_a("Write %s <command> -h for more information about the command.",
			program_name);
	printf_a("\n");

	return true;
}

bool print_print_help(const char* program_name) {
	int result = printf("Usage: %s [OPTION]... legislation_act_url\n\n",
			program_name);
	if (result < 0) {
		return false;
	}
	for (int32_t option_index = 0;; option_index++) {
		struct option_parameter option = print_options[option_index];
		if (is_null_option(option)) {
			break;
		}

		if (!print_option_help(option)) {
			return false;
		}
	}

	return true;
}

static bool parse_global_args(struct global_args* result, int argc,
		char const* argv[]) {
	assert(result != NULL);

	struct arp_iterator iterator = get_arg_parsing_iterator(argc, argv, 0,
			global_options);
	while (arp_next(&iterator)) {
		if (!arp_has(iterator)) {
			return true;
		}
		if (arp_has_option(iterator)) {
			struct arp_option option = arp_get_option(iterator);
			option.set_option(result, arp_get_option_arg(iterator));
		} else if (arp_get_arg_count(iterator) == 1) {
			result->command = arp_get_arg(iterator);
		} else {
//			printf_ea("")
//			break;
		}
	}

	return false;
}

bool parse_print_args(struct print_args* result, int argc, char const* argv[],
		int32_t offset) {
	assert(result != NULL);

	int32_t argument_count = 0;
	struct print_args args = { 0 };
	bool keep_parsing_options = true;
	struct option_parameter option = { 0 };
	const char* option_argument;
	for (int32_t i = 1 + offset; i < argc; i++) {
		const char* argument = argv[i];
		if (!is_null_option(option) && option.argument_name != NULL
				&& *option_argument == '\0') {
			option_argument = argument;
		} else if (argument[0] != '-' || !keep_parsing_options
				|| argument[1] == '\0') {
			argument_count++;
			if (argument_count == 1) {
				args.url = argv[i];
			} else {
				goto error;
			}
		} else if (argument[1] != '-') {
			const char* iter_buffer = begin_parse_short_options(argument);
			bool success = parse_next_short_option(&option, &option_argument,
					&iter_buffer);
			while (success && !is_null_option(option)
					&& option.argument_name == NULL) {
				args = option.set_option_data(args, "");
				success = parse_next_short_option(&option, &option_argument,
						&iter_buffer);
			}

			if (!success) {
				goto error;
			}
		} else if (argument[2] == '\0') {
			keep_parsing_options = false;
		} else if (!parse_long_option(&option, &option_argument, argument)) {
			goto error;
		}

		if (!is_null_option(option)
				&& (option.argument_name == NULL || *option_argument != '\0')) {
			args = option.set_option_data(args, option_argument);
			option = (struct option_parameter )
					{ 0 };
			option_argument = "";
		}
	}

	if (argument_count < 1 && !args.print_help) {
		goto error;
	}

	*result = args;
	return true;

	error: print_print_help(argv[0]);
	return false;
}
