#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

void* malloc_a(size_t nmemb, size_t memb_size);
bool str_is_prefix(const char* prefix, const char* string);
int32_t str_tokenize(char* string, char delimit);
