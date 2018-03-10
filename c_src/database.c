/*
 * database.c
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#include "database.h"

#include <stdlib.h>

#include "printing.h"


sqlite3* db_open_conn() {
	sqlite3* db_conn;
	int return_code = sqlite3_open("data.db", &db_conn);
	if (return_code != SQLITE_OK) {
		abort();
	}
	return db_conn;
}

void db_close_conn(sqlite3* conn) {
	int return_code = sqlite3_close_v2(conn);
	if (return_code != SQLITE_OK) {
		abort();
	}
}

sqlite3_stmt* db_prepare_stmt(sqlite3* db_conn, const char* sql) {
	sqlite3_stmt* statement;
	int return_code = sqlite3_prepare_v2(db_conn, sql, -1, &statement, NULL);
	if (return_code != SQLITE_OK) {
		printf_ea("sqlite3 error: %s\n", sqlite3_errmsg(db_conn));
		abort();
	}
	return statement;
}

void db_close_stmt(sqlite3_stmt* statement) {
	int return_code = sqlite3_finalize(statement);
	if (return_code != SQLITE_OK) {
		abort();
	}
}

void db_bind_text(sqlite3_stmt* statement, int index, const char* text) {
	int return_code = sqlite3_bind_text(statement, index, text, -1,
	SQLITE_STATIC);
	if (return_code != SQLITE_OK) {
		abort();
	}
}

void db_bind_text2(sqlite3_stmt* statement, int index, struct cstring text) {
	int return_code = sqlite3_bind_text(statement, index, cst_content(text), -1,
	SQLITE_STATIC);
	if (return_code != SQLITE_OK) {
		abort();
	}
}

void db_bind_int(sqlite3_stmt* statement, int index, int integer) {
	int return_code = sqlite3_bind_int(statement, index, integer);
	if (return_code != SQLITE_OK) {
		abort();
	}
}

void db_step(bool* is_row, sqlite3_stmt* statement) {
	int return_code = sqlite3_step(statement);
	if (return_code == SQLITE_ROW) {
		*is_row = true;
	} else if (return_code == SQLITE_DONE) {
		*is_row = false;
	} else {
		abort();
	}
}
