#ifndef TOKEN_H
#define TOKEN_H 1

#include <stdint.h>
#include <as.h>


union YYSTYPE {
	char *text;
	uint32_t imm;
	Register reg;
	Operand_Mask mask;
	Operand operand;
	Operand_Sequence opseq;
	Opcode opcode;
	Instruction instruction;
	Directive_Type dirtype;
	Directive directive;
	Statement *statement;
};


#endif
