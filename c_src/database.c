/*
 * database.c
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#include <assert.h>
#include <database.h>
#include <stdio.h>
#include <stdlib.h>
#include <vectors.h>

#include "strings.h"

struct db_conn db_open_conn()
{
    struct db_conn db_conn;
    db_conn.psql_conn = PQconnectdb("dbname = tallinn_tech_legislation_parser");
    if (PQstatus(db_conn.psql_conn) != CONNECTION_OK) {
    	fprintf(stderr, "connection to db failed: %s\n", PQerrorMessage(db_conn.psql_conn));
    	assert(false);
        abort();
    }

    return db_conn;
}

void db_close_conn(struct db_conn db_conn) {
    PQfinish(db_conn.psql_conn);
}

bool begin_transaction(struct db_conn db_conn, struct error* error)
{
	bool success = db_exec(db_conn, "begin transaction", error);
	register_frame(error);
	return success;
}

bool commit_transaction(struct db_conn db_conn, struct error* error)
{
	bool success = db_exec(db_conn, "commit transaction", error);
	register_frame(error);
	return success;
}

long db_get_last_insert_row_id(
		struct db_conn db_conn, const char* table, const char* column,
		struct error* error)
{

    struct db_params params = db_params_init();
	db_params_append(&params, table);
	db_params_append(&params, column);
	struct db_result db_result = { 0 };
	if (!db_query_params(
			&db_result, db_conn,
			"select currval(pg_get_serial_sequence($1, $2))", params, error))
	{
		print_error(*error);
		abort();
	}
    db_params_destroy(&params);

    long int row_id = strtol(db_get_value(db_result, 0, 0), NULL, 10);
    db_close_result(db_result);
    return row_id;
}

struct db_params db_params_init()
{
    struct db_params params = { 0 };
    params.vector = vec_init(sizeof(const char*));
    return params;
}

void db_params_destroy(struct db_params* params)
{
    vec_destroy(&params->vector);
}

void db_params_append(struct db_params* params, const char* value)
{
	vec_append(&params->vector, &value);
}

bool db_exec(struct db_conn db_conn, const char* sql, struct error* error_p)
{
	struct db_result result = { 0 };
    result.psql_result = PQexec(db_conn.psql_conn, sql);
    int result_status = PQresultStatus(result.psql_result);
    assert(result_status != PGRES_TUPLES_OK);
	bool success = true;
    if (result_status != PGRES_COMMAND_OK) {
		register_error(error_p, str_copy(PQerrorMessage(db_conn.psql_conn)));
		success = false;
    }

    db_close_result(result);
	return success;
}

bool db_exec_params(
		struct db_conn db_conn, const char* sql, struct db_params params,
		struct error* error)
{
    struct db_result result = { 0 };
	result.psql_result = PQexecParams(
			db_conn.psql_conn, sql, vec_length(params.vector),
			NULL, vec_content(params.vector),
			NULL,
			NULL, 0);

    int result_status = PQresultStatus(result.psql_result);
    assert(result_status != PGRES_TUPLES_OK);
    bool success = true;
    if (result_status != PGRES_COMMAND_OK) {
		register_error(error, str_copy(PQerrorMessage(db_conn.psql_conn)));
    	success = false;
    }

    db_close_result(result);
    return success;
}

bool db_query(
		struct db_result* result_p, struct db_conn db_conn, const char* sql,
		struct error* error)
{
    result_p->psql_result = PQexec(db_conn.psql_conn, sql);
    int result_status = PQresultStatus(result_p->psql_result);
    assert(result_status != PGRES_COMMAND_OK);
    if (result_status != PGRES_TUPLES_OK) {
		register_error(error, str_copy(PQerrorMessage(db_conn.psql_conn)));
        return false;
	}
	else
	{
		return true;
    }
}

bool db_query_params(
		struct db_result* db_result, struct db_conn db_conn, const char* sql,
		struct db_params params, struct error* error)
{

    struct db_result result = { 0 };
	result.psql_result = PQexecParams(
			db_conn.psql_conn, sql, vec_length(params.vector),
			NULL, vec_elem(params.vector, 0),
			NULL,
			NULL, 0);

    int result_status = PQresultStatus(result.psql_result);
    assert(result_status != PGRES_COMMAND_OK);
	bool success = result_status == PGRES_TUPLES_OK;
	if (!success)
	{
		register_error(error, str_copy(PQerrorMessage(db_conn.psql_conn)));
		print_error(*error);
		abort();
    }

	if (success)
	{
		*db_result = result;
	}
	return success;
}

void db_close_result(struct db_result db_result) {
    PQclear(db_result.psql_result);
}

int db_nrows(struct db_result db_result)
{
    return PQntuples(db_result.psql_result);
}

/* Row and column numbers start at 0. The caller should not free the result directly. */
const char* db_get_value(struct db_result db_result, int row_number, int col_number)
{
    return PQgetvalue(db_result.psql_result, row_number, col_number);
}
