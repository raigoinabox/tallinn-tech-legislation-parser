#pragma once

#include <stdint.h>

#include "legislation.h"
#include "vectors.h"

vec_struct(law_list, struct leg_id);
vec_struct(string_list, const char*);

struct dbu_law_category {
	const char* name;
	struct string_list dbu_categories;
	struct law_list laws;
};

vec_struct(law_category_list, struct dbu_law_category);

void dbu_init();
struct law_category_list get_english_law_categories();
