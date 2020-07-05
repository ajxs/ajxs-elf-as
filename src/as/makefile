#####################################################################
#  Copyright (c) 2020, AJXS.
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the
#  Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Authors:
#     Anthony <ajxs [at] panoptic.online>
#####################################################################

.POSIX:
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.PHONY: check_arch clean

CC       := gcc
CFLAGS   := -std=gnu11    \
	-O2                     \
	-g                      \
	-Wall                   \
	-Wextra                 \
	-Wmissing-prototypes    \
	-Wstrict-prototypes

CC_INCLUDES      := include arch/${ARCH}/include
CC_INCLUDE_PARAM := $(foreach d, ${CC_INCLUDES}, -I$d)

LDLIBS := -lfl

BINARY := ../../${ARCH}-ajxs-elf-as

LEXER_GEN := lexer.c
PARSER_GEN := parser.c
PARSER_HEADER_GEN := include/parser.h

LEXER_SRC := lexer.l
PARSER_SRC := parser.y

ARCH_SOURCES := arch/${ARCH}/codegen.c    \
	arch/${ARCH}/elf.c                     \
	arch/${ARCH}/instruction.c             \
	arch/${ARCH}/macro.c                   \
	arch/${ARCH}/opcode.c                  \
	arch/${ARCH}/register.c                \
	arch/${ARCH}/statement.c

SOURCES := ${ARCH_SOURCES}   \
	${LEXER_GEN}              \
	${PARSER_GEN}             \
	as.c                      \
	directive.c               \
	elf.c                     \
	encoding-entity.c         \
	instruction.c             \
	input.c                   \
	main.c                    \
	operand.c                 \
	preprocessor.c            \
	section.c                 \
	statement.c               \
	status.c                  \
	symtab.c

OBJECTS := ${SOURCES:.c=.o}

all: ${BINARY}

check_arch:
ifndef ARCH
	$(error No architecture selected)
endif

${BINARY}: check_arch ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LDLIBS} -o ${BINARY}

%.o: %.c
	${CC} ${CC_INCLUDE_PARAM} -c $< -o $@ ${CFLAGS}

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
