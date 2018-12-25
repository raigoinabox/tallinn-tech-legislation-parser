# pragma once

#include <stdint.h>
#include <stdbool.h>

#include "vectors.h"

struct str_builder
{
    struct vector content;
};

#define str_null { vec_null }

int32_t str_length(struct str_builder);
char* str_content(struct str_builder*);
struct str_builder str_substring(struct str_builder string, int limit);

void str_appendc(struct str_builder* string, const char* content);
void str_appendn(struct str_builder* string, const char* content, int32_t count);
void str_appendf(struct str_builder* string_p, const char* template, ...);

struct str_builder str_init();
void str_builder_destroy(struct str_builder* string);

char* str_from_long(long value);
char* str_from_double(double value);
char* str_copy(const char* cstring);
char* str_format(const char* template, ...);
bool str_equal(const char* str1, const char* str2);
bool str_empty(const char* str);
const char* str_find(const char* from, const char* substring);
struct vector str_split(char* from, const char* split_string);
char* str_trim(char* string);
