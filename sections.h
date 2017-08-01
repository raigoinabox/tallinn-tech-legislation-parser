#pragma once

#include <stdint.h>

#include "text_parser.h"

struct section
{
	char* id;
	struct section_references references;
};

struct sections
{
	struct section* list;
	int32_t list_size;
	int32_t elem_count;
};

struct sections init_section_darray();
void free_section(struct section* section);
void free_sections(struct sections* array_p);
void free_sections_shallow(struct sections* sections_p);

void section_da_add_elem(struct sections* array_p, struct section element);
void remove_section(struct sections* array_p, int32_t index);
void set_section(struct sections sections, int32_t index,
                 struct section section);
int32_t get_sections_count(struct sections sections);
struct section get_section(struct sections sections, int32_t index);

int32_t get_references_count(struct section section);
char* get_reference(struct section section, int32_t index);
_Bool has_section_references(struct section section);

void remove_foreign_sections(struct sections sections);
void remove_single_sections(struct sections* sections_p);
