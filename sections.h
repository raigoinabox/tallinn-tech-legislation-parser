#pragma once

#include <stdint.h>

#include "text_parser.h"
#include "vectors.h"

struct section
{
	char* id;
	struct section_references references;
};

VECTOR_DECLARE(, struct section, sections)

void sections_free_deep(struct sections* array_p);

int32_t get_references_count(struct section section);
char* get_reference(struct section section, int32_t index);
_Bool has_section_references(struct section section);

void remove_foreign_sections(struct sections sections);
void remove_single_sections(struct sections* sections_p);
