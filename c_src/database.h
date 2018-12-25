/*
 * database.h
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <stdbool.h>
#include <libpq-fe.h>

#include "error.h"

struct db_conn {
    PGconn* psql_conn;
};

struct db_result {
    PGresult* psql_result;
};

struct db_params {
    // const char*
    struct vector vector;
};

struct db_conn db_open_conn();
void db_close_conn(struct db_conn conn);
bool begin_transaction(struct db_conn db_conn, struct error*);
bool commit_transaction(struct db_conn db_conn, struct error*);
long db_get_last_insert_row_id(
		struct db_conn db_conn, const char* table, const char* column,
		struct error*);

struct db_params db_params_init();
void db_params_destroy(struct db_params* params);
void db_params_append(struct db_params* params, const char* value);

bool db_exec(struct db_conn db_conn, const char* sql, struct error*);
bool db_exec_params(
		struct db_conn db_conn, const char* sql, struct db_params params,
		struct error*);
bool db_query(
		struct db_result* result_p, struct db_conn db_conn, const char* sql,
		struct error* error);
bool db_query_params(
		struct db_result* db_result, struct db_conn db_conn, const char* sql,
		struct db_params params, struct error* error);
void db_close_result(struct db_result db_result);

int db_nrows(struct db_result db_result);
const char* db_get_value(
        struct db_result db_result,
        int row_number,
        int col_number);

#endif /* DATABASE_H_ */
