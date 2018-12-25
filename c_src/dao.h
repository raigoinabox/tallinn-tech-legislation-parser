/*
 * results_dao.h
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#ifndef DAO_H_
#define DAO_H_

#include <stdint.h>

#include "sections.h"
#include "database.h"

#define EST_LANG "est"
#define ENG_LANG "eng"

struct section_dto {
	const char* country;
    int year;
	const char* dbu_category;
    int document_id;
	const char* section_id;
	const char* section_text;
};

struct section_connection_dto {
    int section_from_id;
	const char* section_to;
};

struct complexity_result_dto {
	const char* country;
    int year;
	const char* dbu_category;
    double complexity;
    int algorithm;
};

struct legal_act_dto {
	long row_id;
	char* title;
	char* url;
	const char* language;
};

struct legal_act_section_dto {
	long row_id;
	long act_id;
	const char* section_number;
};

struct legal_act_section_text_dto
{
	long section_id;
	const char* section_text;
};

struct legal_act_reference_dto {
	long section_id;
	const char* section_to;
};

void delete_results(struct db_conn);
bool delete_sections(struct db_conn, struct error*);
int insert_section(struct db_conn, struct section_dto, struct error*);
void insert_section_connection(
        struct db_conn db_conn,
        struct section_connection_dto section_connection);
void insert_law_document(struct db_conn db_conn, struct law_document document);
bool insert_complx_result(
		struct db_conn db_conn, struct complexity_result_dto result,
		struct error*);
bool insert_categories(
		struct db_conn db_conn, struct law_category_list law_categories,
		struct error* error);

bool delete_legal_act_sections(
		struct db_conn, const char* language, struct error*);
bool delete_legal_acts(
		struct db_conn db_conn, const char* language, struct error* error);
bool insert_legal_acts(
		struct db_conn db_conn, /* struct legal_act_dto */
		struct vector legal_acts, struct error* error);

bool get_legal_acts(
		/* struct legal_act_dto */struct vector* result, struct db_conn db_conn,
		const char* language, struct error* error);
bool insert_legal_act_section(
		struct db_conn db_conn, struct legal_act_section_dto* section,
		struct error*);
bool insert_legal_act_section_text(
		struct db_conn db_conn, struct legal_act_section_text_dto section,
		struct error* error);
bool insert_legal_act_reference(
		struct db_conn db_conn, struct legal_act_reference_dto reference,
		struct error*);
bool insert_legal_act_references(
		struct db_conn db_conn, long row_id,
		struct section_references references, struct error* error);


#endif /* DAO_H_ */
