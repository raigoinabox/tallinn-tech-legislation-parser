/*
 * leg_feed.c
 *
 *  Created on: 11. okt 2018
 *      Author: raigo
 */

#include <dao.h>
#include <database.h>
#include <mrss.h>
#include <printing.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <vectors.h>
#include <web.h>
#include "leg_feed.h"

static bool find_next(char** result, mrss_tag_t * tag)
{
	mrss_attribute_t *attribute;
	char* next_link = NULL;

	while (tag)
	{
		if (strcmp(tag->name, "link") == 0)
		{
			bool is_next = false;
			for (attribute = tag->attributes; attribute;
					attribute = attribute->next)
			{
				if (is_next && strcmp(attribute->name, "href") == 0)
				{
					next_link = str_copy(attribute->value);
				}
				else if (strcmp(attribute->name, "rel") == 0
						&& strcmp(attribute->value, "next") == 0)
				{
					is_next = true;
				}
			}
		}

		tag = tag->next;
	}

	if (next_link != NULL)
	{
		*result = next_link;
	}
	return next_link != NULL;
}

bool get_laws(/* struct legal_act_dto */
		struct vector* result, struct error* error)
{
	/* struct legal_act_dto */
	struct vector legal_acts = vec_init(sizeof(struct legal_act_dto));
	char* next = str_copy(
			"http://www.legislation.gov.uk/all/data.feed");
	bool next_found = true;
	bool success = true;
	while (success && next_found)
	{
		struct str_builder page = { 0 };
		success = get_web_page(&page, next, error);
		mrss_t *data = NULL;
		if (!success)
		{
			return false;
		}

		if (success)
		{
			mrss_error_t ret = mrss_parse_buffer(
					str_content(&page), str_length(page), &data);
			success = !ret;
			if (!success)
			{
				register_error(error, mrss_strerror(ret));
			}
		}


		if (success)
		{
			free(next);
			next = NULL;
			next_found = false;
			if (data->other_tags)
			{
				next_found = find_next(&next, data->other_tags);
			}

			mrss_item_t* item = data->item;
			while (item)
			{
				struct legal_act_dto legal_act =
				{ 0 };
				if (item->title != NULL)
				{
					legal_act.title = str_copy(item->title);
				}
				legal_act.url = str_copy(item->link);
				legal_act.language = ENG_LANG;
				vec_append(&legal_acts, &legal_act);

				item = item->next;
			}
		}

		mrss_free(data);
		str_builder_destroy(&page);
	}

	if (success)
	{
		*result = legal_acts;
	}
	return success;
}

bool search_laws(
		const char* prog, const char* command, struct arp_parser parser,
		struct error* error)
{
	(void) prog;
	(void) command;
	(void) parser;

	/* struct legal_act_dto */
	struct vector legal_acts;
	bool success = get_laws(&legal_acts, error);
	struct db_conn db_conn = { 0 };
	if (success) {
		db_conn = db_open_conn();
		success = delete_legal_acts(db_conn, ENG_LANG, error);
	}
	if (success)
	{
		success = insert_legal_acts(db_conn, legal_acts, error);
	}

	db_close_conn(db_conn);
	vec_destroy(&legal_acts);
	register_frame(error);
	return success;
}
