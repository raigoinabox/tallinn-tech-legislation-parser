#include "util.h"

#include <stdarg.h>
#include <stdlib.h>

void* malloc_a(size_t nmemb, size_t memb_size)
{
	void* result = malloc(nmemb * memb_size);
	if (result == NULL)
	{
		perror("malloc");
		abort();
	}
	return result;
}

void* realloc_a(void* pointer, size_t nmemb, size_t memb_size)
{
	void* result = realloc(pointer, nmemb * memb_size);
	if (result == NULL)
	{
		perror("realloc");
		abort();
	}
	return result;
}

int printf_a(const char* template, ...) {
	va_list args;
	va_start(args, template);
	int bytes_printed = vprintf(template, args);
	va_end(args);
	if (bytes_printed < 0) {
		perror("vprintf");
		abort();
	}
	return bytes_printed;
}

int fprintf_a(FILE* file, const char* template, ...) {
	va_list args;
	va_start(args, template);
	int bytes_printed = vfprintf(file, template, args);
	va_end(args);
	if (bytes_printed < 0) {
		perror("vfprintf");
		abort();
	}
	return bytes_printed;
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
