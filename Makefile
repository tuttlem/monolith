SHELL := /usr/bin/env bash

.PHONY: all \
  print-config \
  x86_64-uefi run-x86_64 gdb-x86_64 smoke-x86_64 \
  arm64-uefi run-arm64 gdb-arm64 smoke-arm64 \
  riscv64 run-riscv64 gdb-riscv64 smoke-riscv64 \
  clean

X64_CC ?= clang
A64_CC ?= clang
LLD_LINK ?= lld-link

RISCV64_CC ?= clang
RISCV64_LD ?= ld.lld

BUILD_X64 := build/x86_64
BUILD_A64 := build/arm64
BUILD_RISCV64 := build/riscv64

include arch/x86_64/arch.mk
include arch/arm64/arch.mk
include arch/riscv64/arch.mk

all: x86_64-uefi

print-config:
	@echo "Monolith config defaults (kernel/include/config.h):"
	@rg -n "^[[:space:]]*#define[[:space:]]+MONOLITH_" kernel/include/config.h | sed 's/^/  /'

x86_64-uefi: $(BUILD_X64)/uefi.img
arm64-uefi: $(BUILD_A64)/uefi.img
riscv64: $(BUILD_RISCV64)/kernel.elf

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

$(BUILD_X64)/kernel/panic.o: kernel/panic.c kernel/include/panic.h kernel/include/arch_cpu.h kernel/include/arch_irq.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/interrupts.o: kernel/interrupts.c kernel/include/interrupts.h kernel/include/arch_interrupts.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/kernel/timer.o: kernel/timer.c kernel/include/timer.h kernel/include/arch_timer.h kernel/include/interrupts.h arch/x86_64/arch.mk
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

$(BUILD_X64)/arch/x86_64/timer/timer.o: arch/x86_64/timer/timer.c kernel/include/arch_timer.h arch/x86_64/arch.mk
	@mkdir -p $(@D)
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/BOOTX64.EFI: $(BUILD_X64)/boot/efi_main.o $(BUILD_X64)/kernel/kmain.o $(BUILD_X64)/kernel/print.o $(BUILD_X64)/kernel/status.o $(BUILD_X64)/kernel/panic.o $(BUILD_X64)/kernel/interrupts.o $(BUILD_X64)/kernel/timer.o $(BUILD_X64)/kernel/diag/boot_info.o $(BUILD_X64)/kernel/mm/page_alloc.o $(BUILD_X64)/kernel/mm/kmalloc.o $(BUILD_X64)/arch/x86_64/mm/memory_init.o $(BUILD_X64)/arch/x86_64/mm/early_paging.o $(BUILD_X64)/arch/x86_64/irq/interrupts.o $(BUILD_X64)/arch/x86_64/timer/timer.o
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

$(BUILD_A64)/kernel/panic.o: kernel/panic.c kernel/include/panic.h kernel/include/arch_cpu.h kernel/include/arch_irq.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/interrupts.o: kernel/interrupts.c kernel/include/interrupts.h kernel/include/arch_interrupts.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/timer.o: kernel/timer.c kernel/include/timer.h kernel/include/arch_timer.h kernel/include/interrupts.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/kernel/diag/boot_info.o: kernel/diag/boot_info.c kernel/include/kernel.h kernel/include/diag/boot_info.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/arch/arm64/mm/memory_init.o: arch/arm64/mm/memory_init.c kernel/include/memory_init.h kernel/include/arch/arm64/early_paging.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/arch/arm64/mm/early_paging.o: arch/arm64/mm/early_paging.c kernel/include/arch/arm64/early_paging.h kernel/include/boot_info.h arch/arm64/arch.mk
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

$(BUILD_A64)/arch/arm64/irq/entry.o: arch/arm64/irq/entry.S arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/arch/arm64/timer/timer.o: arch/arm64/timer/timer.c kernel/include/arch_timer.h arch/arm64/arch.mk
	@mkdir -p $(@D)
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/BOOTAA64.EFI: $(BUILD_A64)/boot/efi_main.o $(BUILD_A64)/kernel/kmain.o $(BUILD_A64)/kernel/print.o $(BUILD_A64)/kernel/status.o $(BUILD_A64)/kernel/panic.o $(BUILD_A64)/kernel/interrupts.o $(BUILD_A64)/kernel/timer.o $(BUILD_A64)/kernel/diag/boot_info.o $(BUILD_A64)/kernel/mm/page_alloc.o $(BUILD_A64)/kernel/mm/kmalloc.o $(BUILD_A64)/arch/arm64/mm/memory_init.o $(BUILD_A64)/arch/arm64/mm/early_paging.o $(BUILD_A64)/arch/arm64/irq/interrupts.o $(BUILD_A64)/arch/arm64/irq/entry.o $(BUILD_A64)/arch/arm64/timer/timer.o
	@mkdir -p $(@D)
	@command -v $(LLD_LINK) >/dev/null 2>&1 || { echo "error: $(LLD_LINK) not found. Install lld."; exit 1; }
	$(LLD_LINK) $(A64_UEFI_LDFLAGS) /out:$@ $^

$(BUILD_A64)/uefi.img: $(BUILD_A64)/BOOTAA64.EFI scripts/mk-uefi-image.sh
	@mkdir -p $(@D)
	@./scripts/mk-uefi-image.sh arm64 $(BUILD_A64)/BOOTAA64.EFI $@

RISCV64_SRCS := kernel/kmain.c kernel/print.c kernel/status.c kernel/panic.c kernel/interrupts.c kernel/timer.c kernel/diag/boot_info.c kernel/mm/page_alloc.c kernel/mm/kmalloc.c arch/riscv64/mm/memory_init.c arch/riscv64/mm/early_paging.c arch/riscv64/irq/interrupts.c arch/riscv64/timer/timer.c arch/riscv64/boot/main.c arch/riscv64/boot/console.c lib/memset.c lib/memcpy.c lib/strlen.c
RISCV64_OBJS := $(patsubst %.c,$(BUILD_RISCV64)/%.o,$(RISCV64_SRCS)) $(BUILD_RISCV64)/arch/riscv64/start.o $(BUILD_RISCV64)/arch/riscv64/irq/entry.o

$(BUILD_RISCV64)/kernel.elf: $(RISCV64_OBJS) arch/riscv64/linker.ld
	@mkdir -p $(@D)
	@command -v $(RISCV64_LD) >/dev/null 2>&1 || { echo "error: $(RISCV64_LD) not found. Install lld."; exit 1; }
	$(RISCV64_LD) $(RISCV64_LDFLAGS) -o $@ $(RISCV64_OBJS)

$(BUILD_RISCV64)/%.o: %.c
	@mkdir -p $(@D)
	@command -v $(RISCV64_CC) >/dev/null 2>&1 || { echo "error: $(RISCV64_CC) not found. Install clang."; exit 1; }
	$(RISCV64_CC) $(RISCV64_CFLAGS) -c $< -o $@

$(BUILD_RISCV64)/arch/riscv64/start.o: arch/riscv64/start.S
	@mkdir -p $(@D)
	$(RISCV64_CC) $(RISCV64_ASFLAGS) -c $< -o $@

$(BUILD_RISCV64)/arch/riscv64/irq/entry.o: arch/riscv64/irq/entry.S
	@mkdir -p $(@D)
	$(RISCV64_CC) $(RISCV64_ASFLAGS) -c $< -o $@

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

clean:
	rm -rf build
	mkdir -p build
	touch build/.gitkeep
