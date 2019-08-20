CC       := gcc
CC_FLAGS := -std=gnu11    \
	-O2                     \
	-g                      \
	-Wall                   \
	-Wextra                 \
	-Wmissing-prototypes    \
	-Wstrict-prototypes

ARCH:=mips

CC_INCLUDES:=include ../../as/arch/${ARCH}/include
CC_INCLUDE_PARAM:=$(foreach d, ${CC_INCLUDES}, -I$d)

BINARY=test_codegen

SOURCES:=codegen.c

OBJECTS+=${SOURCES:.c=.o}

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
