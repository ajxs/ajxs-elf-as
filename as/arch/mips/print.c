/**
 * @file print.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture-specific functions for printing information about entities.
 * Architecture-specific functionality for printing debugging information about
 * various program entities. Contains functions for identifying and printing
 * instructions/directives and operands/arguments.
 * These functions are for debugging purposes, and will print their output to
 * STDOUT.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <as.h>


/**
 * @brief Gets the string representation of an operand.
 *
 * This function returns the string representation of an operand.
 * @param op The operand to get the string for.
 * @returns a pointer to the string containing the operand name.
 */
char *get_opcode_string(Opcode op) {
	if(op == OPCODE_ADDIU) {
		return "addiu";
	} else if(op == OPCODE_ADDI) {
		return "addi";
	} else if(op == OPCODE_ADDU) {
		return "addu";
	} else if(op == OPCODE_ADD) {
		return "add";
	} else if(op == OPCODE_BAL) {
		return "bal";
	} else if(op == OPCODE_BEQZ) {
		return "beqz";
	} else if(op == OPCODE_BEQ) {
		return "beq";
	} else if(op == OPCODE_BGEZ) {
		return "bgez";
	} else if(op == OPCODE_BNE) {
		return "bne";
	} else if(op == OPCODE_BLEZ) {
		return "blez";
	} else if(op == OPCODE_DIV) {
		return "div";
	} else if(op == OPCODE_JALR) {
		return "jalr";
	} else if(op == OPCODE_JAL) {
		return "jal";
	} else if(op == OPCODE_JR) {
		return "jr";
	} else if(op == OPCODE_J) {
		return "j";
	} else if(op == OPCODE_LA) {
		return "la";
	} else if(op == OPCODE_LBU) {
		return "lbu";
	} else if(op == OPCODE_LB) {
		return "lb";
	} else if(op == OPCODE_LHU) {
		return "lhu";
	} else if(op == OPCODE_LI) {
		return "li";
	} else if(op == OPCODE_LUI) {
		return "lui";
	} else if(op == OPCODE_LW) {
		return "lw";
	} else if(op == OPCODE_ADD) {
		return "move";
	} else if(op == OPCODE_MUHU) {
		return "muhu";
	} else if(op == OPCODE_MUH) {
		return "muh";
	} else if(op == OPCODE_MUL) {
		return "mul";
	} else if(op == OPCODE_MULU) {
		return "mulu";
	} else if(op == OPCODE_MUH) {
		return "mul";
	} else if(op == OPCODE_NOR) {
		return "nor";
	} else if(op == OPCODE_NOP) {
		return "nop";
	} else if(op == OPCODE_ORI) {
		return "ori";
	} else if(op == OPCODE_OR) {
		return "or";
	} else if(op == OPCODE_SB) {
		return "sb";
	} else if(op == OPCODE_SH) {
		return "sh";
	} else if(op == OPCODE_SLL) {
		return "sll";
	} else if(op == OPCODE_SUBU) {
		return "subu";
	} else if(op == OPCODE_SUB) {
		return "sub";
	} else if(op == OPCODE_SW) {
		return "sw";
	} else if(op == OPCODE_SYSCALL) {
		return "syscall";
	} else {
		return "Unknown";
	}
}
