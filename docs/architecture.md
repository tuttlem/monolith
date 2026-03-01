# Monolith Architecture Overview

Monolith is a minimal, reproducible, multi-architecture operating system
baseline designed to boot cleanly under QEMU using UEFI and print a clear
sign-of-life message from C.

It is intentionally small and intentionally boring.

The goal is not to build a feature-rich kernel here.
The goal is to provide a clean, deterministic foundation that can be forked
into future OS experiments.

---

# Design Philosophy

Monolith is:

- UEFI-first
- Multi-architecture
- Freestanding C
- Deterministic to build and run
- Minimal in scope

Monolith is NOT:

- A production kernel
- A POSIX implementation
- A paging or scheduler showcase
- A driver framework

Every design decision prioritizes:

1. Clarity
2. Portability
2. Reproducibility
4. Clean architectural boundaries

---

# Supported Architectures (v1)

Monolith v1 supports:

- x86_64 (UEFI via OVMF)
- arm64  (UEFI via AAVMF / EDK2)

Other architectures may exist historically or experimentally, but only the
above are first-class and guaranteed to boot and print a sign-of-life message.

---

# High-Level Boot Flow

For both architectures:

```
UEFI firmware
    ↓
BOOT*.EFI (UEFI application)
    ↓
efi_main()
    ↓
OutputString()
    ↓
Infinite halt loop
```

Monolith does NOT currently:

- Exit boot services
- Switch to custom page tables
- Install interrupts
- Enter long-lived kernel runtime

It is a minimal UEFI application that forms the seed of a kernel.

---

# Repository Structure

```
.
├── arch/
│   ├── x86_64/
│   └── arm64/
├── build/ (generated)
├── docs/
├── kernel/
├── lib/
├── scripts/
├── toolchain/
├── Makefile
└── README.md
```

## arch/<arch>/

Architecture-specific build flags, linker scripts, and boot entry points.

Each supported architecture provides:

- boot/efi_main.c
- linker.ld
- arch.mk
- qemu.mk
- include/ (architecture-specific headers if needed)

No architecture code is allowed to leak into kernel/.

## kernel/

Contains architecture-neutral kernel logic.

Currently:

- kmain.c (future expansion point)
- include/kernel.h
- include/uefi.h

The UEFI entry currently prints directly from efi_main.
Future iterations may delegate to kmain() once boot services are abstracted.

## lib/

Minimal freestanding implementations of:

- memset
- memcpy
- strlen

These are provided to avoid libc dependency.

## scripts/

Utility scripts:

- run-qemu.sh
- gdb-qemu.sh
- smoke-x86_64.sh
- smoke-arm64.sh

These enforce deterministic execution and CI-friendly validation.

## build/

Architecture-separated build outputs:

```
build/x86_64/
build/arm64/
```

Each contains:

- uefi.img
- BOOTX64.EFI or BOOTAA64.EFI
- object files
- logs

---

# UEFI Model

Monolith uses the standard removable media boot path:

- x86_64 → EFI/BOOT/BOOTX64.EFI
- arm64  → EFI/BOOT/BOOTAA64.EFI

Images are FAT formatted and generated via:

- mtools (mcopy)
- OR mkfs.fat + file copy

UEFI firmware loads the BOOT*.EFI binary automatically.

---

# Why UEFI?

UEFI was chosen over Multiboot for v1 because:

- It is modern and architecture-neutral.
- It works consistently across x86_64 and arm64.
- It avoids legacy BIOS and i386 complexity.
- It provides a consistent entry contract.

Dropping Multiboot reduces historical baggage and simplifies expansion.

---

# Entry Contract

Each architecture defines:

```c
EFI_STATUS efi_main(EFI_HANDLE ImageHandle,
                    EFI_SYSTEM_TABLE *SystemTable)
```

The contract guarantees:

- A valid SystemTable
- Console output available via:
  SystemTable->ConOut->OutputString

Monolith does not assume:

- Memory map stability
- Boot services exit
- Runtime services persistence

This keeps the baseline safe and deterministic.

---

# Output Model

Sign-of-life message:

- x86_64:
  "HELLO FROM CORE KERNEL (UEFI x86_64)"
- arm64:
  "HELLO FROM CORE KERNEL (UEFI arm64)"

Output uses:

```
SystemTable->ConOut->OutputString()
```

Line endings use CRLF for firmware compatibility.

Serial output is optional but QEMU exposes firmware console via stdio.

---

# Build Model

Top-level Makefile targets:

- make x86_64-uefi
- make run-x86_64
- make gdb-x86_64
- make smoke-x86_64
- make arm64-uefi
- make run-arm64
- make gdb-arm64
- make smoke-arm64
- make clean

Builds are deterministic and architecture-isolated.

---

# QEMU Execution Model

## x86_64

```
qemu-system-x86_64 -machine q35 -m 512M \
  -drive if=pflash,format=raw,readonly=on,file=OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=OVMF_VARS.fd \
  -drive format=raw,file=build/x86_64/uefi.img \
  -serial stdio -no-reboot
```

## arm64

```
qemu-system-aarch64 -machine virt -cpu cortex-a57 -m 512M \
  -drive if=pflash,format=raw,readonly=on,file=AAVMF_CODE.fd \
  -drive if=pflash,format=raw,file=AAVMF_VARS.fd \
  -drive format=raw,file=build/arm64/uefi.img \
  -serial stdio -no-reboot
```

Firmware paths are validated and helpful errors are emitted if missing.

---

# What Is Intentionally Missing

Monolith v1 deliberately excludes:

- Paging / virtual memory
- Interrupt handling
- Timers
- Scheduler
- Drivers
- ExitBootServices
- SMP support
- Device tree parsing

These will be introduced in future forks or milestones.

This repository exists to guarantee:

A clean, multi-architecture UEFI kernel entry baseline.

Nothing more.

---

# Extension Strategy

Future development should preserve:

1. Clear separation between arch/ and kernel/
2. Deterministic Makefile targets
3. Reproducible QEMU execution
4. Minimal boot contract

When expanding:

- Introduce a boot_info abstraction before exiting boot services.
- Keep arch-specific logic inside arch/<arch>/.
- Avoid #ifdef leakage across the kernel.

---

# Architectural Invariants

The following must remain true:

- Each supported architecture boots under QEMU.
- The HELLO message prints reliably.
- The build is freestanding (no libc).
- Output images are self-contained FAT ESP images.
- No architecture leaks into common kernel code.

If any of these fail, the baseline is considered broken.

---

# Why This Exists

Monolith is a launchpad.

It is meant to be:

- Forked
- Mutated
- Broken experimentally
- Rebuilt repeatedly

It is not meant to become bloated.

When it becomes opinionated, it should be forked.

---

# Current Status

Monolith v1:

- Boots via UEFI on x86_64 and arm64
- Prints deterministic sign-of-life
- Uses clean architecture separation
- Builds reproducibly under Ubuntu with QEMU

This marks the stable baseline for future OS experiments.