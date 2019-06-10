/**
 * @file directive.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with directive entities.
 * Contains functions for dealing with directive entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <string.h>
#include <as.h>


/**
 * @brief Frees a directive.
 *
 * Frees a directive and the operands that it contains.
 * @param directive The directive to free.
 */
void free_directive(Directive *directive) {
	if(!directive) {
		// @ERROR
		return;
	}

	free_operand_sequence(&directive->opseq);
}


/**
 * @brief Parses a directive symbol.
 *
 * Parses a directive string, returning a value.
 * @param directive_symbol The directive string to parse.
 * @return A directive type.
 */
Directive_Type parse_directive_symbol(char *directive_symbol) {
	if(!directive_symbol) {
		// @ERROR
		return DIRECTIVE_UNKNOWN;
	}

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
