/**
 * @file statement.c
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture-specific functions for working with statement entities.
 * Architecture-specific functionality for working with statement entities.
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
 * get_statement_size
 */
Assembler_Status get_statement_size(Statement* statement,
	size_t* statement_size)
{
	if(!statement) {
		fprintf(stderr, "Error: Invalid statement provided to get statement size function\n");

		return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
	}

	if(statement->type == STATEMENT_TYPE_INSTRUCTION) {
		*statement_size = 4;

		return ASSEMBLER_STATUS_SUCCESS;
	}

	if(statement->type == STATEMENT_TYPE_DIRECTIVE) {
		size_t total_len = 0;
		size_t string_len;
		size_t count = 0;
		size_t fill_size = 0;

		switch(statement->directive.type) {
			case DIRECTIVE_ALIGN:
			case DIRECTIVE_DATA:
			case DIRECTIVE_BSS:
			case DIRECTIVE_SIZE:
			case DIRECTIVE_TEXT:
			case DIRECTIVE_GLOBAL:
				*statement_size = 0;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_ASCII:
				for(size_t i=0; i<statement->directive.opseq.n_operands; i++) {
					string_len = strlen(statement->directive.opseq.operands[i].string_literal);
					total_len += string_len;
				}

				*statement_size = total_len;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_STRING:
			case DIRECTIVE_ASCIZ:
				for(size_t i=0; i<statement->directive.opseq.n_operands; i++) {
					// Extra 1 added to account for the trailing NULL byte.
					string_len = strlen(statement->directive.opseq.operands[i].string_literal);
					total_len += (string_len + 1);
				}

				*statement_size = total_len;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_BYTE:
				*statement_size = 1;

				return ASSEMBLER_STATUS_SUCCESS;
				return 1;
			case DIRECTIVE_SHORT:
				*statement_size = 2;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_LONG:
				*statement_size = 4;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_WORD:
				*statement_size = 4;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_FILL:
				count = statement->directive.opseq.operands[0].numeric_literal;
				fill_size = statement->directive.opseq.operands[1].numeric_literal;
				if(fill_size > 8) {
					// Fill size is capped at 8, as per GAS docs.
					// https://ftp.gnu.org/old-gnu/Manuals/gas-2.9.1/html_chapter/as_7.html#SEC91
					fill_size = 8;
				}

				*statement_size = count * fill_size;

				return ASSEMBLER_STATUS_SUCCESS;
			case DIRECTIVE_SKIP:
			case DIRECTIVE_SPACE:
				*statement_size = statement->directive.opseq.operands[0].numeric_literal;

				return ASSEMBLER_STATUS_SUCCESS;
			default:
				fprintf(stderr, "Error: Unknown directive type in get statement size function\n");
				*statement_size = 0;

				return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
		}
	}

	if(statement->type == STATEMENT_TYPE_EMPTY) {
		*statement_size = 0;

		return ASSEMBLER_STATUS_SUCCESS;
	}

	fprintf(stderr, "Error: Unknown statement type in get statement size function\n");
	*statement_size = 0;

	return ASSEMBLER_ERROR_BAD_FUNCTION_ARGS;
}
