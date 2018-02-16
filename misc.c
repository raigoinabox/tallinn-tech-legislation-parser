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
#include <libxml/xmlstring.h>
#include <stdarg.h>
#include <stddef.h>
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
			state.section_text = str_init();
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

static void sax_warning(void *user_data, const char *msg, ...) {
	(void) user_data;
    va_list args;

    va_start(args, msg);
    fprintf_a(stderr, "sax_warning: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static void sax_error(void *user_data, const char *msg, ...) {
	(void) user_data;
	(void) msg;
//    va_list args;

//    va_start(args, msg);
//    fprintf_a(stderr, "sax_error: ");
//    vfprintf(stderr, msg, args);
//    va_end(args);
}

static void sax_fatal_error(void *user_data, const char *msg, ...) {
	(void) user_data;
    va_list args;

    va_start(args, msg);
    fprintf_a(stderr, "sax_fatal_error: ");
    vfprintf(stderr, msg, args);
    va_end(args);
}

static xmlSAXHandler sax = {
		.startElement = sax_start_element,
		.endElement = sax_end_element,
		.characters = sax_characters,
		.warning = sax_warning,
		.error = sax_error,
		.fatalError = sax_fatal_error
};

static bool get_sections_from_page(struct sections* result, struct page page)
{
	struct SAX_state sax_state = {
			.result = sections_init(),
			.section_depth = 0
	};

	if (xmlSAXUserParseMemory(&sax, &sax_state, page.contents,
			page.contents_size) != 0) {
		sections_free(&sax_state.result);
		return false;
	}
	*result = sax_state.result;
	return true;
}

bool get_sections_from_legislation(struct sections* result,
		struct leg_id legislation)
{
	struct string url = get_api_url(legislation);

	CURLcode curl_error;
	char curl_error_string[CURL_ERROR_SIZE];
	struct page page = get_web_page(str_content(url), &curl_error,
			curl_error_string);
	bool success = true;
	if (curl_error != CURLE_OK) {
		success = false;
		assert(strlen(curl_error_string) > 0);
		fprintf(stderr, "%s\n", curl_error_string);
		goto end;
	}

	success = get_sections_from_page(result, page);

end:
	free(page.contents);
	str_free(&url);
	return success;
}
