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


Assembler_Status encode_i_type(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symtab,
	const uint8_t opcode,
	const uint8_t rs,
	const uint8_t rt,
	const Operand imm,
	const size_t program_counter);

Assembler_Status encode_j_type(Encoding_Entity** encoded_instruction,
	Symbol_Table* const symtab,
	const uint8_t opcode,
	const Operand imm,
	const size_t program_counter);

Assembler_Status encode_offset_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rt,
	const Operand offset_reg);

Assembler_Status encode_r_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rd,
	const uint8_t rs,
	const uint8_t rt,
	const uint8_t sa,
	const uint8_t func);

#endif
