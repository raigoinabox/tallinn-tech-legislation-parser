#pragma once

#include "vectors.h"

vec_struct(section_references, struct string);

struct section_references get_references_from_text(struct string text);
void free_references_deep(struct section_references* array_p);
