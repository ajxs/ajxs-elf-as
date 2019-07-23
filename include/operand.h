/**
 * @file operand.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Operand header.
 * Contains operand definitions and specific functions.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef OPERAND_H
#define OPERAND_H 1

#include <arch.h>
#include <stdbool.h>
#include <stddef.h>


typedef enum {
	OPERAND_MASK_NONE,
	OPERAND_MASK_HIGH,
	OPERAND_MASK_LOW
} Operand_Mask;


typedef struct {
	uint16_t shift;
	Operand_Mask mask;
} Operand_Flags;


static const Operand_Flags DEFAULT_OPERAND_FLAGS = {0, OPERAND_MASK_NONE};


typedef enum {
	OPERAND_TYPE_UNKNOWN,
	OPERAND_TYPE_SYMBOL,
	OPERAND_TYPE_NUMERIC_LITERAL,
	OPERAND_TYPE_STRING_LITERAL,
	OPERAND_TYPE_REGISTER,
} Operand_Type;


typedef struct {
	Operand_Flags flags;
	Operand_Type type;
	uint16_t offset;
	union {
		char *string_literal;
		char *symbol;
		uint32_t numeric_literal;
		Register reg;
	};
} Operand;


typedef struct {
	size_t n_operands;
	Operand *operands;
} Operand_Sequence;


bool check_operand_count(size_t expected_operand_length,
	Operand_Sequence *opseq);

void free_operand(Operand *op);

void free_operand_sequence(Operand_Sequence *opseq);

void print_operand(Operand op);

void print_operand_sequence(Operand_Sequence opseq);

#endif
