CC       := gcc
CC_FLAGS := -std=gnu11    \
	-O2                     \
	-g                      \
	-Wall                   \
	-Wextra                 \
	-Wmissing-prototypes    \
	-Wstrict-prototypes

CC_INCLUDES      = include arch/${ARCH}/include
CC_INCLUDE_PARAM = $(foreach d, ${CC_INCLUDES}, -I$d)

LIBS := -lfl

BINARY = ../../${ARCH}-ajxs-elf-as

LEXER_GEN := lexer.c
PARSER_GEN := parser.c
PARSER_HEADER_GEN := include/parser.h

LEXER_SRC := lexer.l
PARSER_SRC := parser.y

ARCH_SOURCES = arch/${ARCH}/codegen.c    \
	arch/${ARCH}/elf.c                     \
	arch/${ARCH}/macro.c                   \
	arch/${ARCH}/opcode.c                  \
	arch/${ARCH}/register.c                \
	arch/${ARCH}/statement.c

SOURCES = ${ARCH_SOURCES}   \
	${LEXER_GEN}              \
	${PARSER_GEN}             \
	as.c                      \
	directive.c               \
	elf.c                     \
	instruction.c             \
	input.c                   \
	main.c                    \
	preprocessor.c            \
	section.c                 \
	statement.c               \
	status.c                  \
	symtab.c

OBJECTS += ${SOURCES:.c=.o}

.PHONY: check_arch clean

all: ${BINARY}

check_arch:
ifndef ARCH
	$(error No architecture selected)
endif

${BINARY}: check_arch ${OBJECTS}
	${CC} ${CC_FLAGS} ${OBJECTS} ${LIBS} -o ${BINARY}

%.o: %.c
	${CC} ${CC_INCLUDE_PARAM} -c $< -o $@ ${CC_FLAGS}

${LEXER_GEN}: ${PARSER_GEN} ${PARSER_HEADER_GEN}
	flex --outfile=${LEXER_GEN} ${LEXER_SRC}

${PARSER_GEN} ${PARSER_HEADER_GEN}:
	bison --defines=${PARSER_HEADER_GEN}    \
		--output=${PARSER_GEN} ${PARSER_SRC}

clean:
	rm ${LEXER_GEN}
	rm ${PARSER_HEADER_GEN}
	rm ${PARSER_GEN}
	rm ${OBJECTS}
	rm ${BINARY}
