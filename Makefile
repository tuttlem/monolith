SHELL := /usr/bin/env bash

.PHONY: all x86_64-uefi run-x86_64 gdb-x86_64 smoke-x86_64 arm64-uefi run-arm64 gdb-arm64 smoke-arm64 clean

X64_CC ?= clang
A64_CC ?= clang
LLD_LINK ?= lld-link

BUILD_X64 := build/x86_64
BUILD_A64 := build/arm64

include arch/x86_64/arch.mk
include arch/arm64/arch.mk

all: x86_64-uefi

x86_64-uefi: $(BUILD_X64)/uefi.img

arm64-uefi: $(BUILD_A64)/uefi.img

$(BUILD_X64)/boot/efi_main.o: arch/x86_64/boot/efi_main.c kernel/include/uefi.h
	@mkdir -p $(@D)
	@command -v $(X64_CC) >/dev/null 2>&1 || { echo "error: $(X64_CC) not found. Install clang."; exit 1; }
	$(X64_CC) $(X64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_X64)/BOOTX64.EFI: $(BUILD_X64)/boot/efi_main.o
	@mkdir -p $(@D)
	@command -v $(LLD_LINK) >/dev/null 2>&1 || { echo "error: $(LLD_LINK) not found. Install lld."; exit 1; }
	$(LLD_LINK) $(X64_UEFI_LDFLAGS) /out:$@ $<

$(BUILD_X64)/uefi.img: $(BUILD_X64)/BOOTX64.EFI scripts/mk-uefi-image.sh
	@mkdir -p $(@D)
	@./scripts/mk-uefi-image.sh x86_64 $(BUILD_X64)/BOOTX64.EFI $@

$(BUILD_A64)/boot/efi_main.o: arch/arm64/boot/efi_main.c kernel/include/uefi.h
	@mkdir -p $(@D)
	@command -v $(A64_CC) >/dev/null 2>&1 || { echo "error: $(A64_CC) not found. Install clang."; exit 1; }
	$(A64_CC) $(A64_UEFI_CFLAGS) -c $< -o $@

$(BUILD_A64)/BOOTAA64.EFI: $(BUILD_A64)/boot/efi_main.o
	@mkdir -p $(@D)
	@command -v $(LLD_LINK) >/dev/null 2>&1 || { echo "error: $(LLD_LINK) not found. Install lld."; exit 1; }
	$(LLD_LINK) $(A64_UEFI_LDFLAGS) /out:$@ $<

$(BUILD_A64)/uefi.img: $(BUILD_A64)/BOOTAA64.EFI scripts/mk-uefi-image.sh
	@mkdir -p $(@D)
	@./scripts/mk-uefi-image.sh arm64 $(BUILD_A64)/BOOTAA64.EFI $@

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

clean:
	rm -rf build
	mkdir -p build
	touch build/.gitkeep
