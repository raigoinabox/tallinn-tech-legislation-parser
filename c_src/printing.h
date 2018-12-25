#pragma once

#include <stdio.h>

#include "sections.h"
#include "maps.h"

int printeln_a();
int println_a(const char* string);
int printf_a(const char* template, ...);
int printf_ea(const char* template, ...);
void print_section_graph(
		FILE* file, struct section_vec connections, const char* format,
		const char* layout);
void print_graph(
		FILE* file, struct map /* char*, struct vector (char*) */connections,
		const char* format, const char* layout);
