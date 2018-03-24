#pragma once

#include <stdint.h>

#include "legislation.h"
#include "vectors.h"

vec_struct(law_vec, struct leg_id);
vec_struct(cstring_vec, const char*);

struct dbu_law_category
{
    const char* name;
    struct cstring_vec dbu_categories;
    struct law_vec laws;
};

vec_struct(law_category_list, struct dbu_law_category);

void dbu_init();
struct law_category_list get_english_law_categories();
