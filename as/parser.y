%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "as.h"
#include "token.h"


extern int yylex(void);
void yyerror(Statement **statements, const char *s);

%}


%define api.value.type {union YYSTYPE}

%token <text> LABEL
%token <directive> DIRECTIVE
%token <text> SYMBOL
%token <reg> REGISTER
%token <text> STRING_LITERAL
%token <imm> NUMERIC_LITERAL
%token COMMENT
%token STATEMENT_DELIMITER
%token ARGUMENT_DELIMITER
%token <mask> MASK

%nterm <operand> operand
%nterm <directive> directive
%nterm <instruction> instruction
%nterm <statement> statement
%nterm <opseq> operand_seq

%parse-param {Statement **statements}

// https://www.gnu.org/software/bison/manual/html_node/Printer-Decl.html#Printer-Decl

%destructor {
	free($$);
} LABEL

%destructor {
	free($$);
} SYMBOL

%destructor {
	free($$);
} STRING_LITERAL

%destructor {
	free_operand(&$$);
} operand

%destructor {
	free_operand_sequence(&$$);
} operand_seq

%destructor {
	free_instruction(&$$);
} instruction

%destructor {
	free_directive(&$$);
} directive

%destructor {
	free_statement($$);
} statement

%%

input:
	%empty
	| input statement {
		if(!*statements) {
			*statements = $2;
		} else {
			Statement *curr = *statements;

			while(curr->next) {
				curr = curr->next;
			}

			curr->next = $2;
		}
	}
	;


statement:
	statement STATEMENT_DELIMITER
	| LABEL statement {
		// Add label to existing array.
		$2->n_labels++;
		$2->labels = realloc($2->labels, sizeof(char*) * $2->n_labels);
		$2->labels[$2->n_labels-1] = $<text>1;

		$$ = $2;
	}
	| LABEL {
		Statement *statement = malloc(sizeof(Statement));
		statement->type = STATEMENT_TYPE_EMPTY;
		statement->n_labels = 1;
		statement->labels = malloc(sizeof(char*));
		statement->labels[0] = $<text>1;
		statement->next = NULL;

		$$ = statement;
	}
	| instruction {
		Statement *statement = malloc(sizeof(Statement));
		statement->type = STATEMENT_TYPE_INSTRUCTION;
		statement->instruction = $<instruction>1;
		statement->n_labels = 0;
		statement->labels = NULL;
		statement->next = NULL;

		$$ = statement;
	}
	| directive {
		Statement *statement = malloc(sizeof(Statement));
		statement->type = STATEMENT_TYPE_DIRECTIVE;
		statement->directive = $1;
		statement->n_labels = 0;
		statement->labels = NULL;
		statement->next = NULL;

		$$ = statement;
	}


instruction:
	SYMBOL {
		Instruction instruction;
		instruction.type = INSTRUCTION_TYPE_UNKNOWN;
		instruction.opcode = parse_opcode_symbol($1);
		instruction.opseq.n_operands = 0;
		$$ = instruction;
	}
	| SYMBOL operand_seq {
		Instruction instruction;
		instruction.type = INSTRUCTION_TYPE_UNKNOWN;
		instruction.opcode = parse_opcode_symbol($1);
		instruction.opseq = $2;

		$$ = instruction;
	}
	;


directive:
	DIRECTIVE {
		Directive dir;
		dir.type = $<dirtype>1;

		$$ = dir;
	}
	| DIRECTIVE operand_seq {
		Directive directive;
		directive.type = $<dirtype>1;
		directive.opseq = $2;

		$$ = directive;
	}
	;


operand_seq:
	operand {
		Operand_Sequence opseq;
		opseq.n_operands = 1;
		opseq.operands = malloc(opseq.n_operands * sizeof(Operand));
		opseq.operands[0] = $1;

		$$ = opseq;
	}
	| operand ARGUMENT_DELIMITER operand_seq {
		// Shift the operand onto start of the existing array.
		$3.n_operands++;
		$3.operands = realloc($3.operands, 
			sizeof(Operand) * $3.n_operands);

		memmove(&$3.operands[1], &$3.operands[0],
			sizeof(Operand) * ($3.n_operands - 1));

		$3.operands[0] = $1;

		$$ = $3;
	}
	;


operand:
	REGISTER {
		Operand operand;
		operand.type = OPERAND_TYPE_REGISTER;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.offset = 0;
		operand.value.reg = $<reg>1;
		$$ = operand;
	}
	| '(' REGISTER ')' {
		Operand operand;
		operand.type = OPERAND_TYPE_REGISTER;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.offset = 0;
		operand.value.reg = $<reg>1;
		$$ = operand;
	}
	| NUMERIC_LITERAL '(' REGISTER ')' { 
		Operand operand;
		operand.type = OPERAND_TYPE_REGISTER;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.offset = $1;
		operand.value.reg = $<reg>3;
		$$ = operand;
	}
	| NUMERIC_LITERAL {
		Operand operand;
		operand.type = OPERAND_TYPE_NUMERIC_LITERAL;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.value.numeric_literal = $<imm>1;
		$$ = operand;
	}
	| STRING_LITERAL {
		Operand operand;
		operand.type = OPERAND_TYPE_STRING_LITERAL;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.value.string_literal = $<text>1;
		$$ = operand;
	}
	| SYMBOL {
		Operand operand;
		operand.type = OPERAND_TYPE_SYMBOL;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.value.symbol = $<text>1;
		$$ = operand;
	}
	| MASK '(' SYMBOL ')' {
		Operand operand;
		operand.type = OPERAND_TYPE_SYMBOL;
		operand.flags = DEFAULT_OPERAND_FLAGS;
		operand.flags.mask = $<mask>1;
		operand.value.symbol = $<text>3;
		$$ = operand;
	}
	;


%%

void yyerror(Statement **statements, const char *s) {
	(void)statements;
	fprintf(stderr, "Parser Error: %s\n", s);
}
