/**
 * @file instruction.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture specific functions for working with instructions.
 * Architecture-specific functionality for working with instruction entities.
 * @version 0.1
 * @date 2019-03-09
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <as.h>
#include <statement.h>


/**
 * get_opcode_string
 */
const char* get_opcode_string(const Opcode op)
{
	if(op == OPCODE_ADDIU) {
		return "ADDIU";
	} else if(op == OPCODE_ADDI) {
		return "ADDI";
	} else if(op == OPCODE_ADDU) {
		return "ADDU";
	} else if(op == OPCODE_ADD) {
		return "ADD";
	} else if(op == OPCODE_BAL) {
		return "BAL";
	} else if(op == OPCODE_BEQZ) {
		return "BEQZ";
	} else if(op == OPCODE_BEQ) {
		return "BEQ";
	} else if(op == OPCODE_BGEZ) {
		return "BGEZ";
	} else if(op == OPCODE_BNE) {
		return "BNE";
	} else if(op == OPCODE_BLEZ) {
		return "BLEZ";
	} else if(op == OPCODE_DIV) {
		return "DIV";
	} else if(op == OPCODE_JALR) {
		return "JALR";
	} else if(op == OPCODE_JAL) {
		return "JAL";
	} else if(op == OPCODE_JR) {
		return "JR";
	} else if(op == OPCODE_J) {
		return "J";
	} else if(op == OPCODE_LA) {
		return "LA";
	} else if(op == OPCODE_LBU) {
		return "LBU";
	} else if(op == OPCODE_LB) {
		return "LB";
	} else if(op == OPCODE_LHU) {
		return "LHU";
	} else if(op == OPCODE_LI) {
		return "LI";
	} else if(op == OPCODE_LUI) {
		return "LUI";
	} else if(op == OPCODE_LW) {
		return "LW";
	} else if(op == OPCODE_MUHU) {
		return "MUHU";
	} else if(op == OPCODE_MUH) {
		return "MUH";
	} else if(op == OPCODE_MUL) {
		return "MUL";
	} else if(op == OPCODE_MULU) {
		return "MULU";
	} else if(op == OPCODE_NOR) {
		return "NOR";
	} else if(op == OPCODE_NOP) {
		return "NOP";
	} else if(op == OPCODE_ORI) {
		return "ORI";
	} else if(op == OPCODE_OR) {
		return "OR";
	} else if(op == OPCODE_SB) {
		return "SB";
	} else if(op == OPCODE_SH) {
		return "SH";
	} else if(op == OPCODE_SLL) {
		return "SLL";
	} else if(op == OPCODE_SUBU) {
		return "SUBU";
	} else if(op == OPCODE_SUB) {
		return "SUB";
	} else if(op == OPCODE_SW) {
		return "SW";
	} else if(op == OPCODE_SYSCALL) {
		return "SYSCALL";
	}

	return "UNKNOWN";
}

