#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include <curl/curl.h>

#include "legislation.h"
#include "strings.h"

struct page
{
    char* contents;
    size_t contents_size;
};

bool parse_url(struct leg_id* result, const char* url);
void legislation_free(struct leg_id* legislation_p);
struct string get_api_url(struct leg_id legislation);

struct page get_web_page(const char* url, CURLcode* const error,
                         char* error_buffer);
