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

/**
 * @brief Encodes an I type instruction.
 *
 * Encodes an I-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param symbol_table The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param opcode The operand encoding.
 * @param rs The rs field to encode.
 * @param rt The rt field to encode.
 * @param imm The imm operand to encode.
 * @param program_counter The current program_counter.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_i_type(Encoding_Entity** encoded_instruction,
	const Symbol_Table* symtab,
	const uint8_t opcode,
	const uint8_t rs,
	const uint8_t rt,
	const Operand imm,
	const size_t program_counter);

/**
 * @brief Encodes a J type instruction.
 *
 * Encodes a J-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * See: https://stackoverflow.com/questions/7877407/jump-instruction-in-mips-assembly#7877528
 * https://stackoverflow.com/questions/6950230/how-to-calculate-jump-target-address-and-branch-target-address
 * @param symtab The symbol table. This is scanned to find any symbols referenced
 * in instruction operands.
 * @param opcode The operand encoding.
 * @param imm The imm operand to encode.
 * @param program_counter The current program_counter.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_j_type(Encoding_Entity** encoded_instruction,
	const Symbol_Table* symtab,
	const uint8_t opcode,
	const Operand imm,
	const size_t program_counter);

/**
 * @brief Encodes an offset type instruction.
 *
 * Encodes an offset type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param opcode The operand encoding.
 * @param rt The rt field to encode.
 * @param reg The reg operand to encode.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_offset_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rt,
	const Operand offset_reg);

/**
 * @brief Encodes an r-type type instruction.
 *
 * Encodes an r-type instruction entity, creating an `Encoding_Entity` instance representing
 * the generated machine code entities to be written into the executable.
 * @param opcode The operand encoding.
 * @param rd The rd field to encode.
 * @param rt The rt field to encode.
 * @param rs The rs field to encode.
 * @param sa The sa field to encode.
 * @param func The func field to encode.
 * @return The encoded instruction entity. Returns `NULL` in case of error.
 */
Assembler_Status encode_r_type(Encoding_Entity** encoded_instruction,
	const uint8_t opcode,
	const uint8_t rd,
	const uint8_t rs,
	const uint8_t rt,
	const uint8_t sa,
	const uint8_t func);

#endif
