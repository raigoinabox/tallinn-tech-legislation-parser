/*
 * convert_csv.c
 *
 *  Created on: 13. märts 2018
 *      Author: raigo
 */

#include "convert_csv.h"

#include <stdio.h>
#include <string.h>

#include "printing.h"

bool convert_csv(const char* prog, const char* command, struct arp_parser parser)
{
    (void) prog;
    (void) command;
    (void) parser;
    // TODO use prog command and parser
    // TODO implement -h, -o, -i
    char buffer1[256];
    while (fgets(buffer1, 256, stdin) != NULL)
    {
        printf_a("%s\n", buffer1);
        // TODO replace strtok
        strtok(buffer1, ";\n");
        const char* node1 = strtok(NULL, ";");
        if (node1 == NULL)
        {
            return false;
        }
        const char* node2 = strtok(NULL, "\n");
        if (node2 == NULL)
        {
            return false;
        }
    }

    return true;
}
