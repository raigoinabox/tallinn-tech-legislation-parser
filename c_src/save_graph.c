/*
 * calc_graph.c
 *
 *  Created on: 28. sept 2018
 *      Author: raigo
 */

#include "save_graph.h"

#include "dao.h"
#include "legal_act.h"
#include "web.h"

bool save_graph(
		const char* program_name, const char* command_name, struct arp_parser arg_parser,
		struct error* error)
{
	(void) program_name;
	(void) command_name;
	(void) arg_parser;

	// struct legal_act_dto
	struct vector legal_acts = { 0 };
	struct db_conn db_conn = db_open_conn();
	bool success = delete_legal_act_sections(db_conn, ENG_LANG, error);
	if (success) {
		success = get_legal_acts(&legal_acts, db_conn, ENG_LANG, error);
	}

	for (int i = 0; success && i < vec_length(legal_acts); i++) {
		struct legal_act_dto* legal_act = vec_elem(legal_acts, i);
		struct legal_act_id leg_id = { 0 };
		success = parse_leg_url(&leg_id, legal_act->url);

		struct section_vec section_vec = { 0 };
		bool section_success = success;
		if (success) {
			section_success = get_sections_from_legislation(
					&section_vec, leg_id, error);
		}

		for (int i = 0;
				success && section_success && i < vec_length_old(section_vec);
				i++)
		{
			struct section section = vec_elem_old(section_vec, i);
			struct legal_act_section_dto section_dto = { 0 };
			section_dto.act_id = legal_act->row_id;
			section_dto.section_number = section.id;
			success = insert_legal_act_section(db_conn, &section_dto, error);
			if (success)
			{
				struct legal_act_section_text_dto text = { 0 };
				text.section_id = section_dto.row_id;
				text.section_text = section.text;
				success = insert_legal_act_section_text(db_conn, text, error);
			}
			if (success)
			{
				success = insert_legal_act_references(
						db_conn, section_dto.row_id, section.references, error);
			}
		}

		sections_free_deep(&section_vec);
	}

	register_frame(error);
	vec_destroy(&legal_acts);
	db_close_conn(db_conn);
	return success;
}

