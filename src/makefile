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
