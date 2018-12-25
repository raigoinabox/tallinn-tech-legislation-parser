#pragma once

#include "vectors.h"
#include "strings.h"

vec_struct(section_references, char*);

struct section_references get_references_from_text(struct str_builder text);
void free_references_deep(struct section_references* array_p);
