/*
 * error.h
 *
 *  Created on: 20. aug 2018
 *      Author: raigo
 */

#ifndef ERROR_H_
#define ERROR_H_

#include "vectors.h"

struct error_frame
{
	const char* file_name;
	const char* func_name;
};

struct error {
	const char* message;
	const char* file_name;
	int line_number;
	/* struct error_frame */
	struct vector frames;
};

#define register_error(err_p, message) _register_error(err_p, message, __FILE__, __LINE__)
#define register_frame(err_p) _register_frame(err_p, __FILE__, __func__)

struct error init_error();
void _register_error(
		struct error*, const char* message, const char* file, int line_number);
void _register_frame(
		struct error*, const char* file_name, const char* func_name);
void print_error(struct error);

#endif /* ERROR_H_ */
