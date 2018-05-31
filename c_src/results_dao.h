/*
 * results_dao.h
 *
 *  Created on: 16. veebr 2018
 *      Author: raigo
 */

#ifndef RESULTS_DAO_H_
#define RESULTS_DAO_H_

#include <sqlite3.h>
#include <stdint.h>

#include "strings.h"

struct complexity_result_dto
{
    struct string country;
    int32_t year;
    struct string dbu_category;
    double complexity;
    int32_t algorithm;
};

void delete_results(sqlite3* db_conn);
void insert_result(sqlite3* db_conn, struct complexity_result_dto result);

#endif /* RESULTS_DAO_H_ */
