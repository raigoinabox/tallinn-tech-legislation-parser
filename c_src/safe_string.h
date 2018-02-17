# pragma once

#include <stdint.h>
#include <stdbool.h>

struct string {
	char* content;
	int32_t size;
	int32_t length;
	void (*append)(struct string* string, const char* content);
	void (*appendn)(struct string* string, const char* content, int32_t count);
	void (*expand)(struct string* string);
	void (*free)(struct string* string);
};

// string on a stack, meaning a fixed size
struct string str_init_s(char* buffer, int32_t size);
// string on a heap, meaning a dynamic size
struct string str_init();
struct string str_init_ds(int32_t size);
struct string str_init_c(const char* string);
void str_free(struct string* string);
int32_t str_length(struct string string);
char* str_content(struct string string);
void str_append(struct string* string, const char* content);
void str_appendn(struct string* string, const char* content, int32_t count);
void str_appends(struct string* string_p, struct string text);
void str_appendf(struct string* string_p, const char* template, ...);
void str_clear(struct string* string_p);
bool string_is_empty(struct string string);
