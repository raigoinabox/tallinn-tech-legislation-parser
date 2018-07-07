#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "strings.h"
#include "util.h"

static struct string str_c_size(const char* text, int limit)
{
    assert(text != NULL);
    size_t text_len = 0;
    while (0 < limit && text[text_len] != '\0')
    {
        text_len++;
        limit--;
    }

    struct string string;
    string.length = text_len;
    string.type = CONSTANT;
    vec_c(&string.content, text, text_len + 1, sizeof(char));

    return string;
}

struct string str_c(const char* text)
{
    assert(text != NULL);
    size_t text_len = strlen(text);
    struct string string;
    string.length = text_len;
    string.type = CONSTANT;
    vec_c(&string.content, text, text_len + 1, sizeof(char));

    return string;
}

int32_t str_length(struct string string)
{
    return string.length;
}

char* str_content(struct string string)
{
    return string.content.content;
}

char str_elem(struct string text, int index)
{
    return str_content(text)[index];
}

bool str_is_empty(struct string string)
{
    return str_length(string) <= 0;
}

bool str_is_equal(struct string str1, struct string str2)
{
    if (str_length(str1) != str_length(str2))
    {
        return false;
    }
    for (int i = 0; i < str_length(str1); i++)
    {
        if (str_elem(str1, i) != str_elem(str2, i))
        {
            return false;
        }
    }
    return true;
}

struct string str_substring(struct string string, int begin_index)
{
    return str_substring_end(string, begin_index, string.length);
}

struct string str_substring_end(struct string string, int begin_index,
                                int end_index)
{
    struct string substring = string;
    substring.content = vec_subvector_end(substring.content, begin_index,
                                          end_index);
    substring.length = end_index - begin_index;
    return substring;
}

/*
 * DYNAMIC or STATIC
 */

static void str_reserve(struct string* string_p, int elem_count)
{
    struct string string = *string_p;
    assert(vec_length(string.content) < vec_capacity(string.content)
           || string.type == DYNAMIC);
    vec_reserve(&string.content, elem_count);
    *string_p = string;
}

struct string str_init_s(char* buffer, int32_t size)
{
    struct string string;
    string.length = 0;
    string.type = STATIC;
    vec_init_sta(string.content, buffer, size);
    char null = '\0';
    vec_append(&string.content, &null);

    return string;
}

void str_append(struct string* string_p, struct string append)
{
    if (str_length(append) <= 0)
    {
        return;
    }
    assert(string_p != NULL);
    struct string string = *string_p;
    assert(string.type == DYNAMIC || string.type == STATIC);

    str_reserve(&string, str_length(append));

    char character = str_content(append)[0];
    vec_set(&string.content, vec_length(string.content) - 1, &character);
    string.length += 1;

    for (int32_t i = 1; i < str_length(append); i++)
    {
        char character = str_elem(append, i);
        vec_append(&string.content, &character);
        string.length += 1;
    }

    char null = '\0';
    vec_append(&string.content, &null);

    *string_p = string;
}

void str_appends(struct string* string, const char* text)
{
    str_append(string, str_c(text));
}

void str_appendn(struct string* string_p, const char* characters, int32_t limit)
{
    assert(characters != NULL);
    struct string text = str_c_size(characters, limit);
    str_append(string_p, text);
}

void str_appendf(struct string* string_p, const char* template, ...)
{
    va_list args, args_copy;
    va_start(args, template);
    va_copy(args_copy, args);

    int bytes_needed = vsnprintf(NULL, 0, template, args_copy);
    char text[bytes_needed + 10];
    vsprintf(text, template, args);
    str_appends(string_p, text);

    va_end(args_copy);
    va_end(args);
}

void str_clear(struct string* string_p)
{
    struct string string = *string_p;
    assert(string.type == DYNAMIC || string.type == STATIC);
    string.length = 0;
    char null = '\0';
    vec_set(&string.content, 0, &null);
    string.content.length = 1;
    *string_p = string;
}

/*
 * DYNAMIC
 */

struct string str_init()
{
    return str_init_ds(16);
}

struct string str_init_ds(int32_t size)
{
    struct string string;
    string.length = 0;
    string.type = DYNAMIC;
    vec_init_size(&string.content, sizeof(char), size + 1);
    char null = '\0';
    vec_append(&string.content, &null);

    return string;
}

void str_free(struct string* string_p)
{
    struct string string = *string_p;
    assert(string.type == DYNAMIC);
    vec_destroy(&string.content);
    string.length = 0;
    *string_p = string;
}
