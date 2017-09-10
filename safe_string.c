#include "safe_string.h"

#include <assert.h>

#include "util.h"

struct string_s strs_init(char* string, int32_t size)
{
	assert(0 < size);

	string[0] = '\0';
	return (struct string_s)
	{
		.content = string,
		 .size = size,
		  .length = 0
	};
}

void strs_append(struct string_s* string_p, const char* content)
{
	struct string_s string = *string_p;
	while (*content != '\0')
	{
		assert(string.length < string.size - 1);
		string.content[string.length] = *content;
		string.length += 1;
		content++;
	}
	string.content[string.length] = '\0';

	*string_p = string;
}

void strs_appendn(struct string_s* string_p, const char* content, int32_t count)
{
	struct string_s string = *string_p;
	for (int i = 0; i < count && *content != '\0'; i += 1)
	{
		assert(string.length < string.size - 1);
		string.content[string.length] = *content;
		string.length += 1;
		content += 1;
	}
	string.content[string.length] = '\0';

	*string_p = string;
}

struct string_d strd_mallocn(int32_t size)
{
	assert(0 < size);

	struct string_d string =
	{
		.content = malloc_a(size, sizeof(*string.content)),
		.size = size,
		.length = 0
	};
	string.content[0] = '\0';

	return string;
}

void strd_free(struct string_d* string_p)
{
	struct string_d string = *string_p;

	free(string.content);
	string.length = 0;
	string.size = 0;

	*string_p = string;
}

void strd_append(struct string_d* string_p, const char* text)
{
	assert(string_p != NULL);
	assert(text != NULL);

	struct string_d string = *string_p;
	while (*text != '\0')
	{
		if (string.size - 1 <= string.length)
		{
			string.size *= 2;
			string.content = realloc(string.content, string.size * sizeof(*string.content));
			if (string.content == NULL)
			{
				abort();
			}
		}
		string.content[string.length] = *text;
		string.length += 1;
		text++;
	}
	string.content[string.length] = '\0';

	*string_p = string;
}

void strd_appends(struct string_d* string_p, struct string_d text)
{
	strd_append(string_p, strd_content(text));
}

void strd_clear(struct string_d* string_p)
{
	struct string_d string = *string_p;
	string.length = 0;
	string.content[0] = '\0';
	*string_p = string;
}

int32_t strd_length(struct string_d string)
{
	return string.length;
}

char* strd_content(struct string_d string)
{
	return string.content;
}
