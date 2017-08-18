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

// Read only string
struct string_c
{
	const char* content;
	int32_t size;
	int32_t length;
};

struct string_s strs_init(char* string, int32_t size);
struct string_s strs_initn(char* string, int32_t size, int32_t length);
void strs_append(struct string_s* string_p, const char* content);
void strs_appendn(struct string_s* string_p, const char* content, int32_t count);
