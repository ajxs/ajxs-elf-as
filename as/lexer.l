%{
#include <stdio.h>
#include <string.h>
#include <as.h>
#include <token.h>
#include <parser.h>

#define DEBUG_LEXER 0

%}

WHITESPACE [ \t]
NEGATION_SIGN -
HEX_PREFIX 0x
REGISTER_PREFIX \$
DIRECTIVE_PREFIX \.
STATEMENT_DELIMITER \n|;
ARGUMENT_DELIMITER ,
LABEL_DELIMITER \:
COMMENT_PREFIX #
NUMERIC_LITERAL {NEGATION_SIGN}?{HEX_PREFIX}?[[:alnum:]]+
SYMBOL_VALID_CHARS [[:alpha:]][_[:alnum:]]*
STRING_LITERAL \"[^\"]*\"

%%

{WHITESPACE}

{ARGUMENT_DELIMITER} {
	return ARGUMENT_DELIMITER;
}


"(" {
#if DEBUG_LEXER == 1
	printf("Debug lexer: '('\n");
#endif
	return '(';
}


")" {
#if DEBUG_LEXER == 1
	printf("Debug lexer: ')'\n", yytext);
#endif
	return ')';
}


{COMMENT_PREFIX}(.*) {
#if DEBUG_LEXER == 1
	printf("Debug lexer: COMMENT: `%s`\n", yytext);
#endif
	return COMMENT;
}


{STRING_LITERAL} {
	size_t string_len = strcspn(yytext+1, "\"");
	yylval.text = strndup(yytext+1, string_len);

#if DEBUG_LEXER == 1
	printf("Debug lexer: STRING_LITERAL: `%s`\n", yylval.text);
#endif
	return STRING_LITERAL;
}


"%hi" {
	yylval.mask = OPERAND_MASK_HIGH;

#if DEBUG_LEXER == 1
	printf("Debug lexer: HI_MASK\n", yytext);
#endif
	return MASK;
}


"%lo" {
	yylval.mask = OPERAND_MASK_LOW;
#if DEBUG_LEXER == 1
	printf("Debug lexer: LO_MASK\n", yytext);
#endif
	return MASK;
}

{REGISTER_PREFIX}[[:alnum:]]+ {
	yylval.reg = parse_register_symbol(yytext);

#if DEBUG_LEXER == 1
	printf("Debug lexer: REGISTER: `%i`\n", yylval.reg);
#endif

	return REGISTER;
}


{SYMBOL_VALID_CHARS}{LABEL_DELIMITER} {
	size_t string_len = strcspn(yytext, ":");
	yylval.text = strndup(yytext, string_len);

#if DEBUG_LEXER == 1
	printf("Debug lexer: LABEL: `%s`\n", yytext);
#endif
	return LABEL;
}


{DIRECTIVE_PREFIX}{SYMBOL_VALID_CHARS}+ {
	yylval.dirtype = parse_directive_symbol(yytext);

#if DEBUG_LEXER == 1
	printf("Debug lexer: DIRECTIVE: `%i`\n", yylval.dirtype);
#endif
	return DIRECTIVE;
}


{SYMBOL_VALID_CHARS}+ {
	yylval.text = strdup(yytext);

#if DEBUG_LEXER == 1
	printf("Debug lexer: SYMBOL: `%s`\n", yytext);
#endif
	return SYMBOL;
}


{NUMERIC_LITERAL} {
	yylval.imm = strtol(yytext, NULL, 0);
	
#if DEBUG_LEXER == 1
	printf("Debug lexer: NUMERIC_LITERAL: `%i`\n", yylval.imm);
#endif
	return NUMERIC_LITERAL;
}


{STATEMENT_DELIMITER} {
#if DEBUG_LEXER == 1
	printf("Debug lexer: STATEMENT_DELIMITER\n");
#endif
	return STATEMENT_DELIMITER;
}


. {
#if DEBUG_LEXER == 1
	printf("Debug lexer: Unrecognised: `%s`\n", yytext);
#endif
}

%%

int yywrap(void) {
	return 1;
}


Parsed_Statement *scan_string(const char *str) {
	Parsed_Statement *parsed_statements = NULL;

	YY_BUFFER_STATE buffer = yy_scan_bytes(str, strlen(str));
	yyparse(&parsed_statements);
	yy_delete_buffer(buffer);

	return parsed_statements;
}
