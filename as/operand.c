#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <token.h>


void free_operand(Operand op) {
	if(op.type == OPERAND_TYPE_STRING_LITERAL) {
		free(op.value.string_literal);
	} else if(op.type == OPERAND_TYPE_SYMBOL) {
		free(op.value.symbol);
	}
}


void free_operand_sequence(Operand_Sequence opseq) {
	for(size_t i=0; i<opseq.n_operands; i++) {
		free_operand(opseq.operands[i]);
	}
}
