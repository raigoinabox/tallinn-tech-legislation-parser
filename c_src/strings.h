# pragma once

#include <stdint.h>
#include <stdbool.h>

#include "vectors.h"

struct string
{
    struct vector content;
    int32_t length;
    enum { DYNAMIC, STATIC, CONSTANT } type;
};

struct string str_c(const char* string);
int32_t str_length(struct string string);
char* str_content(struct string string);
char str_elem(struct string text, int index);
bool str_is_empty(struct string string);
bool str_is_equal(struct string str1, struct string str2);
struct string str_substring(struct string string, int limit);
struct string str_substring_end(struct string string, int begin_index,
                                int end_index);

// DYNAMIC or STATIC
struct string str_init_s(char* buffer, int32_t size);
void str_append(struct string* string_p, struct string text);
void str_appends(struct string* string, const char* content);
void str_appendn(struct string* string, const char* content, int32_t count);
void str_appendf(struct string* string_p, const char* template, ...);
void str_clear(struct string* string_p);

// DYNAMIC
struct string str_init();
struct string str_init_ds(int32_t size);
void str_free(struct string* string);
