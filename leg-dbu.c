#include <stdlib.h>
#include <stdio.h>

#include "command_line.h"

int main(int argc, const char* argv[]) {
	if (argc < 1) {
		abort();
	}
	struct print_args;
	printf("Hello world!\n");
	return EXIT_SUCCESS;
}
