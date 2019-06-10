#include <string.h>
#include <as.h>


void set_error_message(const char *error) {
	strncpy(error_msg, error, ERROR_MSG_MAX_LEN-1);
}


void print_error(void) {
	return;
}
