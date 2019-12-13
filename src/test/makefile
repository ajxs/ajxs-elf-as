CC       := gcc
CC_FLAGS := -std=gnu11    \
	-O2                     \
	-g                      \
	-Wall                   \
	-Wextra                 \
	-Wmissing-prototypes    \
	-Wstrict-prototypes

AS_DIR := ../as

CC_INCLUDES      := include    \
	${AS_DIR}/include            \
	${AS_DIR}/arch/${ARCH}/include

CC_INCLUDE_PARAM := $(foreach d, ${CC_INCLUDES}, -I$d)


BINARY := ../../test-${ARCH}-ajxs-elf-as

AS_ARCH_SOURCES := ${AS_DIR}/arch/${ARCH}/codegen.c    \
	${AS_DIR}/arch/${ARCH}/elf.c                         \
	${AS_DIR}/arch/${ARCH}/instruction.c                 \
	${AS_DIR}/arch/${ARCH}/macro.c                       \
	${AS_DIR}/arch/${ARCH}/opcode.c                      \
	${AS_DIR}/arch/${ARCH}/register.c                    \
	${AS_DIR}/arch/${ARCH}/statement.c

AS_SOURCES = ${AS_ARCH_SOURCES}   \
	${AS_DIR}/directive.c           \
	${AS_DIR}/elf.c                 \
	${AS_DIR}/encoding-entity.c     \
	${AS_DIR}/instruction.c         \
	${AS_DIR}/operand.c             \
	${AS_DIR}/preprocessor.c        \
	${AS_DIR}/section.c             \
	${AS_DIR}/statement.c           \
	${AS_DIR}/status.c              \
	${AS_DIR}/symtab.c

TEST_SOURCES:=arch/${ARCH}/codegen.c    \
	main.c                                \
	preprocessor.c


OBJECTS+=${AS_SOURCES:.c=.o}
OBJECTS+=${TEST_SOURCES:.c=.o}

LIBS := -lcunit

.PHONY: clean

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} ${CC_FLAGS} ${OBJECTS} ${LIBS} -o ${BINARY}

%.o: %.c
	${CC} ${CC_INCLUDE_PARAM} -c $< -o $@ ${CC_FLAGS}

clean:
	rm ${OBJECTS}
	rm ${BINARY}
