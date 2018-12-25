#include <assert.h>
#include <save_graph.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parsing.h"
#include "command_line.h"
#include "convert_csv.h"
#include "print_leg.h"
#include "printing.h"
#include "save_dbu_compl.h"
#include "vectors.h"
#include "leg_feed.h"
#include "estonia.h"

struct global_args
{
    bool print_help;
    const char* command;
};

static struct arp_option_vec get_leg_options()
{
    struct arp_option_vec leg_options;
    vec_init_old(leg_options);
    struct arp_option global_option = { .short_form = 'h', .long_form = "help",
        .help_text = "Print this help message.", .argument_name =
                         NULL
    };
    vec_append_old(leg_options, global_option);
    return leg_options;
}

static bool parse_init_args(struct global_args* result_p,
                            struct arp_parser* parser_p)
{
    assert(result_p != NULL);
    assert(parser_p != NULL);
    struct global_args result = { 0 };
    struct arp_parser parser = *parser_p;

    bool success = arp_next(&parser);
    while (success && arp_has(parser))
    {
        if (arp_has_option(parser))
        {
            char option = arp_get_option_key(parser);
            switch (option)
            {
            case 'h':
                result.print_help = true;
                break;
            default:
                return false;
            }
        }
        else
        {
            result.command = arp_get_arg(parser);
            break;
        }
        success = arp_next(&parser);
    }

    if (success)
    {
        *result_p = result;
        *parser_p = parser;
    }
    return success;
}

static struct command_vec get_commands()
{
    struct command_vec commands;
    vec_init_old(commands);

    struct command command;
    command.command_text = "print";
    command.description = "Print legislation url as PDF.";
    command.command = print_leg;
    vec_append_old(commands, command);

    command.command_text = "save-dbu-compl";
    command.description = "Save the the complexities of law that"
                          " belong into Doing Business report topics"
                          " into postgresql database table complexity_results.";
    command.command = save_dbu_compl;
    vec_append_old(commands, command);

    command.command_text = "graph";
    command.description = "Calculate the graph of laws.";
    command.command = save_graph;
    vec_append_old(commands, command);

    command.command_text = "csv";
    command.description = "Convert a csv into a dot format graph.";
    command.command = convert_csv;
    vec_append_old(commands, command);

    command.command_text = "search";
    command.description = "Search for all laws.";
    command.command = search_laws;
    vec_append_old(commands, command);

	command.command_text = "estonia";
	command.description = "Parse and insert estonian laws.";
	command.command = estonia;
	vec_append_old(commands, command);

    return commands;
}

int main(int argc, const char* argv[])
{
    if (argc < 1)
    {
        abort();
    }
    struct command_vec commands = get_commands();
    struct arp_option_vec options = get_leg_options();
    struct arp_parser parser = arp_get_parser(argc, argv, 0,
                               options);

    struct global_args args;
    if (!parse_init_args(&args, &parser))
    {
        vec_free(commands);
        vec_free(options);
        fprintf(stderr, "parse_init_args fail\n");
        return EXIT_FAILURE;
    }

    if (args.print_help)
    {
        col_print_init_help(argv[0], options, commands);
        vec_free(commands);
        vec_free(options);
        return EXIT_SUCCESS;
    }
    else
    {
        bool success = false;
        struct error error = init_error();
        if (args.command == NULL)
        {
            printf_ea("%s: command is required\n", argv[0]);
            col_print_init_help(argv[0], options, commands);
			return EXIT_FAILURE;
        }
        else
        {
        	bool command_found = false;
			for (int i = 0; i < vec_length_old(commands); i++)
			{
				struct command command = vec_elem_old(commands, i);
				if (strcmp(args.command, command.command_text) == 0)
				{
					command_found = true;
					success = command.command(
							argv[0], args.command, parser, &error);
					break;
				}
			}
			if (!command_found)
			{
				printf_ea("%s: unknown command: %s\n", argv[0], args.command);
				col_print_init_help(argv[0], options, commands);
			}
        }

        vec_free(commands);
        vec_free(options);
        if (success)
        {
            return EXIT_SUCCESS;
        }
        else
        {
        	print_error(error);
            return EXIT_FAILURE;
        }
    }
}
