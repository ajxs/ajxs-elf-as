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


void set_error_message(const char *error) {
	strncpy(error_msg, error, ERROR_MSG_MAX_LEN-1);
}


void print_error(void) {
	return;
}
