RISCV64_CC ?= clang
RISCV64_LD ?= ld.lld
RISCV64_CFLAGS := --target=riscv64-none-elf -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -fno-stack-protector -fno-builtin -fno-pic -msmall-data-limit=0 -Wall -Wextra -Werror -Ikernel/include -DCORE_ARCH_NAME=\"riscv64\"
RISCV64_ASFLAGS := --target=riscv64-none-elf -march=rv64imac -mabi=lp64 -mcmodel=medany -x assembler-with-cpp
RISCV64_LDFLAGS := -m elf64lriscv -nostdlib -T arch/riscv64/linker.ld
