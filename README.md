# Core OS Repository

Minimal, reproducible OS-core baseline for architecture bring-up experiments.

## Supported Architectures

- `x86_64` (UEFI via OVMF)
- `arm64` (UEFI via AAVMF/EDK2)
- `riscv64` (QEMU `virt` + OpenSBI)
- `mips` (QEMU `malta`)
- `sparc64` (QEMU `sun4u`)

Each target boots in QEMU, enters shared `kmain(const boot_info_t *)`, and prints a sign-of-life string.

## Ubuntu Dependencies

```bash
sudo apt update
sudo apt install -y \
  build-essential clang lld make \
  mtools dosfstools gdisk coreutils \
  qemu-system-x86 qemu-system-arm qemu-system-misc qemu-system-mips qemu-system-sparc \
  ovmf qemu-efi-aarch64 opensbi openbios-sparc
```

Notes:
- Some distros package arm64 firmware under `AAVMF`/`edk2` names instead of `qemu-efi-aarch64`.
- `scripts/firmware.sh` and `scripts/run-qemu.sh` detect common firmware paths and fail with clear messages.

## Build Targets

```bash
make x86_64-uefi
make arm64-uefi
make riscv64
make mips
make sparc64
```

## Run Targets

```bash
make run-x86_64
make run-arm64
make run-riscv64
make run-mips
make run-sparc64
```

Default is headless (`QEMU_HEADLESS=1`).
Set `QEMU_HEADLESS=0` for GUI-capable runs where relevant.

## GDB Targets

```bash
make gdb-x86_64
make gdb-arm64
make gdb-riscv64
make gdb-mips
make gdb-sparc64
```

QEMU starts halted with `-S -s` and listens on `tcp::1234`.

## Smoke Tests

```bash
make smoke-x86_64
make smoke-arm64
make smoke-riscv64
make smoke-mips
make smoke-sparc64
```

Each smoke test runs QEMU under timeout, captures output, and checks for the expected HELLO string.

## Outputs

- `build/x86_64/uefi.img`
- `build/arm64/uefi.img`
- `build/riscv64/kernel.elf`
- `build/mips/kernel.elf`
- `build/sparc64/kernel.elf`

## Troubleshooting

1. Missing firmware
- x86_64: install `ovmf`
- arm64: install `qemu-efi-aarch64` (or distro `AAVMF/edk2` firmware package)
- sparc64: install `openbios-sparc`
- riscv64 OpenSBI comes via QEMU/OpenSBI packages (`opensbi` on Ubuntu)

2. Missing compilers/linker
- Install `clang` and `lld`

3. FAT image tooling errors (UEFI targets)
- Install `mtools`, `dosfstools`, `gdisk`

4. No hello output
- Re-run with `make run-<arch>` and inspect terminal output directly.
- On slower hosts, increase timeout in `scripts/smoke-*.sh`.

## Notes

- `i386` and `ppc64` are intentionally not in the active support set.
- Current focus is deterministic boot + shared C kernel entry (`kmain`) only.
