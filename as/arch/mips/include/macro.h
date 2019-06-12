#ifndef MIPS_MACRO_H
#define MIPS_MACRO_H 1

#include <as.h>

Expand_Macro_Result_Status expand_macro_la(Statement *macro);
Expand_Macro_Result_Status expand_macro_move(Statement *macro);
Expand_Macro_Result_Status expand_branch_delay(Statement *macro);

#endif
