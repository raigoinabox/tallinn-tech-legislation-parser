/*
 * files.h
 *
 *  Created on: 20. dets 2018
 *      Author: raigo
 */

#ifndef FILES_H_
#define FILES_H_

#include <stdbool.h>
#include <stdio.h>

#include "legal_act.h"

bool get_default_law_file(
		FILE** result, struct legal_act_id legislation, const char* format);
bool get_default_file(
		FILE** result, const char* file_name, const char* extension);

#endif /* FILES_H_ */
