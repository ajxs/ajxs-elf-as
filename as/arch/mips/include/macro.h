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


Assembler_Status expand_macro_la(Statement *macro);

Assembler_Status expand_macro_move(Statement *macro);

Assembler_Status expand_branch_delay(Statement *macro);

#endif
