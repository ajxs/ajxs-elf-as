/**
 * @file macro.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture specific macro expansion definitions.
 * Contains definitions for macro expansion specific to the MIPS architecture.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef ARCH_MACRO_H
#define ARCH_MACRO_H 1

#include <as.h>
#include <statement.h>

/**
 * @brief Expands a branch delay instruction.
 *
 * This function expands any branching instructions to insert a NOP in the branch
 * delay slot. This funcionality is as-per GCC's functionality.
 * @param macro The branching instruction statement.
 * @warning @p macro is modified in this function.
 */
Assembler_Status expand_branch_delay(Statement* macro);

/**
 * @brief Expands a `la` or `li` psuedo-instruction.
 *
 * This function expands these pseudo-instructions, replacing them with the correct
 * instruction statements in order to perform the prescribed operation.
 * The resulting instructions are highly dependent on the format of the statements,
 * with different formats of operands yielding different results.
 * @param macro The pseudo-instruction statement.
 * @warning @macro is modified in this function. Additional statements may be
 * appended to the end of this statement.
 */
Assembler_Status expand_macro_la(Statement* macro);

/**
 * @brief Expands a `move` pseudo-instruction.
 *
 * This function expands the MIPS `move` pseudo-instruction. This instruction is
 * analogous to adding a number to $0 and storing the result in a register. So
 * the instruction is converted to this form.
 * @param macro The `move` instruction statement.
 * @warning @p macro is modified in this function.
 */
Assembler_Status expand_macro_move(Statement* macro);

#endif
