#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include <curl/curl.h>

#include "safe_string.h"

struct page
{
	char* contents;
	size_t contents_size;
};

struct legislation
{
	struct string_d type;
	struct string_d year;
	struct string_d number;
};

bool parse_url(struct legislation* result, const char* url);
void legislation_free(struct legislation* legislation_p);
struct string_d get_api_url(struct legislation legislation);

struct page get_web_page(const char* url, CURLcode* const error,
                         char* error_buffer);
