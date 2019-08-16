CC=gcc
CC_FLAGS:=-std=gnu11 -O2 -g -Wall -Wextra -Wmissing-prototypes \
	-Wstrict-prototypes

ARCH:=mips

CC_INCLUDES:=include ../../as/arch/${ARCH}/include
CC_INCLUDE_PARAM:=$(foreach d, ${CC_INCLUDES}, -I$d)

BINARY=test_codegen

ARCH_SOURCES:=../../as/arch/${ARCH}/codegen.c \
	../../as/arch/${ARCH}/elf.c ../../as/arch/${ARCH}/macro.c ../../as/arch/${ARCH}/opcode.c \
	../../as/arch/${ARCH}/register.c ../../as/arch/${ARCH}/statement.c

SOURCES:=${ARCH_SOURCES} ../../as/directive.c ../../as/elf.c ../../as/instruction.c \
	../../as/operand.c ../../as/preprocessor.c \
	../../as/section.c ../../as/statement.c ../../as/symtab.c codegen.c

OBJECTS+=${SOURCES:.c=.o}

.PHONY: clean

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} ${CC_FLAGS} ${OBJECTS} -o ${BINARY}

%.o: %.c
	${CC} ${CC_INCLUDE_PARAM} -c $< -o $@ ${CC_FLAGS}

clean:
	rm ${OBJECTS}
	rm ${BINARY}
