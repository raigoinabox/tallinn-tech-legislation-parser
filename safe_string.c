#include "safe_string.h"

#include <assert.h>

struct string_s strs_init(char* string, int32_t size)
{
	assert(0 < size);

	string[0] = '\0';
	return (struct string_s) {
		.content = string,
		.size = size,
		.length = 0
	};
}

struct string_s strs_initn(char* string, int32_t size, int32_t length)
{
	assert(length < size);

	string[length] = '\0';
	return (struct string_s) {
		.content = string,
		.size = size,
		.length = length
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
