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


/**
 * @brief Operand mask type.
 * Specifies how a particular operand is masked.
 */
typedef enum {
	OPERAND_MASK_NONE,
	OPERAND_MASK_HIGH,
	OPERAND_MASK_LOW
} Operand_Mask;


typedef struct {
	uint16_t shift;
	Operand_Mask mask;
} Operand_Flags;

/**
 * @brief The default operand flags.
 * Specifies no shift, and no masks.
 */
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
		char* string_literal;
		char* symbol;
		uint32_t numeric_literal;
		Register reg;
	};
} Operand;

/**
 * @brief Operand sequence type.
 * Represents a sequence of operands attached to a directive or instruction.
 */
typedef struct {
	size_t n_operands;
	Operand* operands;
} Operand_Sequence;


/**
 * @brief Checks the operand count for an operand sequence.
 *
 * Checks that the number of operands provided in a statement matches the expected
 * amount for the statement's instruction opcode.
 * @param expected_operand_length The expected operand length.
 * @param opseq The operand sequence to check.
 * @return A boolean indicating whether the operand count matches the expected count.
 */
bool check_operand_count(const size_t expected_operand_length,
	const Operand_Sequence* opseq);

/**
 * @brief Frees an operand pointer.
 *
 * Frees an operand pointer. Checks if the operand type is dynamically allocated,
 * freeing it if necessary.
 * @param op A pointer to the operand to free.
 */
void free_operand(Operand* op);

/**
 * @brief Frees an operand sequence.
 *
 * Frees a sequence of operands,
 * @param opseq A pointer to the operand sequence to free.
 */
void free_operand_sequence(Operand_Sequence* opseq);

/**
 * @brief Prints an instruction operand.
 *
 * This function prints information about an instruction operand.
 * @param op The operand to print information about.
 */
void print_operand(const Operand* op);

/**
 * @brief Prints an operand sequence.
 *
 * This function prints an operand sequence entity, printing each operand.
 * @param opseq The operand sequence to print.
 */
void print_operand_sequence(const Operand_Sequence* opseq);

#endif
