#include "command_line.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include <assert.h>
#include <string.h>

#include "vectors.h"
#include "arg_parsing.h"
#include "misc.h"
#include "printing.h"
#include "strings.h"
#include "util.h"

bool col_print_init_help(const char* program_name,
                         struct arp_option_vec options,
                         struct command_vec commands)
{
    int result = printf("Usage: %s [OPTION] <command> [<args>] \n",
                        program_name);
    if (result < 0)
    {
        return false;
    }
    printf_a("\n");
    arp_print_options_help(options);
    printf_a("\n");
    printf_a("Possible commands are:\n");
    for (int32_t i = 0; i < vec_length(commands); i++)
    {
        struct command command = vec_elem(commands, i);
        struct string description = fit_text(command.description,
                                             strlen(command.command) + 3);
        printf_a("   %s%s\n", command.command, str_content(description));
        str_free(&description);
    }
    printf_a("\n");
    printf_a("Write %s <command> -h for more information about the command.\n",
             program_name);

    return true;
}

bool col_print_command_help(const char* program_name, const char* command,
                            struct arp_option_vec options, const char* argument_string)
{
    int result = printf(
                     "Usage: %s %s [OPTION]... %s\n\n",
                     program_name, command, argument_string);
    if (result < 0)
    {
        return false;
    }

    arp_print_options_help(options);
    return true;
}
