
# ajxs-elf-as

A basic general purpose ELF format Assembler.
Assembly syntax is based on the GNU assembler, most directives supported by `GCC`/`as` should be supported. Assembles relocatable ELF object files.
This project is for educational purposes only and does not intend to create a fully-featured assembler suitable for production use, however the functionality is intended to be close to parity with a production assembler. For clarification, this does not create executable binaries, it creates *relocatable* object binaries, which can be linked into an executable with an ELF compatible linker. The decision to target this functionality was made on the basis that it was easier to create a useful utility.

Currently a minimal set of the MIPS32r6 instruction set has been implemented.


## Usage
Example command-line usage:

```bash
# Replace ${ARCH} with the configured target architecture.
./ajxs-${ARCH}-elf-as --output=./output.elf input_file.S
```

## Building
This project requires GNU `flex` and `bison` in order to be built from source. Binaries are readily available for most Linux distros.
The target architecture is configured at build time by setting the `ARCH` environment variable. e.g.

```bash
# Build targeting MIPS.
export ARCH=mips
```

## Targeting a new architecture
The source of the assembler is split into architecture-generic and architecture-specific sections. All arch-specific code is within the `as/arch/${ARCH}` folder. Implementing a new target architecture can be accomplished without needing an in-depth understanding of the assembler's internal functionality.
To target a new architecture you would first need to create a new directory corresponding to your new target architecture within the `as/arch/...` directory structure.
To generate code for a new target arch, you need to implement functions to satisfy the interface for code and header generation.
The following functional interface needs to be satisfied:

| Function | Purpose
|--|--|
|`expand_macros` |Expands any assembler macros or pseudo-instructions. If not needed, this can safely be implemented as a pass-through.|
|`get_statement_size`|Gets the size of a particular assembler statement, used during the first assembler pass to calculate symbol offsets.|
|`encode_instruction`|Generates the binary data encoding for an instruction.
|`encode_directive`|Generate the binary data encoding for an assembler directive.
|`get_opcode_string`|Returns a string representation of an opcode for error-handling and debugging purposes.
|`create_elf_header`|Creates an ELF file header entity specific to the target architecture.


The interface for these functions can be seen in `as/include/as.h`. The MIPS implementation provided can be used for a simple example to copy from.

The specified target name corresponds to the directories and files contained within the `as/arch/...` directory.


## What does not work (yet)
- Does not support subsections.
- Does not support legacy instructions deprecated in, or prior to `mips32r6`.
- *Currently* does not support expression expansion in assembler directives. However, this will eventually be implemented.
- Does not support the `.set` directives used by GAS.
- `.align` directive functionality within `.text` sections may be incorrect. GAS implementation of this directive varies by architecture. This is an area that needs more research. See: [Using as: 7.3 .align](https://sourceware.org/binutils/docs/as/Align.html)


## Reporting Bugs
Please do! Please provide any evidence of failures in the `issues` tab of Github, or email me directly.


## Official Documentation
Here are the sources that were used when researching MIPS, AS, and ELF. These should also be listed in comments where relevant in the source files.
If you are looking to implement a similar assembler, the following sources will definitely be useful at various points.


[MIPS® Architecture for Programmers Volume II-A: The MIPS32® Instruction Set Manual](https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00086-2B-MIPS32BIS-AFP-6.06.pdf)

[MIPS® Architecture For Programmers Volume I-A: Introduction to the MIPS32® Architecture](https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD00082-2B-MIPS32INT-AFP-06.01.pdf)

[SYSTEM V APPLICATION BINARY INTERFACE MIPS RISC Processor Supplement 3rd Edition](https://www.linux-mips.org/pub/linux/mips/doc/ABI/psABI_mips3.0.pdf)

[MIPS ELF header definitions](https://dmz-portal.mips.com/wiki/MIPS_ELF_header_definitions)

[System V Application Binary Interface](http://www.sco.com/developers/gabi/2012-12-31/contents.html)

[Using as](https://sourceware.org/binutils/docs/as/index.html)

## Helpful sources

[Oracle ELF information](https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-54839.html)
