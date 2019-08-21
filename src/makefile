BINARY      := ../${ARCH}-ajxs-elf-as
TEST_BINARY := ../test-${ARCH}-ajxs-elf-as

AS_DIR   := as
TEST_DIR := test

.PHONY: check_arch clean

all: ${BINARY} ${TEST_BINARY}

${BINARY}: check_arch
	make -C ${AS_DIR}

${TEST_BINARY}: check_arch
	make -C ${TEST_DIR}

test: ${TEST_BINARY}

check_arch:
ifndef ARCH
	$(error No architecture selected)
endif

clean:
	make -C ${AS_DIR} clean
	make -C ${TEST_DIR} clean
