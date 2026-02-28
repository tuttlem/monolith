PPC64_CC ?= clang
PPC64_LD ?= ld.lld
PPC64_CFLAGS := --target=powerpc64-none-elf -mabi=elfv2 -ffreestanding -fno-stack-protector -fno-builtin -fno-pic -Wall -Wextra -Werror -Ikernel/include -DCORE_ARCH_NAME=\"ppc64\"
PPC64_ASFLAGS := --target=powerpc64-none-elf -x assembler-with-cpp
PPC64_LDFLAGS := -m elf64ppc -nostdlib -T arch/ppc64/linker.ld
