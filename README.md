# Core OS Repository (UEFI-First v1)

Minimal, reproducible OS core baseline for experiments.

Supported architectures in v1:
- `x86_64` via UEFI + OVMF
- `arm64` via UEFI + AAVMF/EDK2

Each architecture builds a minimal UEFI application in C and prints a sign-of-life:
- `HELLO FROM CORE KERNEL (UEFI x86_64)`
- `HELLO FROM CORE KERNEL (UEFI arm64)`

## Outputs

- `build/x86_64/uefi.img`
- `build/arm64/uefi.img`

Each image is a FAT ESP containing:
- `EFI/BOOT/BOOTX64.EFI` for `x86_64`
- `EFI/BOOT/BOOTAA64.EFI` for `arm64`

## Ubuntu Prerequisites

```bash
sudo apt update
sudo apt install -y \
  build-essential clang lld \
  qemu-system-x86 qemu-system-arm ovmf qemu-efi-aarch64 \
  mtools dosfstools gdb
```

Notes:
- Some distros provide arm64 UEFI firmware under `AAVMF` paths via `edk2` packages instead of `qemu-efi-aarch64`.
- The scripts auto-detect common firmware paths and print a clear error if missing.

## Build Targets

```bash
make x86_64-uefi
make arm64-uefi
```

## Run Targets

```bash
make run-x86_64
make run-arm64
```

By default, run scripts use headless mode (`QEMU_HEADLESS=1`) and print serial output in the terminal.
For a GUI window, run with `QEMU_HEADLESS=0`.

The run scripts use these QEMU baselines:

`x86_64`:
```bash
qemu-system-x86_64 -machine q35 -m 512M \
  -drive if=pflash,format=raw,readonly=on,file=<OVMF_CODE.fd> \
  -drive if=pflash,format=raw,file=<OVMF_VARS copy> \
  -drive format=raw,file=build/x86_64/uefi.img \
  -serial stdio -no-reboot
```

`arm64`:
```bash
qemu-system-aarch64 -machine virt -cpu cortex-a57 -m 512M \
  -drive if=pflash,format=raw,readonly=on,file=<AAVMF_CODE.fd> \
  -drive if=pflash,format=raw,file=<AAVMF_VARS copy> \
  -drive format=raw,file=build/arm64/uefi.img \
  -serial stdio -no-reboot
```

## GDB Targets

```bash
make gdb-x86_64
make gdb-arm64
```

These launch QEMU with `-S -s` and wait for a debugger on `tcp::1234`.

## Smoke Tests

```bash
make smoke-x86_64
make smoke-arm64
```

Smoke tests run QEMU briefly in headless mode and grep output for the expected hello string.

## Troubleshooting

1. Firmware not found
- `x86_64`: install `ovmf`.
- `arm64`: install `qemu-efi-aarch64` or distro `AAVMF/EDK2` firmware package.
- You can inspect detection logic in `scripts/firmware.sh`.

2. Clang/lld missing
- Install `clang` and `lld` (`lld-link` is used for PE/COFF UEFI linking).

3. FAT image tool errors
- Install `mtools`. The image builder uses `mformat`, `mmd`, and `mcopy`.

4. No hello string in smoke tests
- Retry with `make run-x86_64` or `make run-arm64` to inspect interactive output.
- On slower machines, increase timeout in `scripts/smoke-*.sh`.

## Notes

- `i386` support is intentionally removed in v1.
- Legacy GRUB/Multiboot flow is removed from default paths to keep the baseline UEFI-first.
