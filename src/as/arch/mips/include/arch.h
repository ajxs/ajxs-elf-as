/**
 * @file arch.h
 * @author Anthony (ajxs [at] panoptic.online)
 * @brief Architecture specific definitions.
 * Contains definitions specific to the MIPS architecture.
 * @version 0.1
 * @date 2019-03-09
 */

#ifndef ARCH_H
#define ARCH_H 1


/**
 * @brief Opcode type.
 * Enumerated type containing all of the different opcodes in this ISA.
 */
typedef enum {
	OPCODE_UNKNOWN,
	OPCODE_ADD,
	OPCODE_ADDI,
	OPCODE_ADDIU,
	OPCODE_ADDU,
	OPCODE_AND,
	OPCODE_ANDI,
	OPCODE_BAL,
	OPCODE_BEQ,
	OPCODE_BEQZ,
	OPCODE_BGEZ,
	OPCODE_BNE,
	OPCODE_BLEZ,
	OPCODE_DIV,
	OPCODE_J,
	OPCODE_JAL,
	OPCODE_JALR,
	OPCODE_JR,
	OPCODE_LA,
	OPCODE_LB,
	OPCODE_LBU,
	OPCODE_LHU,
	OPCODE_LI,
	OPCODE_LUI,
	OPCODE_LW,
	OPCODE_MOVE,
	OPCODE_MUH,
	OPCODE_MUHU,
	OPCODE_MUL,
	OPCODE_MULU,
	OPCODE_MULT,
	OPCODE_MULTU,
	OPCODE_NOR,
	OPCODE_NOP,
	OPCODE_OR,
	OPCODE_ORI,
	OPCODE_SB,
	OPCODE_SH,
	OPCODE_SLL,
	OPCODE_SUB,
	OPCODE_SUBU,
	OPCODE_SW,
	OPCODE_SYSCALL
} Opcode;


/**
 * @brief Register type.
 * This enumerated type contains all of the register values within this ISA.
 */
typedef enum {
	REGISTER_NONE,
	REGISTER_$ZERO,
	REGISTER_$AT,
	REGISTER_$V0,
	REGISTER_$V1,
	REGISTER_$A0,
	REGISTER_$A1,
	REGISTER_$A2,
	REGISTER_$A3,
	REGISTER_$T0,
	REGISTER_$T1,
	REGISTER_$T2,
	REGISTER_$T3,
	REGISTER_$T4,
	REGISTER_$T5,
	REGISTER_$T6,
	REGISTER_$T7,
	REGISTER_$S0,
	REGISTER_$S1,
	REGISTER_$S2,
	REGISTER_$S3,
	REGISTER_$S4,
	REGISTER_$S5,
	REGISTER_$S6,
	REGISTER_$S7,
	REGISTER_$T8,
	REGISTER_$T9,
	REGISTER_$K0,
	REGISTER_$K1,
	REGISTER_$GP,
	REGISTER_$SP,
	REGISTER_$FP,
	REGISTER_$RA
} Register;


/**
 * @brief Instruction types.
 * This enumerated types contains the different types of instructions this ISA has.
 */
typedef enum {
	INSTRUCTION_TYPE_UNKNOWN,
	INSTRUCTION_TYPE_IMMEDIATE,
	INSTRUCTION_TYPE_JUMP,
	INSTRUCTION_TYPE_REGISTER
} Instruction_Type;


/**
 * @brief Encodes a register operand.
 * 
 * Returns the 8bit encoding of register operand.
 * @param reg The register to encode
 * @return The encoding of this register.
 */
uint8_t encode_operand_register(Register reg);

#endif
