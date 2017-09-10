# pragma once

#include <stdint.h>

// String on a stack
struct string_s
{
	char* content;
	int32_t size;
	int32_t length;
};

// String on a heap
struct string_d
{
	char* content;
	int32_t size;
	int32_t length;
};

struct string_s strs_init(char* string, int32_t size);
void strs_append(struct string_s* string_p, const char* content);
void strs_appendn(struct string_s* string_p, const char* content,
                  int32_t count);

struct string_d strd_mallocn(int32_t size);
void strd_free(struct string_d* string_p);
void strd_append(struct string_d* string_p, const char* text);
void strd_appends(struct string_d* string_p, struct string_d text);
void strd_clear(struct string_d* string_p);
int32_t strd_length(struct string_d string);
char* strd_content(struct string_d string);
