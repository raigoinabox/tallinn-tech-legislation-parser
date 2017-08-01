#include "command_line.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <assert.h>
#include <string.h>

#include "util.h"

struct option_parameter
{
	char short_form;
	const char* long_form;
	const char* help_text;
	const char* argument_name;
};

static const struct option_parameter options[] =
{
	{
		.short_form = 'h',
		.long_form = "help",
		.help_text = "Print this help message.",
		.argument_name = NULL
	},
	{
		.short_form = 'o',
		.long_form = "output",
		.help_text = "Print output to FILE instead.",
		.argument_name = "FILE"
	},
	{
		.short_form = 't',
		.long_form = "dot",
		.help_text = "Print output in dot format rather than in pdf.",
		.argument_name = NULL
	},
	{
		.short_form = 'd',
		.long_form = "debug",
		.help_text = "Debug mode. Activates experimental features.",
		.argument_name = NULL
	},
	{ 0 }
};

static bool parse_option(struct run_info* result, char key)
{
	switch (key)
	{
	case 'd':
		result->debug = true;
		return true;
	case 'r':
		result->dot_format = true;
		return true;
	case 'h':
		result->print_help = true;
		return true;
	default:
		return false;
	}
}

static bool parse_long_option(char* result, const char* argument)
{
	for (int32_t option_index = 0; ; option_index++)
	{
		struct option_parameter option = options[option_index];
		if (option.short_form == '\0')
		{
			break;
		}

		char split_argument[strlen(argument) + 10];
		strcpy(split_argument, argument + 2);
		char* option_argument = str_tokenize(split_argument, '=');
		if (strcmp(option.long_form, split_argument) == 0)
		{
			*result = option.short_form;
			return true;
		}
	}

	fprintf(stderr, "Unkown option %s\n", argument);
	return false;
}

static bool parse_short_options(struct run_info* result, const char* argument)
{
	const char* arg_key_p = argument + 1;
	if (*arg_key_p == '\0')
	{
		fprintf(stderr, "There is a dash, but no options.\n");
		return false;
	}
	while (*arg_key_p != '\0')
	{
		if (!parse_option(result, *arg_key_p))
		{
			fprintf(stderr, "Unknown option -%c\n", *arg_key_p);
			return false;
		}

		arg_key_p++;
	}

	return true;
}

void print_help(const char* program_name)
{
	int result = printf("Usage: %s [OPTION]... legislation_act_url\n\n",
	                    program_name);
	if (result < 0)
	{
		abort();
	}
	for (int32_t option_index = 0; ; option_index++)
	{
		struct option_parameter option = options[option_index];
		if (option.short_form == '\0')
		{
			break;
		}

		char mod_long_form[strlen(option.long_form) + (option.argument_name == NULL ?
		                   0 : strlen(option.argument_name)) + 10];
		if (option.argument_name == NULL)
		{
			strcpy(mod_long_form, option.long_form);
		}
		else
		{
			assert(sprintf(mod_long_form, "%s=%s", option.long_form,
			               option.argument_name) >= 0);
		}

		char indent_help_text[strlen(option.help_text) + 100];
		if (strlen(mod_long_form) > 10)
		{
			assert(sprintf(indent_help_text, "\n                    %s",
			               option.help_text) >= 0);
		}
		else
		{
			assert(sprintf(indent_help_text, "  %s", option.help_text) >= 0);
		}
		result = printf("  -%c, --%-10s%s\n", option.short_form, mod_long_form,
		                indent_help_text);
		if (result < 0)
		{
			abort();
		}
	}
}

bool parse_args(struct run_info* result, int argc, char const* argv[])
{
	assert(result != NULL);

	int32_t argument_count = 0;
	struct run_info args =
	{
		.url = NULL,
		.debug = false,
		.dot_format = false,
		.print_help = false
	};
	bool keep_parsing_options = true;
	char prev_option = '\0';
	for (int32_t i = 1; i < argc; i++)
	{
		const char* argument = argv[i];
		bool is_argument = false;
		if (!keep_parsing_options)
		{
			is_argument = true;
		}
		else if (strcmp(argument, "--") == 0)
		{
			keep_parsing_options = false;
			// this argument is now parsed
			continue;
		}
		else if (strncmp(argument, "--", 2) == 0)
		{
			char short_option;
			if (!parse_long_option(&short_option, argument))
			{
				goto error;
			}
			assert(parse_option(&args, short_option));
		}
		else if (argument[0] == '-')
		{
			if (!parse_short_options(&args, argument))
			{
				goto error;
			}
		}
		else
		{
			is_argument = true;
		}

		if (is_argument)
		{
			argument_count++;
			if (argument_count == 1)
			{
				args.url = argv[i];
			}
			else
			{
				fprintf(stderr, "%s: Usage: %s document_url\n", argv[0], argv[0]);
				goto error;
			}
		}
	}

	if (argument_count < 1 && !args.print_help)
	{
		goto error;
	}

	*result = args;
	return true;

error:
	print_help(argv[0]);
	return false;
}
