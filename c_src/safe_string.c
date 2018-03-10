#include "safe_string.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

static void expand_d(struct string* string_p) {
	struct string string = *string_p;
	_vec_expand(string.content);
	*string_p = string;
}

static void expand_s(struct string* string_p) {
	assert(false);
	expand_d(string_p);
}

static void append(struct string* string_p, const char* text)
{
	assert(string_p != NULL);
	assert(text != NULL);

	if (*text == '\0') {
		return;
	}

	struct string string = *string_p;
	vec_set(string.content, vec_length(string.content) - 1, *text);
	string.length += 1;
	text += 1;

	while (*text != '\0')
	{
		if (vec_size(string.content) - 1 <= vec_length(string.content)) {
			string.expand(&string);
		}
		vec_append(string.content, *text);
		string.length += 1;
		text += 1;
	}
	vec_append(string.content, '\0');

	*string_p = string;
}

static void append_c(struct string* string_p, const char* text) {
	(void) string_p;
	(void) text;
	assert(false);
}

static void appendn(struct string* string_p, const char* text, int32_t count) {
	assert(string_p != NULL);
	assert(text != NULL);
	if (*text == '\0' || count <= 0) {
		return;
	}

	struct string string = *string_p;
	vec_set(string.content, vec_length(string.content) - 1, *text);
	string.length += 1;
	text += 1;
	count--;

	while (*text != '\0' && 0 < count) {
		if (vec_size(string.content) - 1 <= vec_length(string.content)) {
			string.expand(&string);
		}
		vec_append(string.content, *text);
		string.length += 1;
		text += 1;
		count--;
	}
	vec_append(string.content, '\0');

	*string_p = string;
}

static void appendn_c(struct string* string_p, const char* text, int32_t count) {
	(void) string_p;
	(void) text;
	(void) count;
	assert(false);
}

static void str_free_s(struct string* string_p) {
	assert(false);
	string_p->length = 0;
}

static void str_free_c(struct string* string_p) {
	(void) string_p;
	assert(false);
}

static void str_free_d(struct string* string_p) {
	struct string string = *string_p;

	vec_free(string.content);
	string.length = 0;

	*string_p = string;
}

struct cstring cst_init(const char* text) {
	size_t text_len = strlen(text);
	struct cstring cstring;
	vec_init_c(cstring.content, text, text_len + 1);
	cstring.length = text_len;
	return cstring;
}

struct cstring cst_from_str(struct string string) {
	struct cstring cstr;
	vec_init_c(cstr.content, string.content.content, vec_length(string.content));
	cstr.length = string.length;
	return cstr;
}

const char* cst_content(struct cstring string) {
	return vec_content(string.content);
}

int32_t cst_length(struct cstring string) {
	return string.length;
}

struct string str_init_s(char* buffer, int32_t size) {
	struct string string = {
		.length = 0,
		.append = append,
		.appendn = appendn,
		.expand = expand_s,
		.free = str_free_s
	};
	vec_init_sta(string.content, buffer, size);
	vec_append(string.content, '\0');

	return string;
}

struct string str_init()
{
	return str_init_ds(16);
}

struct string str_init_ds(int32_t size) {
	struct string string =
	{
		.length = 0,
		.append = append,
		.appendn = appendn,
		.expand = expand_d,
		.free = str_free_d
	};
	vec_init_siz(string.content, size);
	vec_append(string.content, '\0');

	return string;
}

struct string str_init_c(char* text) {
	size_t text_len = strlen(text);
	struct string string =
	{
		.length = text_len,
		.append = append_c,
		.appendn = appendn_c,
		.expand = expand_s,
		.free = str_free_c
	};
	vec_init_c(string.content, text, text_len + 1);

	return string;
}

void str_free(struct string* string_p) {
	string_p->free(string_p);
}

int32_t str_length(struct string string) {
	return string.length;
}

char* str_content(struct string string) {
	return string.content.content;
}

void str_append(struct string* string, const char* content) {
	string->append(string, content);
}

void str_appendn(struct string* string, const char* content, int32_t count) {
	string->appendn(string, content, count);
}

void str_appends(struct string* string_p, struct cstring text)
{
	str_append(string_p, cst_content(text));
}

bool str_append_char(struct string* string_p, char character) {
	if (character == '\0') {
		return false;
	}
	char text[10];
	sprintf(text, "%c", character);
	str_append(string_p, text);
	return true;
}

void str_appendf(struct string* string_p, const char* template, ...) {
	va_list args, args_copy;
	va_start(args, template);
	va_copy(args_copy, args);

	int bytes_needed = vsnprintf(NULL, 0, template, args_copy);
	char text[bytes_needed + 10];
	vsprintf(text, template, args);
	str_append(string_p, text);

	va_end(args_copy);
	va_end(args);
}

void str_clear(struct string* string_p)
{
	struct string string = *string_p;
	string.length = 0;
	vec_set(string.content, 0, '\0');
	*string_p = string;
}

bool string_is_empty(struct string string)
{
	return str_length(string) == 0;
}
