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
	char* text;
	uint32_t imm;
	Register reg;
	Operand_Mask mask;
	Operand operand;
	Operand_Sequence opseq;
	Opcode opcode;
	Instruction instruction;
	Directive_Type dirtype;
	Directive directive;
	Statement* statement;
};

/**
 * @brief Parses the string representation of a register.
 *
 * This function parses the string representation of a register operand.
 * @param register_symbol The register string to parse.
 * @return The matching register.
 */
Register parse_register_symbol(const char* register_symbol);

/**
 * @brief Parses a string containing an opcode.
 *
 * This function parses a string to find what opcode it corresponds to. In the
 * event that no recognised opcode can be found an `UNKNOWN_OPCODE` result is
 * returned.
 * @param opcode_symbol The C-string containing the opcode value.
 * @return The parsed opcode value.
 */
Opcode parse_opcode_symbol(const char* opcode_symbol);

#endif
