/**
 * @file print.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Functions for printing information about entities.
 * Functions for printing debugging information about various program entities.
 * Contains functions for identifying and printing instructions/directives and
 * operands/arguments.
 * These functions are for debugging purposes, and will print their output to
 * STDOUT.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <as.h>


/**
 * @brief Prints a statement.
 *
 * This function prints information about a statement entity.
 * @param statement The statement to print.
 */
void print_statement(Statement *statement) {
	if(!statement) {
		set_error_message("Invalid statement provided to print function.");
		return;
	}

	printf("Debug Parser: Statement: Type: `%i`\n", statement->type);
	if(statement->n_labels > 0) {
		printf("  Labels: `%zu`:\n", statement->n_labels);
		for(size_t i=0; i<statement->n_labels; i++) {
			printf("    Label: `%s`\n", statement->labels[i]);
		}
	}

	if(statement->type == STATEMENT_TYPE_DIRECTIVE) {
		print_directive(statement->directive);
	} else if(statement->type == STATEMENT_TYPE_INSTRUCTION) {
		print_instruction(statement->instruction);
	}
}


/**
 * @brief Prints an instruction.
 *
 * This function prints information about an instruction entity.
 * @param instruction The instruction to print.
 */
void print_instruction(Instruction instruction) {
	char *opcode_name = get_opcode_string(instruction.opcode);
	printf("  Instruction: Opcode: `%s`\n", opcode_name);
	if(instruction.opseq.n_operands > 0) {
		print_operand_sequence(instruction.opseq);
	}
}


/**
 * @brief Prints a directive.
 *
 * This function prints information about a directive entity.
 * @param dir The directive to print.
 */
void print_directive(Directive dir) {
	char *directive_name = get_directive_string(dir);
	printf("  Directive: Type: `%s`\n", directive_name);
	if(dir.opseq.n_operands > 0) {
		print_operand_sequence(dir.opseq);
	}
}


/**
 * @brief Gets the string representation of a directive type.
 *
 * This function returns the string representation of a directive type.
 * @param dir The directive to get the string for.
 * @returns a pointer to the string containing the directive name.
 */
char *get_directive_string(Directive dir) {
	if(dir.type == DIRECTIVE_ALIGN) {
		return ".align";
	} else if(dir.type == DIRECTIVE_ASCII) {
		return ".ascii";
	} else if(dir.type == DIRECTIVE_ASCIZ) {
		return ".asciz";
	} else if(dir.type == DIRECTIVE_BSS) {
		return ".bss";
	} else if(dir.type == DIRECTIVE_BYTE) {
		return ".byte";
	} else if(dir.type == DIRECTIVE_DATA) {
		return ".data";
	} else if(dir.type == DIRECTIVE_FILL) {
		return ".fill";
	} else if(dir.type == DIRECTIVE_GLOBAL) {
		return ".global";
	} else if(dir.type == DIRECTIVE_LONG) {
		return ".long";
	} else if(dir.type == DIRECTIVE_SHORT) {
		return ".short";
	} else if(dir.type == DIRECTIVE_SIZE) {
		return ".size";
	} else if(dir.type == DIRECTIVE_SKIP) {
		return ".skip";
	} else if(dir.type == DIRECTIVE_SPACE) {
		return ".space";
	} else if(dir.type == DIRECTIVE_STRING) {
		return ".string";
	} else if(dir.type == DIRECTIVE_TEXT) {
		return ".text";
	} else if(dir.type == DIRECTIVE_WORD) {
		return ".word";
	} else {
		return "unknown";
	}
}


/**
 * @brief Prints an instruction operand.
 *
 * This function prints information about an instruction operand.
 * @param op The operand to print information about.
 */
void print_operand(Operand op) {
	if(op.type == OPERAND_TYPE_NUMERIC_LITERAL) {
		printf("      Operand: Numeric Literal: `%i`", op.numeric_literal);
	} else if(op.type == OPERAND_TYPE_STRING_LITERAL) {
		printf("      Operand: String Literal: `%s`", op.string_literal);
	} else if(op.type == OPERAND_TYPE_SYMBOL) {
		printf("      Operand: Symbol Reference: `%s`", op.symbol);
	} else if(op.type == OPERAND_TYPE_REGISTER) {
		printf("      Operand: Register: `%i`", op.reg);
	} else {
		printf("      Unknown Operand Type");
	}

	if(op.offset != 0) {
		printf(" Offset: `%i`", op.offset);
	}

	if(op.flags.mask != OPERAND_MASK_NONE) {
		printf(" Mask: `%i`", op.flags.mask);
	}

	if(op.flags.shift != 0) {
		printf(" Shift: `%i`", op.flags.shift);
	}

	printf("\n");
}


/**
 * @brief Prints an operand sequence.
 *
 * This function prints an operand sequence entity, printing each operand.
 * @param opseq The operand sequence to print.
 */
void print_operand_sequence(Operand_Sequence opseq) {
	printf("    Operand sequence: len: `%zu`\n", opseq.n_operands);
	for(size_t i=0; i<opseq.n_operands; i++) {
		print_operand(opseq.operands[i]);
	}
}


/**
 * @brief Prints a symbol table.
 *
 * This function prints all of the entries inside a symbol table.
 * @param symbol_table The symbol table to print.
 */
void print_symbol_table(Symbol_Table *symbol_table) {
	if(!symbol_table) {
		set_error_message("Invalid symbol table provided to print function.");
		return;
	}

	for(size_t i = 0; i < symbol_table->n_entries; i++) {
		if(symbol_table->symbols[i].section) {
			// Allow for null symbol entry.
			printf("  Symbol: `%s`", symbol_table->symbols[i].name);
			printf(" in section `%s`", symbol_table->symbols[i].section->name);
			printf(" at `%#zx`\n", symbol_table->symbols[i].offset);
		}
	}
}
