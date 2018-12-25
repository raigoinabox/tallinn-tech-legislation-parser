/*
 * estonia.c
 *
 *  Created on: 19. dets 2018
 *      Author: raigo
 */

#include "estonia.h"

#include <string.h>

#include "dao.h"
#include "web.h"
#include "printing.h"
#include "maps.h"
#include "files.h"

static int str_comp(void* void1, void* void2)
{
	const char** str1 = void1;
	const char** str2 = void2;
	return strcmp(*str1, *str2);
}

bool estonia(
		const char* prog, const char* command, struct arp_parser argp,
		struct error* error)
{
	(void) prog;
	(void) command;
	(void) argp;
	struct db_conn db_conn = db_open_conn();

	bool success = delete_legal_act_sections(db_conn, EST_LANG, error);
	struct vector legal_acts = vec_null;
	if (success)
	{
		success = get_legal_acts(&legal_acts, db_conn, EST_LANG, error);
	}
	for (int i = 0; success && i < vec_length(legal_acts); i++)
	{
		struct legal_act_dto* legal_act = vec_elem(legal_acts, i);
		struct str_builder url_builder = str_init();
		str_appendc(&url_builder, "http://localhost/riigiteataja/test56uhes.php"
				"?url=");
		str_appendc(&url_builder, legal_act->url);
		struct str_builder page_builder = str_init();
		success = get_web_page(&page_builder, str_content(&url_builder), error);
		str_builder_destroy(&url_builder);

		char* page = NULL;
		if (success)
		{
			page = str_content(&page_builder);
			struct vector lines = str_split(page, "<br />");
			bool begin = false;
			struct map graph = map_init(
					sizeof(char*), sizeof(struct vector), str_comp);
			for (int i = 0; i < vec_length(lines); i++)
			{
				char** line_p = vec_elem(lines, i);
				char* line = str_trim(*line_p);
				if (begin)
				{
					if (str_equal(line, ""))
					{
						break;
					}

					struct vector nodes = str_split(line, ";");
					if (vec_length(nodes) == 2)
					{
						struct vector node_list;
						if (!map_get(&node_list, graph, vec_elem(nodes, 0)))
						{
							node_list = vec_init(sizeof(char*));
						}
						vec_append(&node_list, vec_elem(nodes, 1));
						map_set(&graph, vec_elem(nodes, 0), &node_list);
					}
				}
				else if (str_equal(line, "OUT1 tavaline valjund"))
				{
					begin = true;
				}
			}

			struct map_iterator graph_iter = map_iterator(graph);
			struct map_entry entry;
			while (success && map_iterator_next(&entry, &graph_iter))
			{
				const char** node = entry.key;
				struct vector* node_list = entry.value;

				struct legal_act_section_dto section_dto = { 0 };
				section_dto.act_id = legal_act->row_id;
				section_dto.section_number = *node;
				success = insert_legal_act_section(
						db_conn, &section_dto, error);

				for (int i = 0; success && i < vec_length(*node_list); i++)
				{
					const char** reference = vec_elem(*node_list, i);
					struct legal_act_reference_dto ref_dto = { 0 };
					ref_dto.section_id = section_dto.row_id;
					ref_dto.section_to = *reference;
					success = insert_legal_act_reference(
							db_conn, ref_dto, error);
				}
			}

			FILE* file;
			success = get_default_file(&file, "est", "pdf");
			if (success)
			{
				print_graph(file, graph, "pdf", "fdp");
			}

			map_free(&graph);
		}

		free(page);
	}
	vec_destroy(&legal_acts);
	db_close_conn(db_conn);
	return success;
}
