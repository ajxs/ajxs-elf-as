#include <stdlib.h>
#include <string.h>

#include "as.h"
#include "parsing.h"


Register parse_register_symbol(char *register_symbol) {
	if(!strncasecmp(register_symbol, "0", 1) || !strncasecmp(register_symbol, "$0", 2) ||
		!strncasecmp(register_symbol, "$zero", 5)) {

		return REGISTER_$ZERO;
	} else if(!strncasecmp(register_symbol, "$1", 2) || !strncasecmp(register_symbol, "$at", 3)) {
		return REGISTER_$AT;
	} else if(!strncasecmp(register_symbol, "$2", 2) || !strncasecmp(register_symbol, "$v0", 3)) {
		return REGISTER_$V0;
	} else if(!strncasecmp(register_symbol, "$3", 2) || !strncasecmp(register_symbol, "$v1", 3)) {
		return REGISTER_$V1;
	} else if(!strncasecmp(register_symbol, "$4", 2) || !strncasecmp(register_symbol, "$a0", 3)) {
		return REGISTER_$A0;
	} else if(!strncasecmp(register_symbol, "$5", 2) || !strncasecmp(register_symbol, "$a1", 3)) {
		return REGISTER_$A1;
	} else if(!strncasecmp(register_symbol, "$6", 2) || !strncasecmp(register_symbol, "$a2", 3)) {
		return REGISTER_$A2;
	} else if(!strncasecmp(register_symbol, "$7", 2) || !strncasecmp(register_symbol, "$a3", 3)) {
		return REGISTER_$A3;
	} else if(!strncasecmp(register_symbol, "$8", 2) || !strncasecmp(register_symbol, "$t0", 3)) {
		return REGISTER_$T0;
	} else if(!strncasecmp(register_symbol, "$9", 2) || !strncasecmp(register_symbol, "$t1", 3)) {
		return REGISTER_$T1;
	} else if(!strncasecmp(register_symbol, "$10", 3) || !strncasecmp(register_symbol, "$t2", 3)) {
		return REGISTER_$T2;
	} else if(!strncasecmp(register_symbol, "$11", 3) || !strncasecmp(register_symbol, "$t3", 3)) {
		return REGISTER_$T3;
	} else if(!strncasecmp(register_symbol, "$12", 3) || !strncasecmp(register_symbol, "$t4", 3)) {
		return REGISTER_$T4;
	} else if(!strncasecmp(register_symbol, "$13", 3) || !strncasecmp(register_symbol, "$t5", 3)) {
		return REGISTER_$T5;
	} else if(!strncasecmp(register_symbol, "$14", 3) || !strncasecmp(register_symbol, "$t6", 3)) {
		return REGISTER_$T6;
	} else if(!strncasecmp(register_symbol, "$15", 3) || !strncasecmp(register_symbol, "$t7", 3)) {
		return REGISTER_$T7;
	} else if(!strncasecmp(register_symbol, "$16", 3) || !strncasecmp(register_symbol, "$s0", 3)) {
		return REGISTER_$S0;
	} else if(!strncasecmp(register_symbol, "$17", 3) || !strncasecmp(register_symbol, "$s1", 3)) {
		return REGISTER_$S1;
	} else if(!strncasecmp(register_symbol, "$18", 3) || !strncasecmp(register_symbol, "$s2", 3)) {
		return REGISTER_$S2;
	} else if(!strncasecmp(register_symbol, "$19", 3) || !strncasecmp(register_symbol, "$s3", 3)) {
		return REGISTER_$S3;
	} else if(!strncasecmp(register_symbol, "$20", 3) || !strncasecmp(register_symbol, "$s4", 3)) {
		return REGISTER_$S4;
	} else if(!strncasecmp(register_symbol, "$21", 3) || !strncasecmp(register_symbol, "$s5", 3)) {
		return REGISTER_$S5;
	} else if(!strncasecmp(register_symbol, "$22", 3) || !strncasecmp(register_symbol, "$s6", 3)) {
		return REGISTER_$S6;
	} else if(!strncasecmp(register_symbol, "$23", 3) || !strncasecmp(register_symbol, "$s7", 3)) {
		return REGISTER_$S7;
	} else if(!strncasecmp(register_symbol, "$24", 3) || !strncasecmp(register_symbol, "$t8", 3)) {
		return REGISTER_$T8;
	} else if(!strncasecmp(register_symbol, "$25", 3) || !strncasecmp(register_symbol, "$t9", 3)) {
		return REGISTER_$T9;
	} else if(!strncasecmp(register_symbol, "$26", 3) || !strncasecmp(register_symbol, "$k0", 3)) {
		return REGISTER_$K0;
	} else if(!strncasecmp(register_symbol, "$27", 3) || !strncasecmp(register_symbol, "$k1", 3)) {
		return REGISTER_$K1;
	} else if(!strncasecmp(register_symbol, "$28", 3) || !strncasecmp(register_symbol, "$gp", 3)) {
		return REGISTER_$GP;
	} else if(!strncasecmp(register_symbol, "$29", 3) || !strncasecmp(register_symbol, "$sp", 3)) {
		return REGISTER_$SP;
	} else if(!strncasecmp(register_symbol, "$30", 3) || !strncasecmp(register_symbol, "$fp", 3)) {
		return REGISTER_$FP;
	} else if(!strncasecmp(register_symbol, "$31", 3) || !strncasecmp(register_symbol, "$ra", 3)) {
		return REGISTER_$RA;
	}

	return REGISTER_NONE;
}


uint8_t encode_operand_register(Register reg) {
	if(reg == REGISTER_$ZERO) {
		return 0;
	} else if(reg == REGISTER_$AT) {
		return 1;
	} else if(reg == REGISTER_$V0) {
		return 2;
	} else if(reg == REGISTER_$V1) {
		return 3;
	} else if(reg == REGISTER_$A0) {
		return 4;
	} else if(reg == REGISTER_$A1) {
		return 5;
	} else if(reg == REGISTER_$A2) {
		return 6;
	} else if(reg == REGISTER_$A3) {
		return 7;
	} else if(reg == REGISTER_$T0) {
		return 8;
	} else if(reg == REGISTER_$T1) {
		return 9;
	} else if(reg == REGISTER_$T2) {
		return 10;
	} else if(reg == REGISTER_$T3) {
		return 11;
	} else if(reg == REGISTER_$T4) {
		return 12;
	} else if(reg == REGISTER_$T5) {
		return 13;
	} else if(reg == REGISTER_$T6) {
		return 14;
	} else if(reg == REGISTER_$T7) {
		return 15;
	} else if(reg == REGISTER_$S0) {
		return 16;
	} else if(reg == REGISTER_$S1) {
		return 17;
	} else if(reg == REGISTER_$S2) {
		return 18;
	} else if(reg == REGISTER_$S3) {
		return 19;
	} else if(reg == REGISTER_$S4) {
		return 20;
	} else if(reg == REGISTER_$S5) {
		return 21;
	} else if(reg == REGISTER_$S6) {
		return 22;
	} else if(reg == REGISTER_$S7) {
		return 23;
	} else if(reg == REGISTER_$T8) {
		return 24;
	} else if(reg == REGISTER_$T9) {
		return 25;
	} else if(reg == REGISTER_$K0) {
		return 26;
	} else if(reg == REGISTER_$K1) {
		return 27;
	} else if(reg == REGISTER_$GP) {
		return 28;
	} else if(reg == REGISTER_$SP) {
		return 29;
	} else if(reg == REGISTER_$FP) {
		return 30;
	} else if(reg == REGISTER_$RA) {
		return 31;
	}

	return REGISTER_NONE;
}
