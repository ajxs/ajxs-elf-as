/**
 * @file codegen.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for generation of encoded binary machine-code from instructions.
 * Contains functions for generation of machine-code from parsed instructions specific
 * to the MIPS architecture.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef CODEGEN_H
#define CODEGEN_H 1

#include <as.h>
#include <symtab.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


Assembler_Status encode_r_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rd,
	const uint8_t rs,
	const uint8_t rt,
	const uint8_t sa,
	const uint8_t func);

Encoding_Entity *encode_offset_type(char *error_message,
	uint8_t opcode,
	uint8_t rt,
	Operand offset_reg);

Encoding_Entity *encode_i_type(char *error_message,
	Symbol_Table *symtab,
	uint8_t opcode,
	uint8_t rs,
	uint8_t rt,
	Operand imm,
	size_t program_counter);

Encoding_Entity *encode_j_type(char *error_message,
	Symbol_Table *symtab,
	uint8_t opcode,
	Operand imm,
	size_t program_counter);

#endif
