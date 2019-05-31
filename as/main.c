#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>


int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	char *input_filename = "./test/fac.s";
	FILE *input_file = fopen(input_filename, "r");
	if(!input_file) {
		fprintf(stderr, "Error opening file: %i\n", errno);
	}

	assemble(input_file);

	int close_status = fclose(input_file);
	if(close_status) {
		fprintf(stderr, "Error closing file handler: %u! Exiting.\n", errno);
	}

	exit(EXIT_SUCCESS);
}
