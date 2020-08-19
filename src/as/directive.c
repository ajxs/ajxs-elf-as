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
#include <directive.h>
#include <statement.h>


/**
 * free_directive
 */
void free_directive(Directive* directive)
{
	if(!directive) {
		fprintf(stderr, "Invalid directive provided to free function.\n");

		return;
	}

	free_operand_sequence(&directive->opseq);
}


/**
 * get_directive_string
 */
const char* get_directive_string(const Directive* directive)
{
	if(directive->type == DIRECTIVE_ALIGN) {
		return ".ALIGN";
	} else if(directive->type == DIRECTIVE_ASCII) {
		return ".ASCII";
	} else if(directive->type == DIRECTIVE_ASCIZ) {
		return ".ASCIZ";
	} else if(directive->type == DIRECTIVE_BSS) {
		return ".BSS";
	} else if(directive->type == DIRECTIVE_BYTE) {
		return ".BYTE";
	} else if(directive->type == DIRECTIVE_DATA) {
		return ".DATA";
	} else if(directive->type == DIRECTIVE_FILL) {
		return ".FILL";
	} else if(directive->type == DIRECTIVE_GLOBAL) {
		return ".GLOBAL";
	} else if(directive->type == DIRECTIVE_LONG) {
		return ".LONG";
	} else if(directive->type == DIRECTIVE_SHORT) {
		return ".SHORT";
	} else if(directive->type == DIRECTIVE_SIZE) {
		return ".SIZE";
	} else if(directive->type == DIRECTIVE_SKIP) {
		return ".SKIP";
	} else if(directive->type == DIRECTIVE_SPACE) {
		return ".SPACE";
	} else if(directive->type == DIRECTIVE_STRING) {
		return ".STRING";
	} else if(directive->type == DIRECTIVE_TEXT) {
		return ".TEXT";
	} else if(directive->type == DIRECTIVE_WORD) {
		return ".WORD";
	} else {
		return "UNKNOWN";
	}
}


/**
 * parse_directive_symbol
 */
Directive_Type parse_directive_symbol(const char* directive_symbol)
{
	if(!directive_symbol) {
		fprintf(stderr, "Invalid directive symbol provided to parse function\n");
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
