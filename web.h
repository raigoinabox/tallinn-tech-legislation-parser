#pragma once

#include <stdlib.h>

#include <curl/curl.h>

struct page
{
	char* contents;
	size_t contents_size;
};

char* normalize_url(const char* url);
struct page get_web_page(const char* url, CURLcode* const error,
                         char* error_buffer);
