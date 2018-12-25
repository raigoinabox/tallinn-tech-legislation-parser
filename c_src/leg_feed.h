/*
 * leg_feed_parsing.h
 *
 *  Created on: 11. okt 2018
 *      Author: raigo
 */

#ifndef LEG_FEED_H_
#define LEG_FEED_H_

#include "error.h"
#include "arg_parsing.h"

bool search_laws(
		const char* prog, const char* command, struct arp_parser parser,
		struct error* error);

#endif /* LEG_FEED_H_ */
