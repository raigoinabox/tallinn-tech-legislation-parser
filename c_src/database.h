/*
 * database.h
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <sqlite3.h>
#include <stdbool.h>

#include "strings.h"

sqlite3* db_open_conn();
void db_close_conn(sqlite3* conn);
sqlite3_stmt* db_prepare_stmt(sqlite3* db_conn, const char* sql);
void db_close_stmt(sqlite3_stmt* stmt);
void db_bind_text(sqlite3_stmt* statement, int index, const char* text);
void db_bind_text2(sqlite3_stmt* statement, int index, struct string text);
void db_bind_int(sqlite3_stmt* statement, int index, int integer);
void db_bind_double(sqlite3_stmt* statement, int index, double value);
bool db_step(bool* is_row, sqlite3_stmt* statement);

#endif /* DATABASE_H_ */
