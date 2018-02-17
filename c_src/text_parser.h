#pragma once

#include "vectors.h"

VECTOR_DECLARE(, char*, section_references)

struct section_references get_references_from_text(const char* text);
void free_references_deep(struct section_references* array_p);
