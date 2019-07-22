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
 * @brief Frees an instruction.
 *
 * Frees an instruction and the operands that it contains.
 * @param instruction The instruction to free.
 */
void free_instruction(Instruction *instruction) {
	if(!instruction) {
		fprintf(stderr, "Invalid instruction provided to free function.\n");
		return;
	}

	free_operand_sequence(&instruction->opseq);
}


/**
 * @brief Prints an instruction.
 *
 * This function prints information about an instruction entity.
 * @param instruction The instruction to print.
 */
void print_instruction(Instruction instruction) {
	const char *opcode_name = get_opcode_string(instruction.opcode);
	printf("  Instruction: Opcode: `%s`\n", opcode_name);
	if(instruction.opseq.n_operands > 0) {
		print_operand_sequence(instruction.opseq);
	}
}
