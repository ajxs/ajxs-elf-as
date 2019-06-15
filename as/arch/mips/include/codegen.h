/**
 * @file codegen.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for generation of encoded binary machine-code from instructions.
 * Contains functions for generation of machine-code from parsed instructions specific
 * to the MIPS architecture.
 * @version 0.1
 * @date 2019-03-09
 */

#include <as.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


Encoding_Entity *encode_r_type(char *error_message,
	uint8_t opcode,
	uint8_t rd,
	uint8_t rs,
	uint8_t rt,
	uint8_t sa,
	uint8_t func);

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
