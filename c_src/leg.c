#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parsing.h"
#include "command_line.h"
#include "print_leg.h"
#include "printing.h"
#include "save_dbu_compl.h"

struct global_args {
	bool print_help;
	const char* command;
};

static struct arp_option_vec get_leg_options() {
	struct arp_option_vec leg_options;
	vec_init(leg_options);
	struct arp_option global_option = { .short_form = 'h', .long_form = "help",
			.help_text = "Print this help message.", .argument_name =
			NULL };
	vec_append(leg_options, global_option);
	return leg_options;
}

static bool parse_init_args(struct global_args* result_p,
		struct arp_parser* parser_p) {
	assert(result_p != NULL);
	assert(parser_p != NULL);
	struct global_args result = { 0 };
	struct arp_parser parser = *parser_p;

	bool success = arp_next(&parser);
	while (success && arp_has(parser)) {
		if (arp_has_option(parser)) {
			char option = arp_get_option_key(parser);
			switch (option) {
			case 'h':
				result.print_help = true;
				break;
			default:
				return false;
			}
		} else {
			result.command = arp_get_arg(parser);
			break;
		}
		success = arp_next(&parser);
	}

	if (success) {
		*result_p = result;
		*parser_p = parser;
	}
	return success;
}

static struct command_vec get_commands() {
	struct command_vec commands;
	vec_init(commands);
	struct command command = { .command = "print", .description =
			"Print legislation url as PDF." };
	vec_append(commands, command);
	command.command = "save-dbu-compl";
	command.description = "Save the the complexities of law that"
			" belong into Doing Business report topics"
			" into sqlite database data.db table complexity_results.";
	vec_append(commands, command);
	return commands;
}

int main(int argc, char const* argv[]) {
	if (argc < 1) {
		abort();
	}
	struct command_vec commands = get_commands();
	struct arp_option_vec options = get_leg_options();
	struct arp_parser parser = arp_get_parser(argc, argv, 0,
			options);

	struct global_args result;
	if (!parse_init_args(&result, &parser)) {
		vec_free(commands);
		vec_free(options);
		return EXIT_FAILURE;
	}

	if (result.print_help) {
		col_print_init_help(argv[0], options, commands);
		vec_free(commands);
		vec_free(options);
		return EXIT_SUCCESS;
	} else {
		bool success = false;
		if (result.command == NULL) {
			printf_ea("%s: command is required\n", argv[0]);
			col_print_init_help(argv[0], options, commands);
		} else if (strcmp("print", result.command) == 0) {
			success = print_leg(argv[0], result.command, parser);
		} else if (strcmp("save-dbu-compl", result.command) == 0) {
			success = save_dbu_compl(argv[0], result.command, parser);
		} else {
			printf_ea("%s: unknown command: %s\n", argv[0], result.command);
			col_print_init_help(argv[0], options, commands);
		}

		vec_free(commands);
		vec_free(options);
		if (success) {
			return EXIT_SUCCESS;
		} else {
			return EXIT_FAILURE;
		}
	}
}
