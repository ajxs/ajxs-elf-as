#define _GNU_SOURCE

#include <stddef.h>
#include <stdlib.h>
#include <as.h>


int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	char *input_filename = "./test/fac.s";
	char *output_filename = "./out.elf";

	assemble(input_filename, output_filename);

	exit(EXIT_SUCCESS);
}
