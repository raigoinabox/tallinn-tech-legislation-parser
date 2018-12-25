/*
 * misc.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "legal_act.h"

#include <assert.h>
#include <curl/curl.h>
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printing.h"
#include "strings.h"
#include "text_parser.h"
#include "util.h"
#include "web.h"

static struct str_builder filter_triple_dots(const char* text)
{
    struct str_builder filtered_text = str_init();

	const char* char_p = text;
	while (*char_p != '\0')
	{
		if (str_equal(char_p, "..."))
        {
			str_appendn(&filtered_text, text, char_p - text);
			char_p += 3;
			text = char_p;
        }
		else
		{
			char_p++;
		}
    }
	str_appendc(&filtered_text, text);

    return filtered_text;
}

static const char* char_from_xml(const xmlChar* text)
{
    return (const char*) text;
}

struct SAX_state
{
    struct section_vec result;
    int section_depth;
	char* section_number;
    struct str_builder section_text;
	struct error* error;
};

static void sax_start_element(void* user_data, const xmlChar* name,
                              const xmlChar** attributes)
{
    (void) name;

    struct SAX_state* state_p = user_data;
    struct SAX_state state = *state_p;
    if (1 <= state.section_depth)
    {
        state.section_depth += 1;
        str_appendc(&state.section_text, " ");
    } else if (attributes != NULL) {
        for (int i = 0; attributes[i] != NULL; i += 2)
        {
            const size_t prefix_length = strlen("section-");
            if (xmlStrcmp(attributes[i], (xmlChar*) "id") == 0
                    && xmlStrncmp(attributes[i + 1], (xmlChar*) "section-",
                                  prefix_length) == 0)
            {
                const xmlChar* id = attributes[i + 1];
				state.section_number = str_copy(
						char_from_xml(id + prefix_length));
                state.section_depth = 1;
                state.section_text = str_init();
                break;
            }
        }
    }

    *state_p = state;
}

static void sax_end_element(void* user_data, const xmlChar* name)
{
    (void) name;
    struct SAX_state* state_p = user_data;
    struct SAX_state state = *state_p;
    if (2 <= state.section_depth)
    {
        state.section_depth -= 1;
        str_appendc(&state.section_text, " ");
    }
    else if (1 <= state.section_depth)
    {
        state.section_depth -= 1;
		struct str_builder node_text = filter_triple_dots(
				str_content(&state.section_text));

        struct section_references references = get_references_from_text(node_text);

        struct section section;
		section.text = str_content(&node_text);
		section.id = state.section_number;
        section.references = references;
        vec_append_old(state.result, section);

        str_builder_destroy(&state.section_text);
    }

    *state_p = state;
}

static void sax_characters(void* user_data, const xmlChar* text, int len)
{
    struct SAX_state* state_p = user_data;
    struct SAX_state state = *state_p;
    if (0 < state.section_depth)
    {
        str_appendn(&state.section_text, char_from_xml(text), len);
    }
    *state_p = state;
}

static void sax_warning(void *user_data, const char *msg, ...)
{
	struct SAX_state* state = user_data;
	register_error(state->error, "sax_fatal_error");

	va_list args;

    va_start(args, msg);
    printf_ea("sax_warning: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static void sax_error(void *user_data, const char *msg, ...)
{
    (void) msg;

	struct SAX_state* state = user_data;
	register_error(state->error, "sax_error");
//    va_list args;

//    va_start(args, msg);
//    fprintf_a(stderr, "sax_error: ");
//    vfprintf(stderr, msg, args);
//    va_end(args);
}

static void sax_fatal_error(void *user_data, const char *msg, ...)
{
	struct SAX_state* state = user_data;
	register_error(state->error, "sax_fatal_error");
    va_list args;

    va_start(args, msg);
    printf_ea("sax_fatal_error: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static xmlSAXHandler sax =
{ .startElement = sax_start_element, .endElement = sax_end_element,
		.characters = sax_characters, .warning = sax_warning,
		.error = sax_error, .fatalError = sax_fatal_error };

static bool get_sections_from_page(
		struct section_vec* result, struct str_builder page, struct error* error)
{
	struct SAX_state sax_state =
	{ 0 };
    vec_init_old(sax_state.result);
	sax_state.error = error;

	bool success = xmlSAXUserParseMemory(
			&sax, &sax_state, str_content(&page), str_length(page)) == 0;
	if (!success)
    {
        assert(vec_length_old(sax_state.result) <= 0);
        vec_free(sax_state.result);
	}
	else
	{
		*result = sax_state.result;
    }

	register_frame(error);
	return success;
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

struct legal_act_id leg_init_c(
		char* type, char* year, char* number)
{
	struct legal_act_id leg =
	{ 0 };
	leg.type = type;
	leg.year = year;
	leg.number = number;
    return leg;
}

void leg_free(struct legal_act_id* legislation_p)
{
    struct legal_act_id legislation = *legislation_p;
	free(legislation.type);
	legislation.type = NULL;
	free(legislation.year);
	legislation.year = NULL;
	free(legislation.number);
	legislation.number = NULL;
    *legislation_p = legislation;
}

bool get_sections_from_legislation(
		struct section_vec* result, struct legal_act_id legislation,
		struct error* error)
{
	char* url = get_leg_api_url(legislation);

    struct str_builder page = { 0 };
	bool success = get_web_page(&page, url, error);
	if (success)
	{
		success = get_sections_from_page(result, page, error);
	}
	if (success)
	{
		remove_foreign_sections(*result, false);
	}

	register_frame(error);
    str_builder_destroy(&page);
	free(url);
    return success;
}

char* fit_text(const char* text, int32_t prefix_length)
{
    char help_indent[100];
    if (prefix_length > 18)
    {
        int print_result = sprintf(help_indent, "\n%18c", ' ');
        assert(print_result >= 0);
    }
    else
    {
        int i = 0;
        for (; i < 18 - prefix_length; i++)
        {
            help_indent[i] = ' ';
        }
        help_indent[i] = '\0';
    }

    int help_text_len = strlen(text);
    int32_t text_read = 0;
    struct str_builder split_text = str_init();
    while (text_read < help_text_len)
    {
        int32_t line_length = 0;
        while (text_read < help_text_len)
        {
            int32_t chars_to_space_w = find_space(text + text_read + 1);
            int32_t chars_to_space = chars_to_space_w + 1;
            if (chars_to_space_w == -1)
            {
                chars_to_space = help_text_len - text_read;
            }
            if (chars_to_space + line_length >= 60 && line_length > 0)
            {
                break;
            }

            str_appendn(&split_text, text + text_read, chars_to_space);
            line_length += chars_to_space;
            text_read += chars_to_space;
        }
        if (text_read >= help_text_len)
        {
            break;
        }

        str_appendc(&split_text, "\n                    ");
        text_read += 1;
    }

	char* help_text = str_format(
			"%s  %s", help_indent, str_content(&split_text));

    str_builder_destroy(&split_text);
	return help_text;
}
