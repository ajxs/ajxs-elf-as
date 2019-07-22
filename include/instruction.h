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

#include <operand.h>
#include <symtab.h>
#include <stdint.h>


typedef struct {
	Instruction_Type type;
	Opcode opcode;
	Operand_Sequence opseq;
} Instruction;


void free_instruction(Instruction *instruction);

void print_instruction(Instruction inst);

#endif
