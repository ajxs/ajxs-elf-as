#include <stdio.h>
#include <stdlib.h>
#include <as.h>


void free_statement(Statement *statement) {
	for(size_t i = 0; i < statement->n_labels; i++) {
		free(statement->labels[i]);
	}

	if(statement->type == STATEMENT_TYPE_DIRECTIVE) {
		free_directive(&statement->body.directive);
	} else if(statement->type == STATEMENT_TYPE_INSTRUCTION) {
		free_instruction(&statement->body.instruction);
	}
}
