#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "command_line.h"
#include "comp_dbu.h"
#include "print_leg.h"

int main(int argc, char const* argv[]) {
	if (argc < 1) {
		abort();
	} else if (argc < 2) {
		print_help(argv[0]);
		return EXIT_FAILURE;
	}

	bool success = false;
	if (strcmp("print", argv[1]) == 0) {
		success = print_leg(argc, argv, 1);
	} else if (strcmp("comp_dbu", argv[1]) == 0) {
		success = comp_dbu(argc, argv, 1);
	}

	if (success) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}
