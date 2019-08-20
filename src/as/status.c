#include <as.h>
#include <stdbool.h>

bool get_status(const Assembler_Status status) {
	if(status == ASSEMBLER_STATUS_SUCCESS) {
		return true;
	} else {
		return false;
	}
}
