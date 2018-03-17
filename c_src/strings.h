# pragma once

#include <stdint.h>
#include <stdbool.h>

#include "vectors.h"

struct string
{
    vec_struct(, char) content;
    int32_t length;
    enum { DYNAMIC, STATIC, CONSTANT } type;
};

// TODO printf and family (stdio.h?) should be used only in printing.h
// TODO if -d option is used, this should reflect in default filename
// TODO clean up parser

struct string str_c(const char* string);
int32_t str_length(struct string string);
char* str_content(struct string string);
bool str_is_empty(struct string string);

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
