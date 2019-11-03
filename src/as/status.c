#include <as.h>
#include <stdbool.h>

/**
 * get_status
 */
bool get_status(const Assembler_Status status)
{
	if(status == ASSEMBLER_STATUS_SUCCESS) {
		return true;
	}

	return false;
}
