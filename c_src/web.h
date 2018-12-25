#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include <curl/curl.h>

#include "strings.h"
#include "legal_act.h"
#include "error.h"

bool parse_leg_url(struct legal_act_id* result, const char* url);
char* get_leg_api_url(struct legal_act_id legislation);

bool get_web_page(struct str_builder* result, const char* url, struct error*);
