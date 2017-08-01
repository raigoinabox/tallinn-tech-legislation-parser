#pragma once

#include <stdlib.h>
#include <stdbool.h>

void* safe_malloc(size_t nmemb, size_t memb_size);
bool str_is_prefix(const char* prefix, const char* string);
char* str_tokenize(char* string, char delimit);
