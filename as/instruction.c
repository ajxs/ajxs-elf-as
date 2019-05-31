#include <stdio.h>
#include <stdlib.h>
#include <token.h>
#include <as.h>


void free_instruction(Instruction instruction) {
	for(size_t i=0; i<instruction.opseq.n_operands; i++) {
		free_operand(instruction.opseq.operands[i]);
	}
}


bool instruction_check_operand_length(size_t expected_operand_length,
	Instruction instruction) {

	if(instruction.opseq.n_operands != expected_operand_length) {
		return false;
	} else {
		return true;
	}
}
