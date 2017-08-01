#pragma once

struct section_references
{
	char** list;
	int32_t list_size;
	int32_t elem_count;
};

struct section_references init_references();
void free_references(struct section_references* array_p);
void add_reference(struct section_references* array_p, char* const element);
void remove_reference(struct section_references* array_p, int32_t index);
struct section_references get_references_from_text(const char* text);
