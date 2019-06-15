#include <as.h>

void print_directive(Directive dir);
void free_directive(Directive *directive);
const char *get_directive_string(Directive dir);
Directive_Type parse_directive_symbol(char *directive_symbol);
