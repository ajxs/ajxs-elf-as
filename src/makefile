AS_DIR := as
BINARY := ../${ARCH}-ajxs-elf-as

.PHONY: check_arch clean

all: ${BINARY}

${BINARY}: check_arch
	make -C ${AS_DIR}

check_arch:
ifndef ARCH
	$(error No architecture selected)
endif

clean:
	make -C ${AS_DIR} clean
