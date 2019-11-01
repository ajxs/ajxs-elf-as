/**
 * @file instruction.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for dealing with instruction entities.
 * Contains functions for dealing with instruction entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <as.h>
#include <instruction.h>
#include <statement.h>


/**
 * free_instruction
 */
void free_instruction(Instruction* instruction)
{
	if(!instruction) {
		fprintf(stderr, "Invalid instruction provided to free function.\n");

		return;
	}

	free_operand_sequence(&instruction->opseq);
}


/**
 * print_instruction
 */
void print_instruction(Instruction instruction)
{
	const char* opcode_name = get_opcode_string(instruction.opcode);
	printf("  Instruction: Opcode: `%s`\n", opcode_name);

	if(instruction.opseq.n_operands > 0) {
		print_operand_sequence(&instruction.opseq);
	}
}
