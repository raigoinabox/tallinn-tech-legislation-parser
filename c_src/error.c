/*
 * error.c
 *
 *  Created on: 20. aug 2018
 *      Author: raigo
 */

#include "error.h"

#include <stdlib.h>

struct error init_error()
{
	struct error error = { 0 };
	error.frames = vec_init(sizeof(struct error_frame));
	return error;
}

void _register_error(
		struct error* error, const char* message, const char* file,
		int line_number)
{
	if (error->message == NULL)
	{
		error->message = message;
		error->file_name = file;
		error->line_number = line_number;
	}
}

void _register_frame(
		struct error* error, const char* file_name, const char* func_name)
{
	if (error->message != NULL)
	{
		struct error_frame line = { 0 };
		line.file_name = file_name;
		line.func_name = func_name;
		vec_append(&error->frames, &line);
	}
}

void print_error(struct error error)
{
	if (error.message != NULL)
	{
		fprintf(
		stderr, "%s: %d: %s\n", error.file_name, error.line_number, error.message);
	}
	else
	{
		fprintf(stderr, "Unknown error\n");
	}

	for (int i = 0; i < vec_length(error.frames); i++)
	{
		struct error_frame* line = vec_elem(error.frames, i);
		fprintf(
		stderr, "\t%s: %s\n", line->file_name, line->func_name);
	}
}
