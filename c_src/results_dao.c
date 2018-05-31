/*
 * results_dao.c
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#include "results_dao.h"

#include <stdbool.h>
#include <assert.h>

#include "database.h"
#include "printing.h"

void delete_results(sqlite3* db_conn)
{
    sqlite3_stmt* statement = db_prepare_stmt(db_conn,
                              "delete from complexity_results;");
    bool is_row;
    if (!db_step(&is_row, statement))
    {
        printf_ea(sqlite3_errmsg(db_conn));
        abort();
    }
    assert(!is_row);
    db_close_stmt(statement);
}

void insert_result(sqlite3* db_conn, struct complexity_result_dto result)
{
    sqlite3_stmt* statement =
        db_prepare_stmt(db_conn,
                        "insert into complexity_results"
                        " (country, year, dbu_category, complexity, algorithm)"
                        " values (?, ?, ?, ?, ?);");
    db_bind_text2(statement, 1, result.country);
    db_bind_int(statement, 2, result.year);
    db_bind_text2(statement, 3, result.dbu_category);
    db_bind_double(statement, 4, result.complexity);
    db_bind_int(statement, 5, result.algorithm);
    bool is_row;
    if (!db_step(&is_row, statement))
    {
        printf_ea(sqlite3_errmsg(db_conn));
        abort();
    }
    assert(!is_row);

    db_close_stmt(statement);
}
