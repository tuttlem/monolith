SPARC64_CC ?= clang
SPARC64_LD ?= ld.lld
SPARC64_CFLAGS := --target=sparc64-none-elf -ffreestanding -fno-stack-protector -fno-builtin -Wall -Wextra -Werror -Ikernel/include -DCORE_ARCH_NAME=\"sparc64\"
SPARC64_ASFLAGS := --target=sparc64-none-elf -x assembler-with-cpp
SPARC64_LDFLAGS := -m elf64_sparc -nostdlib -z max-page-size=0x1000 -T arch/sparc64/linker.ld
