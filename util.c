#include "util.h"

#include <stdlib.h>
#include <stdbool.h>

void* safe_malloc(size_t nmemb, size_t memb_size)
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

char* str_tokenize(char* string, char delimit)
{
	char* char_p = string;
	while (*char_p != '\0')
	{
		if (*char_p == delimit)
		{
			*char_p = '\0';
			return char_p + 1;
		}
	}

	return char_p;
}
