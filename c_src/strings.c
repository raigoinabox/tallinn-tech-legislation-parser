#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "strings.h"
#include "util.h"

static bool is_zero_terminated(struct str_builder string)
{
	if (vec_length(string.content) <= 0)
	{
		return false;
	}
	else
	{
		char* char_p = vec_elem(string.content, vec_length(string.content) - 1);
		return *char_p == '\0';
	}
}

int32_t str_length(struct str_builder string)
{
	if (is_zero_terminated(string))
	{
		return vec_length(string.content) - 1;
	}
	else
	{
		return vec_length(string.content);
	}
}

char* str_content(struct str_builder* string)
{
	if (!is_zero_terminated(*string))
	{
		char null = '\0';
		vec_append(&string->content, &null);
	}
	return vec_content(string->content);
}

struct str_builder str_substring(struct str_builder string, int begin_index)
{
	struct str_builder substring = string;
	substring.content = vec_subvector(substring.content, begin_index);
	return substring;
}

static void str_reserve(struct str_builder* string_p, int elem_count)
{
	vec_reserve(&string_p->content, elem_count);
}

void str_appendc(struct str_builder* string_p, const char* text)
{
	if (str_empty(text))
	{
		return;
	}

	assert(string_p != NULL);
	struct str_builder string = *string_p;

	str_reserve(&string, strlen(text));

	char character = *text;
	while (character != '\0')
	{
		vec_append(&string.content, &character);
		text++;

		character = *text;
	}

	*string_p = string;
}

void str_appendn(struct str_builder* string_p, const char* characters, int32_t limit)
{

	if (str_empty(characters))
	{
		return;
	}

	assert(string_p != NULL);
	struct str_builder string = *string_p;

	str_reserve(&string, limit);

	char character = *characters;
	while (character != '\0' && 0 < limit)
	{
		vec_append(&string.content, &character);
		characters++;
		limit--;

		character = *characters;
	}

	*string_p = string;
}

void str_appendf(struct str_builder* string_p, const char* template, ...)
{
    va_list args, args_copy;
    va_start(args, template);
    va_copy(args_copy, args);

    int bytes_needed = vsnprintf(NULL, 0, template, args_copy);
    char text[bytes_needed + 10];
    vsprintf(text, template, args);
    str_appendc(string_p, text);

    va_end(args_copy);
    va_end(args);
}

/*
 * DYNAMIC
 */

struct str_builder str_init()
{
    struct str_builder string = { 0 };
    string.content = vec_init(sizeof(char));
    return string;
}

void str_builder_destroy(struct str_builder* string_p)
{
	struct str_builder string = *string_p;
	vec_destroy(&string.content);
	*string_p = string;
}

char* str_from_long(long value)
{
	return str_format("%ld", value);
}

char* str_from_double(double value)
{
	return str_format("%f", value);
}

char* str_copy(const char* from)
{
	size_t length = strlen(from);
	char* new_string = malloc_a(length + 1, sizeof(*from));
	for (size_t i = 0; i < length + 1; i++)
	{
		new_string[i] = from[i];
	}
	return new_string;
}

char* str_format(const char* template, ...)
{
	va_list args, args_copy;
	va_start(args, template);
	va_copy(args_copy, args);

	int bytes_needed = vsnprintf(NULL, 0, template, args_copy);
	char text[bytes_needed + 10];
	vsprintf(text, template, args);
	char* result = str_copy(text);

	va_end(args_copy);
	va_end(args);

	return result;
}

bool str_equal(const char* str1, const char* str2)
{
	return strcmp(str1, str2) == 0;
}

bool str_empty(const char* str)
{
	return str == NULL || str[0] == '\0';
}

const char* str_find(const char* from, const char* substring)
{
	int i = 0;
	int cmp_result;
	while (*from != '\0' && (cmp_result = strcmp(from, substring)) != 0)
	{
		i++;
		from++;
	}

	if (cmp_result == 0)
	{
		return from;
	}
	else
	{
		return NULL;
	}
}

// no string copying is done, pointers are calculated and nulls are inserted
// return vector of char*
struct vector str_split(char* from, const char* substring)
{
	struct vector split = vec_init(sizeof(const char*));
	vec_append(&split, &from);
	int sub_len = strlen(substring);
	while (*from != '\0')
	{
		if (strncmp(from, substring, sub_len) == 0)
		{
			*from = '\0';
			from += sub_len;
			vec_append(&split, &from);
		}
		else
		{
			from++;
		}
	}
	return split;
}

char* str_trim(char* string)
{
	while (*string != '\0' && (*string == '\n' || *string == ' '))
	{
		string++;
	}
	char* pointer = string;
	while (*pointer != '\0')
	{
		pointer++;
	}
	while (pointer != string && (*pointer == '\n' || *pointer == ' '))
	{
		pointer = '\0';
		pointer--;
	}

	return string;
}
