#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void* malloc_a(size_t nmemb, size_t memb_size);
void* realloc_a(void* pointer, size_t nmemb, size_t memb_size);
bool str_is_prefix(const char* prefix, const char* string);
int32_t str_tokenize(char* string, char delimit);
