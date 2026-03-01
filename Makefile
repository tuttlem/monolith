SHELL := /usr/bin/env bash

.PHONY: all \
  x86_64-uefi run-x86_64 gdb-x86_64 smoke-x86_64 \
  arm64-uefi run-arm64 gdb-arm64 smoke-arm64 \
  riscv64 run-riscv64 gdb-riscv64 smoke-riscv64 \
  mips run-mips gdb-mips smoke-mips \
  sparc64 run-sparc64 gdb-sparc64 smoke-sparc64 \
  clean

X64_CC ?= clang
A64_CC ?= clang
LLD_LINK ?= lld-link

RISCV64_CC ?= clang
RISCV64_LD ?= ld.lld
MIPS_CC ?= clang
MIPS_LD ?= ld.lld
SPARC64_CC ?= clang
SPARC64_LD ?= ld.lld

BUILD_X64 := build/x86_64
BUILD_A64 := build/arm64
BUILD_RISCV64 := build/riscv64
BUILD_MIPS := build/mips
BUILD_SPARC64 := build/sparc64

include arch/x86_64/arch.mk
include arch/arm64/arch.mk
include arch/riscv64/arch.mk
include arch/mips/arch.mk
include arch/sparc64/arch.mk

all: x86_64-uefi

x86_64-uefi: $(BUILD_X64)/uefi.img
arm64-uefi: $(BUILD_A64)/uefi.img

riscv64: $(BUILD_RISCV64)/kernel.elf
mips: $(BUILD_MIPS)/kernel.elf
sparc64: $(BUILD_SPARC64)/kernel.elf

$(BUILD_X64)/boot/efi_main.o: arch/x86_64/boot/efi_main.c kernel/include/uefi.h kernel/include/kernel.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	@command -v $(X64_CC) >/dev/null 2>&1 || { echo "error: $(X64_CC) not found. Install clang."; exit 1; }
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/kmain.o: kernel/kmain.c kernel/include/kernel.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/print.o: kernel/print.c kernel/include/kernel.h kernel/include/print.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/status.o: kernel/status.c kernel/include/status.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/interrupts.o: kernel/interrupts.c kernel/include/interrupts.h kernel/include/arch_interrupts.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/diag/boot_info.o: kernel/diag/boot_info.c kernel/include/kernel.h kernel/include/diag/boot_info.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/arch/x86_64/mm/memory_init.o: arch/x86_64/mm/memory_init.c kernel/include/memory_init.h kernel/include/arch/x86_64/early_paging.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/arch/x86_64/mm/early_paging.o: arch/x86_64/mm/early_paging.c kernel/include/arch/x86_64/early_paging.h kernel/include/boot_info.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/mm/page_alloc.o: kernel/mm/page_alloc.c kernel/include/page_alloc.h kernel/include/boot_info.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/mm/kmalloc.o: kernel/mm/kmalloc.c kernel/include/kmalloc.h kernel/include/page_alloc.h kernel/include/boot_info.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/arch/x86_64/irq/interrupts.o: arch/x86_64/irq/interrupts.c kernel/include/arch_interrupts.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/BOOTX64.EFI: $(BUILD_X64)/boot/efi_main.o $(BUILD_X64)/kernel/kmain.o $(BUILD_X64)/kernel/print.o $(BUILD_X64)/kernel/status.o $(BUILD_X64)/kernel/interrupts.o $(BUILD_X64)/kernel/diag/boot_info.o $(BUILD_X64)/kernel/mm/page_alloc.o $(BUILD_X64)/kernel/mm/kmalloc.o $(BUILD_X64)/arch/x86_64/mm/memory_init.o $(BUILD_X64)/arch/x86_64/mm/early_paging.o $(BUILD_X64)/arch/x86_64/irq/interrupts.o
	@mkdir -p $(@D)
	@command -v $(LLD_LINK) >/dev/null 2>&1 || { echo "error: $(LLD_LINK) not found. Install lld."; exit 1; }
	$(LLD_LINK) $(X64_UEFI_LDFLAGS) /out:$@ $^

$(BUILD_X64)/uefi.img: $(BUILD_X64)/BOOTX64.EFI scripts/mk-uefi-image.sh
	@mkdir -p $(@D)
	@./scripts/mk-uefi-image.sh x86_64 $(BUILD_X64)/BOOTX64.EFI $@

$(BUILD_A64)/boot/efi_main.o: arch/arm64/boot/efi_main.c kernel/include/uefi.h kernel/include/kernel.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	@command -v $(A64_CC) >/dev/null 2>&1 || { echo "error: $(A64_CC) not found. Install clang."; exit 1; }
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/kmain.o: kernel/kmain.c kernel/include/kernel.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/print.o: kernel/print.c kernel/include/kernel.h kernel/include/print.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/status.o: kernel/status.c kernel/include/status.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/interrupts.o: kernel/interrupts.c kernel/include/interrupts.h kernel/include/arch_interrupts.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/diag/boot_info.o: kernel/diag/boot_info.c kernel/include/kernel.h kernel/include/diag/boot_info.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/arch/arm64/mm/memory_init.o: arch/arm64/mm/memory_init.c kernel/include/memory_init.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/mm/page_alloc.o: kernel/mm/page_alloc.c kernel/include/page_alloc.h kernel/include/boot_info.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/mm/kmalloc.o: kernel/mm/kmalloc.c kernel/include/kmalloc.h kernel/include/page_alloc.h kernel/include/boot_info.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/arch/arm64/irq/interrupts.o: arch/arm64/irq/interrupts.c kernel/include/arch_interrupts.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/BOOTAA64.EFI: $(BUILD_A64)/boot/efi_main.o $(BUILD_A64)/kernel/kmain.o $(BUILD_A64)/kernel/print.o $(BUILD_A64)/kernel/status.o $(BUILD_A64)/kernel/interrupts.o $(BUILD_A64)/kernel/diag/boot_info.o $(BUILD_A64)/kernel/mm/page_alloc.o $(BUILD_A64)/kernel/mm/kmalloc.o $(BUILD_A64)/arch/arm64/mm/memory_init.o $(BUILD_A64)/arch/arm64/irq/interrupts.o
	@mkdir -p $(@D)
	@command -v $(LLD_LINK) >/dev/null 2>&1 || { echo "error: $(LLD_LINK) not found. Install lld."; exit 1; }
	$(LLD_LINK) $(A64_UEFI_LDFLAGS) /out:$@ $^

$(BUILD_A64)/uefi.img: $(BUILD_A64)/BOOTAA64.EFI scripts/mk-uefi-image.sh
	@mkdir -p $(@D)
	@./scripts/mk-uefi-image.sh arm64 $(BUILD_A64)/BOOTAA64.EFI $@

RISCV64_SRCS := kernel/kmain.c kernel/print.c kernel/status.c kernel/interrupts.c kernel/diag/boot_info.c kernel/mm/page_alloc.c kernel/mm/kmalloc.c arch/riscv64/mm/memory_init.c arch/riscv64/irq/interrupts.c arch/riscv64/boot/main.c arch/riscv64/boot/console.c lib/memset.c lib/memcpy.c lib/strlen.c
RISCV64_OBJS := $(patsubst %.c,$(BUILD_RISCV64)/%.o,$(RISCV64_SRCS)) $(BUILD_RISCV64)/arch/riscv64/start.o

MIPS_SRCS := kernel/kmain.c kernel/print.c kernel/status.c kernel/interrupts.c kernel/diag/boot_info.c kernel/mm/page_alloc.c kernel/mm/kmalloc.c arch/mips/mm/memory_init.c arch/mips/irq/interrupts.c arch/mips/boot/main.c arch/mips/boot/console.c lib/memset.c lib/memcpy.c lib/strlen.c
MIPS_OBJS := $(patsubst %.c,$(BUILD_MIPS)/%.o,$(MIPS_SRCS)) $(BUILD_MIPS)/arch/mips/start.o

SPARC64_SRCS := kernel/kmain.c kernel/print.c kernel/status.c kernel/interrupts.c kernel/diag/boot_info.c kernel/mm/page_alloc.c kernel/mm/kmalloc.c arch/sparc64/mm/memory_init.c arch/sparc64/irq/interrupts.c arch/sparc64/boot/main.c arch/sparc64/boot/console.c lib/memset.c lib/memcpy.c lib/strlen.c
SPARC64_OBJS := $(patsubst %.c,$(BUILD_SPARC64)/%.o,$(SPARC64_SRCS)) $(BUILD_SPARC64)/arch/sparc64/start.o

$(BUILD_RISCV64)/kernel.elf: $(RISCV64_OBJS) arch/riscv64/linker.ld
	@mkdir -p $(@D)
	@command -v $(RISCV64_LD) >/dev/null 2>&1 || { echo "error: $(RISCV64_LD) not found. Install lld."; exit 1; }
	$(RISCV64_LD) $(RISCV64_LDFLAGS) -o $@ $(RISCV64_OBJS)

$(BUILD_MIPS)/kernel.elf: $(MIPS_OBJS) arch/mips/linker.ld
	@mkdir -p $(@D)
	@command -v $(MIPS_LD) >/dev/null 2>&1 || { echo "error: $(MIPS_LD) not found. Install lld."; exit 1; }
	$(MIPS_LD) $(MIPS_LDFLAGS) -o $@ $(MIPS_OBJS)

$(BUILD_SPARC64)/kernel.elf: $(SPARC64_OBJS) arch/sparc64/linker.ld
	@mkdir -p $(@D)
	@command -v $(SPARC64_LD) >/dev/null 2>&1 || { echo "error: $(SPARC64_LD) not found. Install lld."; exit 1; }
	$(SPARC64_LD) $(SPARC64_LDFLAGS) -o $@ $(SPARC64_OBJS)

$(BUILD_RISCV64)/%.o: %.c
	@mkdir -p $(@D)
	@command -v $(RISCV64_CC) >/dev/null 2>&1 || { echo "error: $(RISCV64_CC) not found. Install clang."; exit 1; }
	$(RISCV64_CC) $(RISCV64_CFLAGS) -c $< -o $@

$(BUILD_MIPS)/%.o: %.c
	@mkdir -p $(@D)
	@command -v $(MIPS_CC) >/dev/null 2>&1 || { echo "error: $(MIPS_CC) not found. Install clang."; exit 1; }
	$(MIPS_CC) $(MIPS_CFLAGS) -c $< -o $@

$(BUILD_SPARC64)/%.o: %.c
	@mkdir -p $(@D)
	@command -v $(SPARC64_CC) >/dev/null 2>&1 || { echo "error: $(SPARC64_CC) not found. Install clang."; exit 1; }
	$(SPARC64_CC) $(SPARC64_CFLAGS) -c $< -o $@

$(BUILD_RISCV64)/arch/riscv64/start.o: arch/riscv64/start.S
	@mkdir -p $(@D)
	$(RISCV64_CC) $(RISCV64_ASFLAGS) -c $< -o $@

$(BUILD_MIPS)/arch/mips/start.o: arch/mips/start.S
	@mkdir -p $(@D)
	$(MIPS_CC) $(MIPS_ASFLAGS) -c $< -o $@

$(BUILD_SPARC64)/arch/sparc64/start.o: arch/sparc64/start.S
	@mkdir -p $(@D)
	$(SPARC64_CC) $(SPARC64_ASFLAGS) -c $< -o $@

run-x86_64: x86_64-uefi
	@./scripts/run-qemu.sh x86_64

gdb-x86_64: x86_64-uefi
	@./scripts/gdb-qemu.sh x86_64

smoke-x86_64: x86_64-uefi
	@./scripts/smoke-x86_64.sh

run-arm64: arm64-uefi
	@./scripts/run-qemu.sh arm64

gdb-arm64: arm64-uefi
	@./scripts/gdb-qemu.sh arm64

smoke-arm64: arm64-uefi
	@./scripts/smoke-arm64.sh

run-riscv64: riscv64
	@./scripts/run-qemu.sh riscv64

gdb-riscv64: riscv64
	@./scripts/gdb-qemu.sh riscv64

smoke-riscv64: riscv64
	@./scripts/smoke-riscv64.sh

run-mips: mips
	@./scripts/run-qemu.sh mips

gdb-mips: mips
	@./scripts/gdb-qemu.sh mips

smoke-mips: mips
	@./scripts/smoke-mips.sh

run-sparc64: sparc64
	@./scripts/run-qemu.sh sparc64

gdb-sparc64: sparc64
	@./scripts/gdb-qemu.sh sparc64

smoke-sparc64: sparc64
	@./scripts/smoke-sparc64.sh

clean:
	rm -rf build
	mkdir -p build
	touch build/.gitkeep
