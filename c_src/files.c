/*
 * files.c
 *
 *  Created on: 20. dets 2018
 *      Author: raigo
 */

#include "files.h"

#include <stdlib.h>
#include <errno.h>

static char* get_legal_act_filename(struct legal_act_id legislation)
{
	struct str_builder result = str_init();
	str_appendc(&result, legislation.type);
	str_appendc(&result, "_");
	str_appendc(&result, legislation.year);
	str_appendc(&result, "_");
	str_appendc(&result, legislation.number);
	if (!str_empty(legislation.version_date))
	{
		str_appendc(&result, "_");
		str_appendc(&result, legislation.version_date);
	}
	return str_content(&result);
}

static char* get_default_filename(
		const char* file_name, int32_t file_number, const char* extension)
{
	struct str_builder result = str_init();
	str_appendc(&result, file_name);
	if (file_number > 0)
	{
		str_appendc(&result, "_");
		str_appendf(&result, "%d", file_number);
	}
	str_appendc(&result, ".");
	str_appendc(&result, extension);
	return str_content(&result);
}

bool get_default_law_file(
		FILE** result, struct legal_act_id legislation, const char* format)
{
	char* lfile_name = get_legal_act_filename(legislation);

	bool success = get_default_file(result, lfile_name, format);
	free(lfile_name);
	return success;
}

bool get_default_file(
		FILE** result, const char* file_name, const char* extension)
{
	char* whole_name = get_default_filename(file_name, 0, extension);
	FILE* output_file = fopen(whole_name, "r");

	int32_t file_number = 1;
	while (output_file != NULL)
	{
		fclose(output_file);
		free(whole_name);
		whole_name = get_default_filename(file_name, file_number, extension);

		output_file = fopen(whole_name, "r");
		file_number += 1;
	}
	if (errno != ENOENT)
	{
		perror(whole_name);
		return false;
	}

	output_file = fopen(whole_name, "w");
	if (output_file == NULL)
	{
		perror(whole_name);
		return false;
	}

	free(whole_name);
	*result = output_file;
	return true;
}
