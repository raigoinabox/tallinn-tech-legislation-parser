/*
 * estonia.h
 *
 *  Created on: 19. dets 2018
 *      Author: raigo
 */

#ifndef ESTONIA_H_
#define ESTONIA_H_

#include <stdbool.h>

#include "arg_parsing.h"
#include "error.h"

bool estonia(
		const char* prog, const char* command, struct arp_parser, struct error*);

// bool (*command)(const char*, const char*, struct arp_parser, struct error*);

#endif /* ESTONIA_H_ */
