#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <string.h>
#include <errno.h>

#include <libxml/parser.h>

#include "sections.h"
#include "text_parser.h"
#include "printing.h"
#include "web.h"
#include "command_line.h"
#include "util.h"
#include "safe_string.h"

static xmlChar* filter_triple_dots(xmlChar* text)
{
	xmlChar* filtered_text = malloc(sizeof(xmlChar) * (xmlStrlen(text) + 1));
	if (filtered_text == NULL)
	{
		abort();
	}

	xmlChar* filtered_text_p = filtered_text;
	while (true)
	{
		if (*text == '.' && *(text + 1) == '.' && *(text + 2) == '.')
		{
			text += 3;
			continue;
		}

		*filtered_text_p = *text;
		if (*text == '\0')
		{
			break;
		}

		filtered_text_p++;
		text++;
	}

	return filtered_text;
}

static struct sections get_sections_from_doc(xmlNodePtr node)
{
	struct sections result = init_section_darray();

	xmlChar* id = xmlGetProp(node, (xmlChar*)"id");
	size_t prefix_length = strlen("section-");
	if (id != NULL && xmlStrncmp(id, (xmlChar*)"section-", prefix_length) == 0)
	{
		xmlChar* section_number = malloc((xmlStrlen(id) + 1 - prefix_length) * sizeof(
		                                     xmlChar));
		if (section_number == NULL)
		{
			abort();
		}
		section_number[0] = '\0';
		section_number = xmlStrcat(section_number, id + prefix_length);

		xmlChar* node_text_w_three_dots = xmlNodeGetContent(node);
		xmlChar* node_text = filter_triple_dots(node_text_w_three_dots);
		xmlFree(node_text_w_three_dots);

		struct section_references references = get_references_from_text((
		        char*)node_text);
		xmlFree(node_text);

		struct section section =
		{
			.id = (char*)section_number,
			.references = references
		};
		section_da_add_elem(&result, section);
	}
	else
	{
		for (xmlNodePtr child = node->children; child != NULL; child = child->next)
		{
			struct sections child_result = get_sections_from_doc(child);
			for (int32_t i = 0; i < child_result.elem_count; i++)
			{
				struct section child_section = child_result.list[i];
				section_da_add_elem(&result, child_section);
			}
			free_sections_shallow(&child_result);
		}
	}

	xmlFree(id);
	return result;
}

struct run_info
{
	struct string_d url;
	FILE* output_file;
	const char* format;
	bool debug;
};

static bool get_default_file(FILE** result, struct legislation legislation, const char* format)
{
	struct string_d file_name = strd_mallocn(strd_length(legislation.type) + strd_length(
				legislation.year) + strd_length(legislation.number) + strlen(format) + 10);

	strd_appends(&file_name, legislation.type);
	strd_append(&file_name, "_");
	strd_appends(&file_name, legislation.year);
	strd_append(&file_name, "_");
	strd_appends(&file_name, legislation.number);
	strd_append(&file_name, ".");
	strd_append(&file_name, format);

	int32_t file_number = 1;
	FILE* output_file = fopen(strd_content(file_name), "r");
	while (output_file != NULL)
	{
		fclose(output_file);

		char file_number_str[100];
		sprintf(file_number_str, "%d", file_number);
		strd_clear(&file_name);
		strd_appends(&file_name, legislation.type);
		strd_append(&file_name, "_");
		strd_appends(&file_name, legislation.year);
		strd_append(&file_name, "_");
		strd_appends(&file_name, legislation.number);
		strd_append(&file_name, "-");
		strd_append(&file_name, file_number_str);
		strd_append(&file_name, ".");
		strd_append(&file_name, format);

		output_file = fopen(strd_content(file_name), "r");
		file_number += 1;
	}
	if (errno != ENOENT)
	{
		perror("Unknown error while searching for the default file name ");
		return false;
	}

	output_file = fopen(strd_content(file_name), "w");
	if (output_file == NULL)
	{
		perror(strd_content(file_name));
		return false;
	}

	strd_free(&file_name);
	*result = output_file;
	return true;
}

static bool process_args(struct run_info* result, struct print_args args)
{
	struct run_info run_info;

	run_info.debug = args.debug;

	struct legislation legislation;
	if (!parse_url(&legislation, args.url))
	{
		fprintf(stderr, "Url is malformed.\n");
		return false;
	}

	run_info.format = args.format;
	if (run_info.format == NULL || strcmp(run_info.format, "") == 0)
	{
		run_info.format = "pdf";
	}

	if (args.output_file_name == NULL)
	{
		if (!get_default_file(&run_info.output_file, legislation, run_info.format))
		{
			return false;
		}

	}
	else if (strcmp(args.output_file_name, "-") == 0)
	{
		run_info.output_file = stdout;
	}
	else
	{
		run_info.output_file = fopen(args.output_file_name, "w");
		if (run_info.output_file == NULL)
		{
			perror(args.output_file_name);
			return false;
		}
	}

	run_info.url = get_api_url(legislation);

	*result = run_info;
	return true;
}

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		abort();
	}

	struct print_args args;
	bool success = parse_args(&args, argc, argv);
	if (!success)
	{
		return EXIT_FAILURE;
	}

	if (args.print_help)
	{
		print_help(argv[0]);
		return EXIT_SUCCESS;
	}

	struct run_info run_info;
	if (!process_args(&run_info, args))
	{
		return EXIT_FAILURE;
	}

	CURLcode curl_error;
	char curl_error_string[CURL_ERROR_SIZE];
	struct page page = get_web_page(strd_content(run_info.url), &curl_error, curl_error_string);
	strd_free(&run_info.url);
	if (curl_error != CURLE_OK)
	{
		assert(strlen(curl_error_string) > 0);
		fprintf(stderr, "%s\n", curl_error_string);
		free(page.contents);
		return EXIT_FAILURE;
	}

	xmlDocPtr doc = xmlParseMemory(page.contents, page.contents_size);
	struct sections sections = get_sections_from_doc(xmlDocGetRootElement(doc));
	if (!args.debug)
	{
		remove_foreign_sections(sections);
	}
	remove_single_sections(&sections);
	print_from_sections(run_info.output_file, sections, run_info.format);

	free_sections(&sections);
	xmlFreeDoc(doc);
	free(page.contents);
	return EXIT_SUCCESS;
}
