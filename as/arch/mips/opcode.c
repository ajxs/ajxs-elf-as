#include <string.h>
#include "as.h"


/**
 * @brief Parses a string containing an opcode.
 *
 * This function parses a string to find what opcode it corresponds to. In the
 * event that no recognised opcode can be found an `UNKNOWN_OPCODE` result is
 * returned.
 * @param opcode_symbol The C-string containing the opcode value.
 * @return The parsed opcode value.
 */
Opcode parse_opcode_symbol(char *opcode_symbol) {
	if(!strncasecmp(opcode_symbol, "addiu", 5)) {
		return OPCODE_ADDIU;
	} else if(!strncasecmp(opcode_symbol, "addi", 4)) {
		return OPCODE_ADDI;
	} else if(!strncasecmp(opcode_symbol, "addu", 4)) {
		return OPCODE_ADDU;
	} else if(!strncasecmp(opcode_symbol, "add", 3)) {
		return OPCODE_ADD;
	} else if(!strncasecmp(opcode_symbol, "bal", 5)) {
		return OPCODE_BAL;
	} else if(!strncasecmp(opcode_symbol, "beqz", 4)) {
		return OPCODE_BEQZ;
	} else if(!strncasecmp(opcode_symbol, "beq", 3)) {
		return OPCODE_BEQ;
	} else if(!strncasecmp(opcode_symbol, "bgez", 4)) {
		return OPCODE_BGEZ;
	} else if(!strncasecmp(opcode_symbol, "bne", 3)) {
		return OPCODE_BNE;
	} else if(!strncasecmp(opcode_symbol, "blez", 4)) {
		return OPCODE_BLEZ;
	} else if(!strncasecmp(opcode_symbol, "div", 3)) {
		return OPCODE_DIV;
	} else if(!strncasecmp(opcode_symbol, "jalr", 4)) {
		return OPCODE_JALR;
	} else if(!strncasecmp(opcode_symbol, "jal", 3)) {
		return OPCODE_JAL;
	} else if(!strncasecmp(opcode_symbol, "jr", 2)) {
		return OPCODE_JR;
	} else if(!strncasecmp(opcode_symbol, "j", 1)) {
		return OPCODE_J;
	} else if(!strncasecmp(opcode_symbol, "la", 2)) {
		return OPCODE_LA;
	} else if(!strncasecmp(opcode_symbol, "lbu", 3)) {
		return OPCODE_LBU;
	} else if(!strncasecmp(opcode_symbol, "lb", 2)) {
		return OPCODE_LB;
	} else if(!strncasecmp(opcode_symbol, "lhu", 3)) {
		return OPCODE_LHU;
	} else if(!strncasecmp(opcode_symbol, "li", 2)) {
		return OPCODE_LI;
	} else if(!strncasecmp(opcode_symbol, "lui", 3)) {
		return OPCODE_LUI;
	} else if(!strncasecmp(opcode_symbol, "lw", 2)) {
		return OPCODE_LW;
	} else if(!strncasecmp(opcode_symbol, "move", 4)) {
		return OPCODE_MOVE;
	} else if(!strncasecmp(opcode_symbol, "muhu", 4)) {
		return OPCODE_MUHU;
	} else if(!strncasecmp(opcode_symbol, "muh", 3)) {
		return OPCODE_MUH;
	} else if(!strncasecmp(opcode_symbol, "multu", 4)) {
		// Deprecated.
		return OPCODE_UNKNOWN;
	} else if(!strncasecmp(opcode_symbol, "mulu", 4)) {
		return OPCODE_MULU;
	} else if(!strncasecmp(opcode_symbol, "mult", 4)) {
		// Deprecated.
		return OPCODE_UNKNOWN;
	} else if(!strncasecmp(opcode_symbol, "mul", 3)) {
		return OPCODE_MUL;
	} else if(!strncasecmp(opcode_symbol, "nor", 3)) {
		return OPCODE_NOR;
	} else if(!strncasecmp(opcode_symbol, "nop", 3)) {
		return OPCODE_NOP;
	} else if(!strncasecmp(opcode_symbol, "ori", 3)) {
		return OPCODE_ORI;
	} else if(!strncasecmp(opcode_symbol, "or", 2)) {
		return OPCODE_OR;
	} else if(!strncasecmp(opcode_symbol, "sb", 2)) {
		return OPCODE_SB;
	} else if(!strncasecmp(opcode_symbol, "sh", 2)) {
		return OPCODE_SH;
	} else if(!strncasecmp(opcode_symbol, "sll", 3)) {
		return OPCODE_SLL;
	} else if(!strncasecmp(opcode_symbol, "subu", 4)) {
		return OPCODE_SUBU;
	} else if(!strncasecmp(opcode_symbol, "sub", 3)) {
		return OPCODE_SUB;
	} else if(!strncasecmp(opcode_symbol, "sw", 2)) {
		return OPCODE_SW;
	} else if(!strncasecmp(opcode_symbol, "syscall", 6)) {
		return OPCODE_SYSCALL;
	}

	return OPCODE_UNKNOWN;
}
