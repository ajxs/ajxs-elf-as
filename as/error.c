/**
 * @file error.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Error reporting and handling functionality.
 * Contains functions for handling and reporting program errors.
 * @version 0.1
 * @date 2019-03-09
 */

#include <string.h>
#include <as.h>


/**
 * @brief Sets the global error message.
 * Sets the global error message, this message is accessible through the whole
 * application. This function can be used to ensure that the details behind
 * an application failure is correctly propagated to the final error handler.
 * @param error The error to print
 */
void set_error_message(const char *error) {
	strncpy(assembler_error_msg, error, ERROR_MSG_MAX_LEN-1);
}


/**
 * @brief Prints the global error message.
 * Prints the global assembler error message to STDERR.
 */
void print_error(void) {
	fprintf(stderr, "Error: %s\n", assembler_error_msg);
}
