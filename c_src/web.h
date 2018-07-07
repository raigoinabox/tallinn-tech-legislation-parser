#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include <curl/curl.h>

#include "legislation.h"
#include "strings.h"

bool parse_leg_url(struct leg_id* result, const char* url);
struct string get_leg_api_url(struct leg_id legislation);

bool get_web_page(
        struct string* result, const char* url, char* error_buffer);
