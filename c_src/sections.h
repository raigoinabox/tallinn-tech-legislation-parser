#pragma once

#include <stdint.h>

#include "text_parser.h"
#include "vectors.h"

struct section
{
    char* id;
    struct section_references references;
};

vec_struct(section_vec, struct section);

void sections_free_deep(struct section_vec* array_p);

int32_t get_references_count(struct section section);
char* get_reference(struct section section, int32_t index);
_Bool has_section_references(struct section section);

void remove_foreign_sections(struct section_vec sections);
void remove_single_sections(struct section_vec* sections_p);
