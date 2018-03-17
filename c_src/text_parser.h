#pragma once

#include "vectors.h"

// TODO replace this with string_list
vec_struct(section_references, char*);

struct section_references get_references_from_text(const char* text);
void free_references_deep(struct section_references* array_p);
