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
    vec_init_c2(&string.content, text, text_len + 1, sizeof(char));

    return string;
}

struct string str_c(const char* text)
{
    assert(text != NULL);
    size_t text_len = strlen(text);
    struct string string;
    string.length = text_len;
    string.type = CONSTANT;
    vec_init_c2(&string.content, text, text_len + 1, sizeof(char));

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

bool str_equal(struct string str1, struct string str2)
{
    return strcmp(str_content(str1), str_content(str2)) == 0;
}

/*
 * DYNAMIC or STATIC
 */

static void str_reserve(struct string* string_p, int elem_count)
{
    struct string string = *string_p;
    assert(vec_len(string.content) < vec_cap(string.content) || string.type == DYNAMIC);
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
    vec_append2(&string.content, &null);

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
    vec_set2(&string.content, vec_len(string.content) - 1, &character);
    string.length += 1;

    for (int32_t i = 1; i < str_length(append); i++)
    {
        char character = str_content(append)[i];
        vec_append2(&string.content, &character);
        string.length += 1;
    }

    char null = '\0';
    vec_append2(&string.content, &null);

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
    vec_set2(&string.content, 0, &null);
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
    vec_init_size2(&string.content, size, sizeof(char));
    char null = '\0';
    vec_append2(&string.content, &null);

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
