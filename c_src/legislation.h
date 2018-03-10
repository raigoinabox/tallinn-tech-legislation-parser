/*
 * legislation.h
 *
 *  Created on: 12. jaan 2018
 *      Author: raigo
 */

#ifndef LEGISLATION_H_
#define LEGISLATION_H_

#include "safe_string.h"

struct leg_id
{
	struct string type;
	struct string year;
	struct string number;
	struct string version_date;
};

struct cleg_id
{
	struct cstring type;
	struct cstring year;
	struct cstring number;
};
struct leg_id leg_init(char* type, char* year, char* number);
struct cleg_id cleg_init(struct leg_id leg);

#endif /* LEGISLATION_H_ */
