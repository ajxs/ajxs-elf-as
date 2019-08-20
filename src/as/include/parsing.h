/**
 * @file parsing.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for parsing and lexing.
 * Contains functions for and definitions for parsing and lexing of input files.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef PARSING_H
#define PARSING_H 1

#include <stdint.h>
#include <as.h>
#include <directive.h>
#include <instruction.h>
#include <operand.h>
#include <statement.h>


union YYSTYPE {
	char *text;
	uint32_t imm;
	Register reg;
	Operand_Mask mask;
	Operand operand;
	Operand_Sequence opseq;
	Opcode opcode;
	Instruction instruction;
	Directive_Type dirtype;
	Directive directive;
	Statement *statement;
};

Register parse_register_symbol(const char* register_symbol);

Opcode parse_opcode_symbol(const char* opcode_symbol);

#endif
