#include "safe_string.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

static void expand_d(struct string* string_p) {
	struct string string = *string_p;

	string.size *= 2;
	string.content = realloc(string.content, string.size * sizeof(*string.content));
	if (string.content == NULL)
	{
		abort();
	}
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

	struct string string = *string_p;
	while (*text != '\0')
	{
		if (string.size - 1 <= string.length) {
			string.expand(&string);
		}
		string.content[string.length] = *text;
		string.length += 1;
		text++;
	}
	string.content[string.length] = '\0';

	*string_p = string;
}

static void append_c(struct string* string_p, const char* text) {
	(void) string_p;
	(void) text;
	assert(false);
}

static void appendn(struct string* string_p, const char* text, int32_t count)
{
	assert(string_p != NULL);
	assert(text != NULL);

	struct string string = *string_p;
	for (int i = 0; i < count && *text != '\0'; i += 1)
	{
		if (string.size - 1 <= string.length) {
					string.expand(&string);
				}string.content[string.length] = *text;
		string.length += 1;
		text += 1;
	}
	string.content[string.length] = '\0';

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

	free(string.content);
	string.length = 0;
	string.size = 0;

	*string_p = string;
}

struct cstring cst_init(const char* text) {
	size_t text_len = strlen(text);
	struct cstring cstring;
	vec_init_c(cstring.vector, text, text_len + 1);
	cstring.string_length = text_len;
	return cstring;
}

struct cstring cst_from_str(struct string string) {
	struct cstring cstr;
	vec_init_c(cstr.vector, string.content, string.size);
	cstr.string_length = string.length;
	return cstr;
}

const char* cst_content(struct cstring string) {
	return vec_content(string.vector);
}

size_t cst_length(struct cstring string) {
	return string.string_length;
}

struct string str_init_s(char* buffer, int32_t size) {
	assert(0 < size);
	buffer[0] = '\0';
	return (struct string)
	{
		.content = buffer,
		.size = size,
		.length = 0,
		.append = append,
		.appendn = appendn,
		.expand = expand_s,
		.free = str_free_s
	};
}

struct string str_init()
{
	return str_init_ds(16);
}

struct string str_init_ds(int32_t size) {
	assert(0 < size);

	struct string string =
	{
		.content = malloc_a(size, sizeof(*string.content)),
		.size = size,
		.length = 0,
		.append = append,
		.appendn = appendn,
		.expand = expand_d,
		.free = str_free_d
	};
	string.content[0] = '\0';

	return string;
}

struct string str_init_c(const char* text) {
	size_t text_len = strlen(text);
	struct string string =
	{
		.content = (char*) text,
		.size = text_len + 1,
		.length = text_len,
		.append = append_c,
		.appendn = appendn_c,
		.expand = expand_s,
		.free = str_free_c
	};

	return string;
}

void str_free(struct string* string_p) {
	string_p->free(string_p);
}

int32_t str_length(struct string string) {
	return string.length;
}

char* str_content(struct string string) {
	return string.content;
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
	string.content[0] = '\0';
	*string_p = string;
}

bool string_is_empty(struct string string)
{
	return str_length(string) == 0;
}
