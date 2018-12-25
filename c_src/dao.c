/*
 * results_dao.c
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#include <stdbool.h>
#include <assert.h>
#include <dao.h>

#include "printing.h"

void delete_results(struct db_conn db_conn)
{
	struct error error = init_error();
    if (!db_exec(db_conn, "delete from complexity_results where algorithm != 5;", &error)) {
		register_frame(&error);
    	print_error(error);
    	abort();
    }
}

bool delete_sections(struct db_conn db_conn, struct error* error)
{
    if (!db_exec(db_conn, "delete from section_connections;", error))
    {
		register_frame(error);
		print_error(*error);
		abort();

    	return false;
    }
    if (!db_exec(db_conn, "delete from sections;", error))
    {
    	return false;
    }
    return true;
}

int insert_section(
		struct db_conn db_conn, struct section_dto section, struct error* error)
{
	char* year = str_from_long(section.year);
	char* document_id = str_from_long(section.document_id);

    struct db_params params = db_params_init();
	db_params_append(&params, section.country);
	db_params_append(&params, year);
	db_params_append(&params, section.dbu_category);
	db_params_append(&params, document_id);
	db_params_append(&params, section.section_id);
	db_params_append(&params, section.section_text);
	assert(
			db_exec_params(
					db_conn,
					"insert into sections" " (country, year, dbu_category, document_id, section_id, section_text)" " values ($1, $2, $3, $4, $5, $6);",
					params, error));

	free(year);
	free(document_id);
    db_params_destroy(&params);

	return db_get_last_insert_row_id(db_conn, "sections", "row_id", error);
}

void insert_section_connection(
        struct db_conn db_conn,
        struct section_connection_dto connection) {
	char* section_from_id = str_from_long(connection.section_from_id);

    struct db_params params = db_params_init();
	db_params_append(&params, section_from_id);
	db_params_append(&params, connection.section_to);
	assert(
			db_exec_params(db_conn, "insert into section_connections" " (section_from_id, section_to)" " values ($1, $2);", params, NULL));

	free(section_from_id);
    db_params_destroy(&params);
}

void insert_law_document(struct db_conn db_conn, struct law_document document) {
    for (int i = 0; i < vec_length_old(document.sections); i++) {
        struct section section_from = vec_elem_old(document.sections, i);
        for (int j = 0; j < vec_length_old(section_from.references); j++) {
			const char* section_to = vec_elem_old(section_from.references, j);
            struct db_params params = db_params_init();
			db_params_append(&params, section_from.id);
			db_params_append(&params, section_to);
			assert(
					db_exec_params(db_conn, "insert into section_connections (section_from, section_to)" " values ($1, $2);", params, NULL));
            db_params_destroy(&params);
        }
    }
}

bool insert_complx_result(
		struct db_conn db_conn, struct complexity_result_dto result,
		struct error* error)
{
	char* year = str_from_long(result.year);
	char* complexity = str_from_double(result.complexity);
	char* algorithm = str_from_long(result.algorithm);

    struct db_params params = db_params_init();
	db_params_append(&params, result.country);
	db_params_append(&params, year);
	db_params_append(&params, result.dbu_category);
	db_params_append(&params, complexity);
	db_params_append(&params, algorithm);
	bool success = db_exec_params(db_conn, "insert into complexity_results"
			" (country, year, dbu_category, complexity, algorithm)"
			" values ($1, $2, $3, $4, $5);", params, error);

	free(year);
	free(complexity);
	free(algorithm);
    db_params_destroy(&params);

	register_frame(error);
	return success;
}

bool insert_categories(
		struct db_conn db_conn, struct law_category_list law_categories,
		struct error* error)
{
	bool success = db_exec(
			db_conn, "truncate doing_business_laws, doing_business_categories,"
					" doing_business_law_groups", error);
	if (!success)
	{
		register_frame(error);
		print_error(*error);
		abort();
	}

	for (int i = 0; success && i < vec_length_old(law_categories); i++)
    {
        struct dbu_law_category law_group = vec_elem_old(law_categories, i);
        struct db_params params = db_params_init();
		db_params_append(&params, law_group.name);
        struct db_result result = { 0 };
		if (!db_query_params(
				&result, db_conn,
				"insert into doing_business_law_groups (name) values ($1)"
						" returning row_id", params, error))
		{
			print_error(*error);
			abort();
		}
        db_params_destroy(&params);

        const char* law_group_id = db_get_value(result, 0, 0);

		for (int j = 0; success && j < vec_length_old(law_group.dbu_categories);
				j++)
        {
            const char* category = vec_elem_old(law_group.dbu_categories, j);
            params = db_params_init();
			db_params_append(&params, law_group_id);
			db_params_append(&params, category);
			success = db_exec_params(
					db_conn,
					"insert into doing_business_categories(law_group_id, name) "
							"values ($1, $2)", params, error);
            db_params_destroy(&params);
        }

		for (int j = 0; success && j < vec_length(law_group.law_urls); j++)
        {
			char** url = vec_elem(law_group.law_urls, j);
            params = db_params_init();
			db_params_append(&params, law_group_id);
			db_params_append(&params, *url);
			success =
					db_exec_params(
							db_conn,
							"insert into doing_business_laws(law_group_id, country, url) "
									"values ($1, 'GB', $2)", params, error);
            db_params_destroy(&params);
        }
    }

	register_frame(error);
	return success;
}

bool delete_legal_act_sections(
		struct db_conn db_conn, const char* language, struct error* error)
{
	struct db_params params = db_params_init();
	db_params_append(&params, language);
	bool success = db_exec_params(
			db_conn, "delete from legal_act_references"
					" where section_id in ("
					"select section_id from legal_act_sections"
					" where act_id in ("
					"select act_id from legal_acts where language = $1))",
			params, error);
	if (success)
	{
		success = db_exec_params(
				db_conn, "delete from legal_act_section_texts"
						" where section_id in ("
						"select section_id from legal_act_sections"
						" where act_id in ("
						"select act_id from legal_acts where language = $1))",
				params, error);
	}
	if (success)
	{
		success = db_exec_params(
				db_conn, "delete from legal_act_sections"
						" where act_id in ("
						"select act_id from legal_acts where language = $1)",
				params, error);
	}

	db_params_destroy(&params);
	register_frame(error);
	return success;
}


bool delete_legal_acts(
		struct db_conn db_conn, const char* language, struct error* error)
{

	struct db_params params = db_params_init();
	db_params_append(&params, language);

	bool success = db_exec_params(
			db_conn, "delete from legal_acts where language = $1", params,
			error);
	db_params_destroy(&params);

	register_frame(error);
	return success;
}

bool insert_legal_acts(
		struct db_conn db_conn, /* struct legal_act_dto */
		struct vector legal_acts, struct error* error)
{
	bool success = true;
	for (int i = 0; success && i < vec_length(legal_acts); i++)
	{
		struct legal_act_dto* act = vec_elem(legal_acts, i);
		struct db_params params = db_params_init();
		db_params_append(&params, act->title);
		db_params_append(&params, act->url);
		db_params_append(&params, act->language);
		success = db_exec_params(
						db_conn,
						"insert into legal_acts (title, url, language) values ($1, $2, $3) on conflict (url) do nothing",
				params, error);
		db_params_destroy(&params);
	}

	register_frame(error);
	return success;
}

bool get_legal_acts(
		/* struct legal_act_dto */struct vector* result, struct db_conn db_conn,
		const char* language, struct error* error)
{
	struct db_params params = db_params_init();
	db_params_append(&params, language);
	struct db_result db_result =  { 0 };
	bool success = db_query_params(
			&db_result, db_conn,
			"select row_id, title, url from legal_acts where language = $1",
			params, error);
	db_params_destroy(&params);

	if (success)
	{
		*result = vec_init(sizeof(struct legal_act_dto));
		for (int i = 0; i < db_nrows(db_result); i++)
		{
			const char* row_id = db_get_value(db_result, i, 0);
			const char* title = db_get_value(db_result, i, 1);
			const char* url = db_get_value(db_result, i, 2);

			struct legal_act_dto dto =
			{ 0 };
			dto.row_id = strtol(row_id, NULL, 10);
			dto.title = str_copy(title);
			dto.url = str_copy(url);
			vec_append(result, &dto);
		}
	}

	return success;
}

bool insert_legal_act_section(
		struct db_conn db_conn, struct legal_act_section_dto* section,
		struct error* error)
{
	struct db_params params = db_params_init();
	char* row_id = str_from_long(section->act_id);
	db_params_append(&params, row_id);
	db_params_append(&params, section->section_number);
	struct db_result db_result = { 0 };
	bool success = db_query_params(
			&db_result, db_conn,
			"insert into legal_act_sections (act_id, section_number)"
					" values ($1, $2) returning row_id", params,
					error);

	if (success) {
		assert(db_nrows(db_result) == 1);
		section->row_id = strtol(db_get_value(db_result, 0, 0), NULL, 10);
	}

	register_frame(error);
	db_params_destroy(&params);
	free(row_id);
	return success;
}

bool insert_legal_act_section_text(
		struct db_conn db_conn, struct legal_act_section_text_dto section,
		struct error* error)
{
	struct db_params params = db_params_init();
	char* row_id = str_from_long(section.section_id);
	db_params_append(&params, row_id);
	db_params_append(&params, section.section_text);
	bool success = db_exec_params(
			db_conn,
			"insert into legal_act_section_texts (section_id, section_text)"
					" values ($1, $2)", params, error);

	db_params_destroy(&params);
	free(row_id);
	register_frame(error);
	return success;
}

bool insert_legal_act_reference(
		struct db_conn db_conn, struct legal_act_reference_dto reference,
		struct error* error)
{
	struct db_params params = db_params_init();
	char* section_id = str_from_long(reference.section_id);
	db_params_append(&params, section_id);
	db_params_append(&params, reference.section_to);
	bool success =
			db_exec_params(
					db_conn,
					"insert into legal_act_references (section_id, reference) values ($1, $2)",
					params, error);
	db_params_destroy(&params);
	free(section_id);

	register_frame(error);
	return success;
}

bool insert_legal_act_references(
		struct db_conn db_conn, long row_id,
		struct section_references references, struct error* error)
{
	bool success = true;
	for (int j = 0;
			success && j < vec_length_old(references); j++)
	{
		const char* reference = vec_elem_old(references, j);
		struct legal_act_reference_dto reference_dto =
		{ 0 };
		reference_dto.section_id = row_id;
		reference_dto.section_to = reference;
		success = insert_legal_act_reference(db_conn, reference_dto, error);
	}

	return success;
}
