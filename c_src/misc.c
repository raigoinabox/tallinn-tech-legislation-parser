/*
 * misc.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "misc.h"

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

static struct string filter_triple_dots(struct string text)
{
    struct string filtered_text = str_init();

    for (int i = 0; i < str_length(text); i++)
    {
        if (str_elem(text, i) == '.' && str_elem(text, i + 1) == '.'
                && str_elem(text, i + 2) == '.')
        {
            str_append(&filtered_text, str_substring_end(text, 0, i));

            i += 2;
            text = str_substring(text, i + 1);
        }
    }
    str_append(&filtered_text, text);

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
    struct string section_number;
    struct string section_text;
};

static void sax_start_element(void* user_data, const xmlChar* name,
                              const xmlChar** attributes)
{
    (void) name;

    struct SAX_state* state_p = user_data;
    struct SAX_state state = *state_p;
    if (0 < state.section_depth)
    {
        state.section_depth += 1;
        goto end;
    }

    if (attributes == NULL)
    {
        goto end;
    }

    for (int i = 0; attributes[i] != NULL; i += 2)
    {
        const size_t prefix_length = strlen("section-");
        if (xmlStrcmp(attributes[i], (xmlChar*) "id") == 0
                && xmlStrncmp(attributes[i + 1], (xmlChar*) "section-",
                              prefix_length) == 0)
        {
            const xmlChar* id = attributes[i + 1];
            state.section_number = str_init();
            str_appends(&state.section_number, char_from_xml(id + prefix_length));
            state.section_depth = 1;
            state.section_text = str_init();
            goto end;
        }
    }

end:
    *state_p = state;
}

static void sax_end_element(void* user_data, const xmlChar* name)
{
    (void) name;
    struct SAX_state* state_p = user_data;
    struct SAX_state state = *state_p;
    if (1 < state.section_depth)
    {
        state.section_depth -= 1;
    }
    else if (0 < state.section_depth)
    {
        state.section_depth -= 1;
        struct string node_text_w_three_dots = state.section_text;
        struct string node_text = filter_triple_dots(node_text_w_three_dots);

        struct section_references references = get_references_from_text(node_text);
        str_free(&node_text);

        struct section section;
        section.id = state.section_number;
        section.references = references;
        vec_append_old(state.result, section);
        str_free(&state.section_text);
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
    (void) user_data;
    va_list args;

    va_start(args, msg);
    printf_ea("sax_warning: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static void sax_error(void *user_data, const char *msg, ...)
{
    (void) user_data;
    (void) msg;
//    va_list args;

//    va_start(args, msg);
//    fprintf_a(stderr, "sax_error: ");
//    vfprintf(stderr, msg, args);
//    va_end(args);
}

static void sax_fatal_error(void *user_data, const char *msg, ...)
{
    (void) user_data;
    va_list args;

    va_start(args, msg);
    printf_ea("sax_fatal_error: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static xmlSAXHandler sax = { .startElement = sax_start_element, .endElement =
                                 sax_end_element, .characters = sax_characters, .warning = sax_warning,
                             .error = sax_error, .fatalError = sax_fatal_error
                           };

static bool get_sections_from_page(struct section_vec* result, struct string page)
{
    struct SAX_state sax_state =
    { .section_depth = 0 };
    vec_init_old(sax_state.result);

    if (xmlSAXUserParseMemory(&sax, &sax_state, str_content(page),
                              str_length(page)) != 0)
    {
        assert(vec_length_old(sax_state.result) <= 0);
        vec_free(sax_state.result);
        return false;
    }
    *result = sax_state.result;
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

bool get_sections_from_legislation(struct section_vec* result,
                                   struct leg_id legislation)
{
    struct string url = get_leg_api_url(legislation);

    char curl_error_string[CURL_ERROR_SIZE];
    struct string page;
    bool success = get_web_page(&page, str_content(url),
                                    curl_error_string);
    if (!success)
    {
        assert(strlen(curl_error_string) > 0);
        fprintf(stderr, "%s\n", curl_error_string);
        goto end;
    }

    success = get_sections_from_page(result, page);
end:
    str_free(&page);
    str_free(&url);
    return success;
}

struct string fit_text(const char* text, int32_t prefix_length)
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
    char split_text_buf[help_text_len * 2 + 10];
    struct string split_text = str_init_s(split_text_buf,
                                          sizeof(split_text_buf));
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

        str_appends(&split_text, "\n                    ");
        text_read += 1;
    }

    struct string help_text = str_init();
    str_appendf(&help_text, "%s  %s", help_indent, str_content(split_text));
    return help_text;
}
