#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "sections.h"
#include "text_parser.h"
#include "printing.h"
#include "web.h"
#include "command_line.h"

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

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		abort();
	}

	struct run_info args;
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

	char* url = normalize_url(args.url);
	if (url == NULL)
	{
		fprintf(stderr, "Url is malformed.\n");
		return EXIT_FAILURE;
	}

	CURLcode curl_error;
	char curl_error_string[CURL_ERROR_SIZE];
	struct page page = get_web_page(url, &curl_error, curl_error_string);
	free(url);
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
	if (args.dot_format)
	{
		print_dot_from_sections(sections);
	}
	else
	{
		print_pdf_from_sections(sections);
	}

	free_sections(&sections);
	xmlFreeDoc(doc);
	free(page.contents);
	return EXIT_SUCCESS;
}
