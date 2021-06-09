/**
 * @file main.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Main application entry point.
 * Contains the main application entry point and the application logic for
 * managing input options.
 * @version 0.1
 * @date 2019-03-09
 */

#define _GNU_SOURCE

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <as.h>


/**
 * @brief Main entry function.
 * Main entry function of the program.
 */
int main(int argc,
	char **argv);

/**
 * @brief Prints the help text for the program.
 * Prints the help text for the program's command line options to STDOUT.
 */
static void print_help(void);

/**
 * @brief Handles errors parsing the command line arguments.
 * Handles errors parsing the command line arguments, prints out the error string
 * provided and then exits with a failure status.
 * @param error The error to print.
 * @warning Exits with a failure status within this function.
 */
static void handle_opts_error(const char* error);


/**
 * print_help
 */
static void print_help(void) {
	printf("Usage 'ajxs-{ARCH}-elf-as' input_file\n");
	printf("[-?|--help]\n");
	printf("-o|--output\n");
	printf("[-v|--verbose]\n");
	printf("output: The output filename. Defaults to `out.elf`\n");
	printf("verbose: Enables verbose program output.\n");
}


/**
 * handle_opts_error
 */
static void handle_opts_error(const char* error) {
	printf("Error: %s\n", error);
	print_help();
	exit(EXIT_FAILURE);
}


/**
 * main
 */
int main(int argc,
	char **argv)
{
	/** The input filename. */
	const char* input_filename = NULL;
	/** The default output filename. */
	const char* default_output_filename = "./out.elf";

	/**
	 * @brief The output filename.
	 * The output filename defaults to `out.elf` if not specified. This variable
	 * is set with the -o/--output command line argument.
	 */
	const char* output_filename = default_output_filename;
	/** Whether or not verbose mode is enabled. */
	bool verbose = false;
	/** getopts configuration. */
	static struct option long_options[] = {
		{"help", no_argument, NULL, '?'},
		{"output", required_argument, NULL, 'o'},
		{"verbose", no_argument, NULL, 'v'},
		{0, 0, 0, 0}
	};
	/** The option char being checked. */
	int c = 0;
	/** The option index being checked. */
	int option_index = 0;

	while((c = getopt_long(argc, argv, "?o:v", long_options, &option_index)) != -1) {
		switch(c) {
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
			case 'o':
				if(!optarg || strlen(optarg) == 0) {
					handle_opts_error("Invalid output filename.");
				}

				output_filename = optarg;
				break;
			case 'v':
				verbose = true;
				break;
			default:
				handle_opts_error("Unrecognised option.");
		}
	}

	if(optind < argc) {
		// Store first non-option ARGV element as the input filename.
		// Currently only one input file is supported.
		input_filename = argv[optind];
	}


	if(!input_filename || strlen(input_filename) == 0) {
		handle_opts_error("No input filename specified.");
	}

	// Begin the main assembler process.
	Assembler_Status assembler_result = assemble(input_filename, output_filename, verbose);
	if(!get_status(assembler_result)) {
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
