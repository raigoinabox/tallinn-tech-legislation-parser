/*
 * comp_dbu.h
 *
 *  Created on: 2. veebr 2018
 *      Author: raigo
 */

#ifndef SAVE_DBU_COMPL_H_
#define SAVE_DBU_COMPL_H_

#include <stdbool.h>

#include "arg_parsing.h"
#include "error.h"

bool save_dbu_compl(const char* prog, const char* command,
                    struct arp_parser, struct error*);

#endif /* SAVE_DBU_COMPL_H_ */
