#include <as.h>


bool check_operand_count(size_t expected_operand_length,
	Operand_Sequence *opseq);

void free_statement(Statement *statement);
void free_operand(Operand *op);
void free_operand_sequence(Operand_Sequence *opseq);

const char *get_opcode_string(Opcode op);
void print_operand(Operand op);
void print_operand_sequence(Operand_Sequence opseq);
void print_statement(Statement *statement);
