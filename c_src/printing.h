#pragma once

#include <stdio.h>

#include "sections.h"

int printf_a(const char* template, ...);
int printf_ea(const char* template, ...);
void print_from_sections(FILE* file, struct sections connections,
                         const char* format);
