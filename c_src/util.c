#include "util.h"

#include <stdarg.h>
#include <stdlib.h>

void* malloc_a(size_t elem_count, size_t elem_size)
{
    if (SIZE_MAX / elem_count < elem_size)
    {
        fprintf(stderr, "error: requested too much memory\n");
        abort();
    }
    size_t malloc_size = elem_count * elem_size;
    void* result = malloc(malloc_size);
    if (result == NULL && malloc_size > 0)
    {
        perror("malloc");
        abort();
    }
    return result;
}

void* realloc_a(void* pointer, size_t nmemb, size_t memb_size)
{
    if (SIZE_MAX / nmemb < memb_size)
    {
        abort();
    }
    void* result = realloc(pointer, nmemb * memb_size);
    if (result == NULL)
    {
        perror("realloc");
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
