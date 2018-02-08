/*
 * misc.c
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#include "misc.h"

#include <assert.h>
#include <curl/curl.h>
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safe_string.h"
#include "text_parser.h"
#include "util.h"
#include "web.h"



static xmlChar* filter_triple_dots(const xmlChar* text) {
	xmlChar* filtered_text = malloc(sizeof(xmlChar) * (xmlStrlen(text) + 1));
	if (filtered_text == NULL) {
		abort();
	}

	xmlChar* filtered_text_p = filtered_text;
	while (true) {
		if (*text == '.' && *(text + 1) == '.' && *(text + 2) == '.') {
			text += 3;
			continue;
		}

		*filtered_text_p = *text;
		if (*text == '\0') {
			break;
		}

		filtered_text_p++;
		text++;
	}

	return filtered_text;
}

static const char* char_from_xml(const xmlChar* text)
{
	return (const char*) text;
}

static const xmlChar* xml_from_char(const char* text) {
	return (const xmlChar*) text;
}

struct SAX_state {
	struct sections result;
	int section_depth;
	const xmlChar* section_number;
	struct string section_text;
};

static void sax_start_document(void* user_data)
{
	struct SAX_state* state_p = user_data;
	state_p->result = sections_init();
	state_p->section_depth = 0;
}

static void sax_start_element(void* user_data, const xmlChar* name,
		const xmlChar** attributes)
{
	(void) name;

	struct SAX_state* state_p = user_data;
	struct SAX_state state = *state_p;
	if (0 < state.section_depth) {
		state.section_depth += 1;
		goto end;
	}

	if (attributes == NULL) {
		goto end;
	}

	for (int i = 0; attributes[i] != NULL; i += 2) {
		const size_t prefix_length = strlen("section-");
		if (xmlStrcmp(attributes[i], (xmlChar*) "id") == 0
				&& xmlStrncmp(attributes[i + 1], (xmlChar*) "section-",
					prefix_length) == 0)
		{
			const xmlChar* id = attributes[i + 1];
			xmlChar* section_number = malloc_a((xmlStrlen(id) + 1 - prefix_length),
					sizeof(xmlChar));
			section_number[0] = '\0';
			state.section_number = xmlStrcat(section_number, id + prefix_length);
			state.section_depth = 1;
			state.section_text = string_init();
			goto end;
		}
	}

end:
	*state_p = state;
}

static void sax_end_element(void* user_data, const xmlChar* name)
{
	(void) name;
	struct SAX_state* state_p = user_data;
	struct SAX_state state = *state_p;
	if (1 < state.section_depth) {
		state.section_depth -= 1;
	} else if (0 < state.section_depth) {
		state.section_depth -= 1;
		struct string node_text_w_three_dots = state.section_text;
		xmlChar* node_text = filter_triple_dots(
				xml_from_char(str_content(node_text_w_three_dots)));

		struct section_references references = get_references_from_text(
				char_from_xml(node_text));
		xmlFree(node_text);

		struct section section = { .id = (char*) state.section_number, .references =
				references };
		sections_append(&state.result, section);
		str_free(&state.section_text);
	}

	*state_p = state;
}

static void sax_characters(void* user_data, const xmlChar* text, int len)
{
	struct SAX_state* state_p = user_data;
	struct SAX_state state = *state_p;
	if (0 < state.section_depth) {
		str_appendn(&state.section_text, char_from_xml(text), len);
	}
	*state_p = state;
}

static xmlSAXHandler sax = {
		.startDocument = sax_start_document,
		.startElement = sax_start_element,
		.endElement = sax_end_element,
		.characters = sax_characters
};

static bool process_node(struct section* result, xmlTextReaderPtr xmlReader)
{
	xmlChar* id = xmlTextReaderGetAttribute(xmlReader, (xmlChar*) "id");
	const size_t prefix_length = strlen("section-");
	if (id == NULL
			|| xmlStrncmp(id, (xmlChar*) "section-", prefix_length) != 0) {
		return false;
	}

	xmlChar* section_number = malloc_a((xmlStrlen(id) + 1 - prefix_length),
			sizeof(xmlChar));
	section_number[0] = '\0';
	section_number = xmlStrcat(section_number, id + prefix_length);

	xmlNodePtr node = xmlTextReaderExpand(xmlReader);
	xmlChar* node_text_w_three_dots = xmlNodeGetContent(node);
	xmlChar* node_text = filter_triple_dots(node_text_w_three_dots);
	xmlFree(node_text_w_three_dots);

	struct section_references references = get_references_from_text(
			(char*) node_text);
	xmlFree(node_text);

	struct section section = { .id = (char*) section_number, .references =
			references };
	*result = section;
	return true;
}

static struct sections get_sections_from_page(struct page page,
		struct string url) {
	struct SAX_state sax_state;
	if (false) {
		if (xmlSAXUserParseMemory(&sax, &sax_state, page.contents, page.contents_size) != 0) {
			abort();
		}
		return sax_state.result;
	} else {
		struct sections sections = sections_init();
		xmlTextReaderPtr xmlReader = xmlReaderForMemory(page.contents,
				page.contents_size, str_content(url), NULL, 0);
		if (xmlReader == NULL) {
			abort();
		}
		int result = xmlTextReaderRead(xmlReader);
		while (result == 1) {
			struct section section;
			if (process_node(&section, xmlReader)) {
				sections_append(&sections, section);
				result = xmlTextReaderNext(xmlReader);
			} else {
				result = xmlTextReaderRead(xmlReader);
			}
		}
		xmlFreeTextReader(xmlReader);
		if (result != 0) {
			abort();
		}
		return sections;
	}
}

bool get_sections_from_legislation(struct sections* result,
		struct leg_id legislation)
{
	struct string url = get_api_url(legislation);

	CURLcode curl_error;
	char curl_error_string[CURL_ERROR_SIZE];
	struct page page = get_web_page(str_content(url), &curl_error,
			curl_error_string);
	if (curl_error != CURLE_OK) {
		assert(strlen(curl_error_string) > 0);
		fprintf(stderr, "%s\n", curl_error_string);
		free(page.contents);
		str_free(&url);
		return false;
	}

	*result = get_sections_from_page(page, url);

	free(page.contents);
	str_free(&url);
	return true;
}
