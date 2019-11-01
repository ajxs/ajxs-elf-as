/**
 * @file instruction.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Instruction header.
 * Contains Instruction definitions and specific functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef INSTRUCTION_H
#define INSTRUCTION_H 1

#include <arch.h>
#include <operand.h>
#include <symtab.h>
#include <stdint.h>

/**
 * @brief Instruction type.
 *
 * Represents an encodable instruction entity.
 */
typedef struct {
	Opcode opcode;
	Operand_Sequence opseq;
} Instruction;


/**
 * @brief Frees an instruction.
 *
 * Frees an instruction and the operands that it contains.
 * @param instruction The instruction to free.
 */
void free_instruction(Instruction* instruction);

/**
 * @brief Prints an instruction.
 *
 * This function prints information about an instruction entity.
 * @param instruction The instruction to print.
 */
void print_instruction(Instruction inst);

/**
 * @brief Gets a representation of the provided opcode in string form.
 *
 * This function gets a representation of the provided opcode in string form.
 * @param op The opcode to get the string representation of.
 */
const char* get_opcode_string(Opcode op);

#endif
