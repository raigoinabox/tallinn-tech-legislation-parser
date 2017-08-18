#include "util.h"

#include <stdlib.h>
#include <stdbool.h>

void* malloc_a(size_t nmemb, size_t memb_size)
{
	void* result = malloc(nmemb * memb_size);
	if (result == NULL)
	{
		abort();
	}
	return result;
}

bool str_is_prefix(const char* string, const char* prefix)
{
	while (*prefix == *string && *prefix != '\0')
	{
		prefix++;
		string++;
	}

	return *prefix == '\0';
}

int32_t str_tokenize(char* string, char delimit)
{
	for (int index = 0; string[index] != '\0'; index++)
	{
		if (string[index] == delimit)
		{
			string[index] = '\0';
			return index + 1;
		}
	}

	return -1;
}
