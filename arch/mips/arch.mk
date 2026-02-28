MIPS_CC ?= clang
MIPS_LD ?= ld.lld
MIPS_UART_DEFINE ?= -DUART_BASE=0xB80003F8UL
MIPS_CFLAGS := --target=mips-none-elf -ffreestanding -fno-stack-protector -fno-builtin -mno-abicalls -fno-pic -G0 -Wall -Wextra -Werror -Ikernel/include -DCORE_ARCH_NAME=\"mips\" $(MIPS_UART_DEFINE)
MIPS_ASFLAGS := --target=mips-none-elf -mno-abicalls -fno-pic -x assembler-with-cpp
MIPS_LDFLAGS := -m elf32btsmip -nostdlib -T arch/mips/linker.ld
