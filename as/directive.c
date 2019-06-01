#include <stdio.h>
#include <string.h>
#include <as.h>


void free_directive(Directive directive) {
	for(size_t i=0; i<directive.opseq.n_operands; i++) {
		free_operand(directive.opseq.operands[i]);
	}
}


Directive_Type parse_directive_symbol(char *directive_symbol) {
	if(!strncasecmp(directive_symbol, ".asciiz", 7) ||
		!strncasecmp(directive_symbol, ".asciz", 6)) {
		return DIRECTIVE_ASCIZ;
	} else if(!strncasecmp(directive_symbol, ".ascii", 6)) {
		return DIRECTIVE_ASCII;
	} else if(!strncasecmp(directive_symbol, ".bss", 4)) {
		return DIRECTIVE_BSS;
	} else if(!strncasecmp(directive_symbol, ".byte", 5)) {
		return DIRECTIVE_BYTE;
	} else if(!strncasecmp(directive_symbol, ".data", 5)) {
		return DIRECTIVE_DATA;
	} else if(!strncasecmp(directive_symbol, ".fill", 5)) {
		return DIRECTIVE_FILL;
	} else if(!strncasecmp(directive_symbol, ".globl", 6) ||
		!strncasecmp(directive_symbol, ".global", 7)) {
		return DIRECTIVE_GLOBAL;
	} else if(!strncasecmp(directive_symbol, ".long", 5)) {
		return DIRECTIVE_LONG;
	} else if(!strncasecmp(directive_symbol, ".short", 6)) {
		return DIRECTIVE_SHORT;
	} else if(!strncasecmp(directive_symbol, ".space", 6)) {
		return DIRECTIVE_SPACE;
	} else if(!strncasecmp(directive_symbol, ".text", 5)) {
		return DIRECTIVE_TEXT;
	} else if(!strncasecmp(directive_symbol, ".word", 5)) {
		return DIRECTIVE_WORD;
	}

	return DIRECTIVE_UNKNOWN;
}
