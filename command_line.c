#include "command_line.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include <assert.h>
#include <string.h>

#include "util.h"
#include "safe_string.h"

struct option_parameter
{
	char short_form;
	const char* long_form;
	const char* help_text;
	const char* argument_name;
	struct user_args (*set_option_data)(struct user_args, const char*);
};

static struct user_args set_print_help_data(struct user_args result,
        const char* argument)
{
	(void) argument;
	result.print_help = true;
	return result;
}
static struct user_args set_output_data(struct user_args result,
                                        const char* argument)
{
	result.output_file_name = argument;
	return result;
}
static struct user_args set_format_data(struct user_args result,
                                        const char* argument)
{
	result.format = argument;
	return result;
}
static struct user_args set_debug_data(struct user_args result,
                                       const char* argument)
{
	(void) argument;
	result.debug = true;
	return result;
}


static const struct option_parameter options[] =
{
	{
		.short_form = 'h',
		.long_form = "help",
		.help_text = "Print this help message.",
		.argument_name = NULL,
		.set_option_data = set_print_help_data
	},
	{
		.short_form = 'o',
		.long_form = "output",
		.help_text = "Print output to FILE instead."
			" If instead of a file there is a -,"
			" the program will write to standard output."
			" By default calculates a reasonable file name"
			" and doesn't overwrite any existing file.",
		.argument_name = "FILE",
		.set_option_data = set_output_data
	},
	{
		.short_form = 'f',
		.long_form = "format",
		.help_text = "Write output in FORMAT instead."
			" By default prints output in pdf."
			" The possible formats are the same"
			" as for graphviz: http://www.graphviz.org/doc/info/output.html.",
		.argument_name = "FORMAT",
		.set_option_data = set_format_data
	},
	{
		.short_form = 'g',
		.long_form = "debug",
		.help_text = "Debug mode. Shows false-positives.",
		.argument_name = NULL,
		.set_option_data = set_debug_data
	},
	{ 0 }
};

static void print_err_a(const char* format, ...)
{
	va_list this_va_list;
	va_start(this_va_list, format);
	if (vfprintf(stderr, format, this_va_list) < 0)
	{
		abort();
	}
}

static bool is_null_option(struct option_parameter option)
{
	return option.short_form == '\0';
}

static bool parse_long_option(struct option_parameter* result,
                              const char** option_argument, const char* argument)
{
	for (int32_t option_index = 0; ; option_index++)
	{
		struct option_parameter option = options[option_index];
		if (is_null_option(option))
		{
			break;
		}

		char split_argument[strlen(argument) + 10];
		strcpy(split_argument, argument + 2);
		int32_t option_argument_index = str_tokenize(split_argument, '=');
		if (strcmp(option.long_form, split_argument) == 0)
		{
			*result = option;
			if (option_argument_index < 0)
			{
				*option_argument = argument + strlen(argument);
			}
			else
			{
				*option_argument = argument + option_argument_index + 2;
			}
			return true;
		}
	}

	print_err_a("Unkown option %s\n", argument);
	return false;
}

static const char* begin_parse_short_options(const char* argument)
{
	return argument + 1;
}

static bool parse_next_short_option(struct option_parameter* result,
                                    const char** option_argument,
                                    const char** iterator)
{
	char option_key = **iterator;
	if (option_key == '\0')
	{
		*result = (struct option_parameter)
		{
			0
		};
		return true;
	}

	struct option_parameter option;
	for (int32_t option_index = 0; ; option_index++)
	{
		option = options[option_index];
		if (is_null_option(option))
		{
			print_err_a("Unknown option -%c\n", option.short_form);
			return false;
		}

		if (option_key == option.short_form)
		{
			break;
		}
	}

	const char* next_str = *iterator + 1;
	if (option.argument_name != NULL)
	{
		*option_argument = next_str;

		next_str += 1;
		while (*next_str != '\0')
		{
			next_str += 1;
		}
	}

	*iterator = next_str;
	*result = option;

	return true;
}

static int32_t find_space(const char* string)
{
	int32_t space_idx = 0;

	while (string[space_idx] != '\0')
	{
		if (string[space_idx] == ' ')
		{
			return space_idx;
		}

		space_idx++;
	}

	return -1;
}

static bool print_option_help(struct option_parameter option)
{
	char mod_long_form[strlen(option.long_form) + (option.argument_name == NULL ?
	                   0 : strlen(option.argument_name)) + 10];
	int print_result;
	if (option.argument_name == NULL)
	{
		strcpy(mod_long_form, option.long_form);
	}
	else
	{
		print_result = sprintf(mod_long_form, "%s=%s", option.long_form,
		                       option.argument_name);
		if (print_result < 0)
		{
			return false;
		}
	}

	char help_indent[100];
	if (strlen(mod_long_form) > 10)
	{
		print_result = sprintf(help_indent, "\n%18c", ' ');
		if (print_result < 0)
		{
			return false;
		}
	}
	else
	{
		help_indent[0] = '\0';
	}

	int help_text_len = strlen(option.help_text);
	int32_t text_read = 0;
	char split_text_buf[help_text_len * 2 + 10];
	struct string_s split_text = strs_init(split_text_buf, sizeof(split_text_buf));
	while (text_read < help_text_len)
	{
		int32_t line_length = 0;
		while (text_read < help_text_len)
		{
			int32_t chars_to_space_w = find_space(option.help_text + text_read + 1);
			int32_t chars_to_space = chars_to_space_w + 1;
			if (chars_to_space_w == -1)
			{
				chars_to_space = help_text_len - text_read;
			}
			if (chars_to_space + line_length >= 60 && line_length > 0)
			{
				break;
			}

			strs_appendn(&split_text, option.help_text + text_read, chars_to_space);
			line_length += chars_to_space;
			text_read += chars_to_space;
		}
		if (text_read >= help_text_len)
		{
			break;
		}

		strs_append(&split_text, "\n                    ");
		text_read += 1;
	}

	char help_text[split_text.length + strlen(help_indent) + 10];
	assert(sprintf(help_text, "%s  %s", help_indent, split_text.content) >= 0);

	int result = printf("  -%c, --%-10s%s\n", option.short_form, mod_long_form,
	                    help_text);
	if (result < 0)
	{
		return false;
	}

	return true;
}

bool print_help(const char* program_name)
{
	int result = printf("Usage: %s [OPTION]... legislation_act_url\n\n",
	                    program_name);
	if (result < 0)
	{
		return false;
	}
	for (int32_t option_index = 0; ; option_index++)
	{
		struct option_parameter option = options[option_index];
		if (is_null_option(option))
		{
			break;
		}

		if (!print_option_help(option))
		{
			return false;
		}
	}

	return true;
}

bool parse_args(struct user_args* result, int argc, char const* argv[])
{
	assert(result != NULL);

	int32_t argument_count = 0;
	struct user_args args = { 0 };
	bool keep_parsing_options = true;
	struct option_parameter option = {0};
	const char* option_argument;
	for (int32_t i = 1; i < argc; i++)
	{
		const char* argument = argv[i];
		if (!is_null_option(option) && option.argument_name != NULL
		        && *option_argument == '\0')
		{
			option_argument = argument;
		}
		else if (argument[0] != '-' || !keep_parsing_options || argument[1] == '\0')
		{
			argument_count++;
			if (argument_count == 1)
			{
				args.url = argv[i];
			}
			else
			{
				goto error;
			}
		}
		else if (argument[1] != '-')
		{
			const char* iter_buffer = begin_parse_short_options(argument);
			bool success = parse_next_short_option(&option, &option_argument, &iter_buffer);
			while (success && !is_null_option(option) && option.argument_name == NULL)
			{
				args = option.set_option_data(args, "");
				success = parse_next_short_option(&option, &option_argument, &iter_buffer);
			}

			if (!success)
			{
				goto error;
			}
		}
		else if (argument[2] == '\0')
		{
			keep_parsing_options = false;
		}
		else if (!parse_long_option(&option, &option_argument, argument))
		{
			goto error;
		}

		if (!is_null_option(option) && (option.argument_name == NULL
		                                || *option_argument != '\0'))
		{
			args = option.set_option_data(args, option_argument);
			option = (struct option_parameter)
			{
				0
			};
			option_argument = "";
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
