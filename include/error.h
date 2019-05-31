#ifndef ERROR_H
#define ERROR_H 1

#define ERROR_MSG_MAX_LEN 256


char error_msg[ERROR_MSG_MAX_LEN];

void set_error(const char *error);
void print_error(void);

#endif
