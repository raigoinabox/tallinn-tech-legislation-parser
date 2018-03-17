#pragma once

#include <stdio.h>

#include "sections.h"
#include "strings.h"

int printeln_a();
int println_a(struct string string);
int printf_a(const char* template, ...);
int printf_ea(const char* template, ...);
void print_graph(FILE* file, struct section_vec connections,
                         const char* format);
