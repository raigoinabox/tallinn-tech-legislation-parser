#pragma once

struct sections;

void print_from_sections(FILE* file, struct sections connections,
                         const char* format);
