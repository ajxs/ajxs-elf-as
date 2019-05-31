#ifndef MIPS_MACRO_H
#define MIPS_MACRO_H 1

#include <as.h>

/**
 * @brief The result of a macro expansion.
 *
 * Indicates the result of the expansion of a macro. The expansion functions do not
 * return an actual value, so this is used to track the success of the expansion operation.
 */
typedef enum _expand_macro_status_result {
	EXPAND_MACRO_FAILURE,
	EXPAND_MACRO_SUCCESS,
} Expand_Macro_Result_Status;

Expand_Macro_Result_Status expand_macro_la(Parsed_Statement *macro);
Expand_Macro_Result_Status expand_macro_move(Parsed_Statement *macro);
Expand_Macro_Result_Status expand_branch_delay(Parsed_Statement *macro);

#endif
