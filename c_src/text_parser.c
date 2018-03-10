#include "text_parser.h"

#include <assert.h>
#include <ctype.h>
#include <pcre.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strings.h"

static bool str_is_prefix_ci(const char* text, const char* prefix)
{
    while (*prefix != 0)
    {
        if (*text == 0 || tolower(*text) != tolower(*prefix))
        {
            return false;
        }

        text++;
        prefix++;
    }

    return true;
}

static const char* parse_prefix(const char* char_p)
{
    const char prefix[] = "section";
    if (!str_is_prefix_ci(char_p, prefix))
    {
        return NULL;
    }
    char_p += strlen(prefix);

    if (tolower(*char_p) == 's')
    {
        char_p++;
    }

    return char_p;
}

static const char* parse_spaces(const char* char_p)
{
    if (*char_p != ' ')
    {
        return NULL;
    }
    while (*char_p == ' ')
    {
        char_p++;
    }

    return char_p;
}

// return value:
// 0 means no error
// 1 means the section reference was missing
// 2 means that the text is something the parser doesn't recognize
static int32_t parse_section_reference(struct string* result_p,
                                       const char** char_pp)
{
    const char* char_p = *char_pp;
    bool reference_exists;
    struct string result;
    bool result_string_initialized = false;
    if (isdigit(*char_p))
    {
        reference_exists = true;

        char ref_buffer[10];
        int32_t ref_buf_i = 0;
        while (isdigit(*char_p))
        {
            if (ref_buf_i >= 10 - 1)
            {
                abort();
            }
            ref_buffer[ref_buf_i] = *char_p;

            ref_buf_i++;
            char_p++;
        }
        while (*char_p >= 'A' && *char_p <= 'Z')
        {
            assert(ref_buf_i < 10 - 1);
            ref_buffer[ref_buf_i] = *char_p;

            ref_buf_i++;
            char_p++;
        }
        ref_buffer[ref_buf_i] = 0;

        result = str_init_ds(ref_buf_i + 2);
        result_string_initialized = true;
        str_appends(&result, ref_buffer);
    }
    else if (*char_p == '(')
    {
        reference_exists = false;
    }
    else
    {
        goto error;
    }

    while (*char_p == '(')
    {
        char_p++;
        if (isdigit(*char_p))
        {
            while (isdigit(*char_p))
            {
                char_p++;
            }
            int32_t failsafe = 0;
            while (*char_p >= 'A' && *char_p <= 'Z')
            {
                assert(failsafe < 100);
                char_p++;
            }
            if (*char_p != ')')
            {
                goto error;
            }
        }
        else if (*(char_p + 1) == ')')
        {
            char_p++;
        }
        else
        {
            goto error;
        }
        char_p++;
    }

    if (result_string_initialized)
    {
        *result_p = result;
    }

    *char_pp = char_p;

    if (reference_exists)
    {
        return 0;
    }
    else
    {
        return 1;
    }

error:
    if (result_string_initialized)
    {
        str_free(&result);
    }
    return 2;
}

// doesn't handle cases:
// section 119A(4) of the Enterprise Act 2002 (c. 40) (which applies to functions
static struct section_references get_references_from_match(const char* text)
{
    struct section_references result;
    vec_init(result);

    const char* char_p = text;

    char_p = parse_prefix(char_p);
    if (char_p == NULL)
    {
        goto parse_end;
    }

    char_p = parse_spaces(char_p);
    if (char_p == NULL)
    {
        goto parse_end;
    }

    struct string reference;
    int32_t error = parse_section_reference(&reference, &char_p);
    if (error > 0)
    {
        goto parse_end;
    }
    vec_append(result, str_content(reference));

    bool had_space;
    while (true)
    {
        had_space = false;
        if (*char_p == ' ')
        {
            had_space = true;
            while (*char_p == ' ')
            {
                char_p++;
            }
        }

        bool is_from_to = false;
        if (*char_p == ',')
        {
            char_p++;
            while (*char_p == ' ')
            {
                char_p++;
            }
        }
        else if (had_space)
        {
            if (str_is_prefix_ci(char_p, "and"))
            {
                char_p += strlen("and");
            }
            else if (str_is_prefix_ci(char_p, "or"))
            {
                char_p += strlen("or");
            }
            else if (str_is_prefix_ci(char_p, "to"))
            {
                char_p += strlen("to");
                is_from_to = true;
            }
            else
            {
                break;
            }

            char_p = parse_spaces(char_p);
            if (char_p == NULL)
            {
                goto parse_end;
            }
        }
        else
        {
            goto parse_end;
        }

        struct string previous_reference = reference;
        error = parse_section_reference(&reference, &char_p);
        if (error > 1)
        {
            goto parse_end;
        }
        else if (error < 1)
        {
            // what to with section 199A to 210?
            if (is_from_to)
            {
                for (long int i = strtol(str_content(previous_reference), NULL, 10) + 1;
                        i < strtol(str_content(reference), NULL, 10); i++)
                {
                    struct string i_string = str_init();
                    int chars = snprintf(str_content(i_string), 10, "%ld", i);
                    if (chars <= 0)
                    {
                        abort();
                    }

                    vec_append(result, str_content(i_string));
                }
            }
            vec_append(result, str_content(reference));
        }
    }

    if (had_space)
    {
        char const of_suffix[] = "of ";
        if (str_is_prefix_ci(char_p, of_suffix))
        {
            char_p += strlen(of_suffix);
            if (!str_is_prefix_ci(char_p, "this act"))
            {
                goto ignore_references;
            }
        }
    }

parse_end:
    return result;

ignore_references:
    free_references_deep(&result);
    vec_init(result);
    return result;
}

void free_references_deep(struct section_references* references_p)
{
    struct section_references references = *references_p;

    for (int32_t i = 0; i < vec_length(references); i++)
    {
        free(vec_elem(references, i));
    }

    vec_free(references);

    *references_p = references;
}

struct section_references get_references_from_text(const char* text)
{
    struct section_references result;
    vec_init(result);

    const char* error_message;
    int error_offset;
    pcre* regex = pcre_compile("sections?+ ++\\d++", PCRE_CASELESS,
                               &error_message, &error_offset, NULL);
    if (regex == NULL)
    {
        abort();
    }

    int32_t ovector_size = 30;
    int ovector[30];
    int text_len = strlen(text);
    int matches = pcre_exec(regex, NULL, text, text_len, 0, 0, ovector,
                            ovector_size);
    while (matches > 0)
    {
        struct section_references refs_from_match = get_references_from_match(
                    text + ovector[0]);
        for (int32_t i = 0; i < vec_length(refs_from_match); i++)
        {
            char* const reference = vec_elem(refs_from_match, i);
            vec_append(result, reference);
        }
        vec_free(refs_from_match);

        matches = pcre_exec(regex, NULL, text, text_len, ovector[1], 0, ovector,
                            ovector_size);
    }
    if (matches != -1)
    {
        abort();
    }

    pcre_free(regex);
    return result;
}
