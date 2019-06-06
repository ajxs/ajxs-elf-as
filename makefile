CC=gcc
CC_FLAGS:=-std=gnu99 -O2 -Wall -Wextra -Wmissing-prototypes \
	-Wstrict-prototypes

CC_INCLUDES=include arch/${ARCH}/include
CC_INCLUDE_PARAM=$(foreach d, ${CC_INCLUDES}, -I$d)

LIBS:=-lfl

BINARY=ajxs-${ARCH}-elf-as

LEXER_GEN:=as/lexer.c
PARSER_GEN:=as/parser.c
PARSER_HEADER_GEN:=include/parser.h

LEXER_SRC:=as/lexer.l
PARSER_SRC:=as/parser.y

ARCH_SOURCES:=arch/${ARCH}/codegen.c \
	arch/${ARCH}/elf.c arch/${ARCH}/macro.c arch/${ARCH}/opcode.c \
	arch/${ARCH}/register.c arch/${ARCH}/statement.c

SOURCES:=${ARCH_SOURCES} ${LEXER_GEN} ${PARSER_GEN} \
	as/as.c as/directive.c as/elf.c as/error.c as/instruction.c \
	as/main.c as/operand.c as/preprocessor.c as/print.c \
	as/section.c as/statement.c as/symtab.c

OBJECTS+=${SOURCES:.c=.o}

.PHONY: clean

all: ${BINARY}

${BINARY}: ${OBJECTS}
	${CC} ${CC_FLAGS} ${OBJECTS} ${LIBS} -o ${BINARY}

%.o: %.c
	${CC} ${CC_INCLUDE_PARAM} -c $< -o $@ ${CC_FLAGS}

${LEXER_GEN}: ${PARSER_GEN} ${PARSER_HEADER_GEN}
	flex --outfile=${LEXER_GEN} ${LEXER_SRC}

${PARSER_GEN} ${PARSER_HEADER_GEN}:
	bison --defines=${PARSER_HEADER_GEN} --output=${PARSER_GEN} ${PARSER_SRC}

clean:
	rm ${LEXER_GEN}
	rm ${PARSER_HEADER_GEN}
	rm ${PARSER_GEN}
	rm ${OBJECTS}
	rm ${BINARY}
