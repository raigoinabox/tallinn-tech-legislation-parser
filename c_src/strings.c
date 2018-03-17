#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "strings.h"
#include "util.h"

struct string str_c(const char* text)
{
    assert(text != NULL);
    size_t text_len = strlen(text);
    struct string string;
    string.length = text_len;
    string.type = CONSTANT;
    vec_init_c(string.content, (char* ) text, text_len + 1);

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

bool str_is_empty(struct string string)
{
    return str_length(string) <= 0;
}

/*
 * DYNAMIC or STATIC
 */

static void expand(struct string* string)
{
    assert(string->type == DYNAMIC);
    _vec_expand(string->content);
}

static void str_append_char(struct string* string_p, char character)
{
    assert(string_p != NULL);
    assert(character != '\0');
    struct string string = *string_p;
    assert(string.type == DYNAMIC || string.type == STATIC);

    if (vec_size(string.content) <= vec_length(string.content))
    {
        expand(&string);
    }
    vec_set(string.content, vec_length(string.content) - 1, character);
    vec_append(string.content, '\0');
    string.length += 1;

    *string_p = string;
}

struct string str_init_s(char* buffer, int32_t size)
{
    struct string string;
    string.length = 0;
    string.type = STATIC;
    vec_init_sta(string.content, buffer, size);
    vec_append(string.content, '\0');

    return string;
}

void str_append(struct string* string_p, struct string text)
{
    for (int32_t i = 0; i < str_length(text); i++)
    {
        str_append_char(string_p, str_content(text)[i]);
    }
}

void str_appends(struct string* string, const char* text)
{
    str_append(string, str_c(text));
}

void str_appendn(struct string* string_p, const char* text, int32_t count)
{
    assert(text != NULL);

    while (*text != '\0' && 0 < count)
    {
        str_append_char(string_p, *text);
        text += 1;
        count -= 1;
    }
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
    vec_set(string.content, 0, '\0');
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
    vec_init_siz(string.content, size);
    vec_append(string.content, '\0');

    return string;
}

void str_free(struct string* string_p)
{
    struct string string = *string_p;
    assert(string.type == DYNAMIC);
    vec_free(string.content);
    string.length = 0;
    *string_p = string;
}
