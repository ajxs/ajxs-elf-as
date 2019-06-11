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
	printf("  Instruction: Opcode: `");
	print_opcode(instruction.opcode);
	printf("`\n");
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
	printf("  Directive: Type: `");
	print_directive_type(dir);
	printf("`\n");

	if(dir.opseq.n_operands > 0) {
		print_operand_sequence(dir.opseq);
	}
}


/**
 * @brief Prints a directive type.
 *
 * This function identifies and prints the type of a directive entity.
 * @param dir The directive to print the type of.
 */
void print_directive_type(Directive dir) {
	if(dir.type == DIRECTIVE_ALIGN) {
		printf(".align");
	} else if(dir.type == DIRECTIVE_ASCII) {
		printf(".ascii");
	} else if(dir.type == DIRECTIVE_ASCIZ) {
		printf(".asciz");
	} else if(dir.type == DIRECTIVE_BSS) {
		printf(".bss");
	} else if(dir.type == DIRECTIVE_BYTE) {
		printf(".byte");
	} else if(dir.type == DIRECTIVE_DATA) {
		printf(".data");
	} else if(dir.type == DIRECTIVE_FILL) {
		printf(".fill");
	} else if(dir.type == DIRECTIVE_GLOBAL) {
		printf(".global");
	} else if(dir.type == DIRECTIVE_LONG) {
		printf(".long");
	} else if(dir.type == DIRECTIVE_SHORT) {
		printf(".short");
	} else if(dir.type == DIRECTIVE_SIZE) {
		printf(".size");
	} else if(dir.type == DIRECTIVE_SKIP) {
		printf(".skip");
	} else if(dir.type == DIRECTIVE_SPACE) {
		printf(".space");
	} else if(dir.type == DIRECTIVE_STRING) {
		printf(".string");
	} else if(dir.type == DIRECTIVE_TEXT) {
		printf(".text");
	} else if(dir.type == DIRECTIVE_WORD) {
		printf(".word");
	} else {
		printf("unknown");
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
 * @brief Prints an operand.
 *
 * This function prints information about an operand.
 * @param op The operand to print.
 */
void print_opcode(Opcode op) {
	if(op == OPCODE_ADDIU) {
		printf("addiu");
	} else if(op == OPCODE_ADDI) {
		printf("addi");
	} else if(op == OPCODE_ADDU) {
		printf("addu");
	} else if(op == OPCODE_ADD) {
		printf("add");
	} else if(op == OPCODE_BAL) {
		printf("bal");
	} else if(op == OPCODE_BEQZ) {
		printf("beqz");
	} else if(op == OPCODE_BEQ) {
		printf("beq");
	} else if(op == OPCODE_BGEZ) {
		printf("bgez");
	} else if(op == OPCODE_BNE) {
		printf("bne");
	} else if(op == OPCODE_BLEZ) {
		printf("blez");
	} else if(op == OPCODE_DIV) {
		printf("div");
	} else if(op == OPCODE_JALR) {
		printf("jalr");
	} else if(op == OPCODE_JAL) {
		printf("jal");
	} else if(op == OPCODE_JR) {
		printf("jr");
	} else if(op == OPCODE_J) {
		printf("j");
	} else if(op == OPCODE_LA) {
		printf("la");
	} else if(op == OPCODE_LBU) {
		printf("lbu");
	} else if(op == OPCODE_LB) {
		printf("lb");
	} else if(op == OPCODE_LHU) {
		printf("lhu");
	} else if(op == OPCODE_LI) {
		printf("li");
	} else if(op == OPCODE_LUI) {
		printf("lui");
	} else if(op == OPCODE_LW) {
		printf("lw");
	} else if(op == OPCODE_ADD) {
		printf("move");
	} else if(op == OPCODE_MUHU) {
		printf("muhu");
	} else if(op == OPCODE_MUH) {
		printf("muh");
	} else if(op == OPCODE_MUL) {
		printf("mul");
	} else if(op == OPCODE_MULU) {
		printf("mulu");
	} else if(op == OPCODE_MUH) {
		printf("mul");
	} else if(op == OPCODE_NOR) {
		printf("nor");
	} else if(op == OPCODE_NOP) {
		printf("nop");
	} else if(op == OPCODE_ORI) {
		printf("ori");
	} else if(op == OPCODE_OR) {
		printf("or");
	} else if(op == OPCODE_SB) {
		printf("sb");
	} else if(op == OPCODE_SH) {
		printf("sh");
	} else if(op == OPCODE_SLL) {
		printf("sll");
	} else if(op == OPCODE_SUBU) {
		printf("subu");
	} else if(op == OPCODE_SUB) {
		printf("sub");
	} else if(op == OPCODE_SW) {
		printf("sw");
	} else if(op == OPCODE_SYSCALL) {
		printf("syscall");
	} else {
		printf("Unknown opcode: %u", op);
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
