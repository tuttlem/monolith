#!/usr/bin/env bash
set -euo pipefail

arch="${1:-}"
shift || true

if [[ -z "${arch}" ]]; then
  echo "usage: $0 <x86_64|arm64|riscv64|mips|sparc64> [extra-qemu-flags...]" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=/dev/null
source "${SCRIPT_DIR}/firmware.sh"

headless="${QEMU_HEADLESS:-1}"
if [[ "${headless}" == "1" ]]; then
  qemu_ui_flags=(-display none -monitor none)
else
  qemu_ui_flags=(-monitor none)
fi

case "${arch}" in
  x86_64)
    command -v qemu-system-x86_64 >/dev/null 2>&1 || {
      echo "error: qemu-system-x86_64 not found. Install qemu-system-x86." >&2
      exit 1
    }
    img="build/x86_64/uefi.img"
    [[ -f "${img}" ]] || {
      echo "error: missing image ${img}. Run 'make x86_64-uefi'." >&2
      exit 1
    }

    code_fw="$(find_uefi_code_firmware x86_64 || true)"
    vars_fw="$(find_uefi_vars_firmware x86_64 || true)"

    if [[ -z "${code_fw}" || -z "${vars_fw}" ]]; then
      echo "error: x86_64 UEFI firmware not found." >&2
      echo "install package: ovmf" >&2
      exit 1
    fi

    vars_copy="build/x86_64/OVMF_VARS.fd"
    prepare_vars_copy "${vars_fw}" "${vars_copy}"

    serial_backend="${QEMU_SERIAL:-stdio}"

    exec qemu-system-x86_64 -machine q35 -m 512M \
      -drive if=pflash,format=raw,readonly=on,file="${code_fw}" \
      -drive if=pflash,format=raw,file="${vars_copy}" \
      -drive format=raw,file="${img}" \
      -serial "${serial_backend}" -no-reboot \
      "${qemu_ui_flags[@]}" "$@"
    ;;
  arm64)
    command -v qemu-system-aarch64 >/dev/null 2>&1 || {
      echo "error: qemu-system-aarch64 not found. Install qemu-system-arm." >&2
      exit 1
    }
    img="build/arm64/uefi.img"
    [[ -f "${img}" ]] || {
      echo "error: missing image ${img}. Run 'make arm64-uefi'." >&2
      exit 1
    }

    code_fw="$(find_uefi_code_firmware arm64 || true)"
    vars_fw="$(find_uefi_vars_firmware arm64 || true)"

    if [[ -z "${code_fw}" || -z "${vars_fw}" ]]; then
      echo "error: arm64 UEFI firmware not found." >&2
      echo "install package: qemu-efi-aarch64 or edk2/aavmf firmware package" >&2
      exit 1
    fi

    vars_copy="build/arm64/AAVMF_VARS.fd"
    prepare_vars_copy "${vars_fw}" "${vars_copy}"

    serial_backend="${QEMU_SERIAL:-stdio}"

    exec qemu-system-aarch64 -machine virt -cpu cortex-a57 -m 512M \
      -drive if=pflash,format=raw,readonly=on,file="${code_fw}" \
      -drive if=pflash,format=raw,file="${vars_copy}" \
      -drive format=raw,file="${img}" \
      -serial "${serial_backend}" -no-reboot \
      "${qemu_ui_flags[@]}" "$@"
    ;;
  riscv64)
    command -v qemu-system-riscv64 >/dev/null 2>&1 || {
      echo "error: qemu-system-riscv64 not found. Install qemu-system-misc." >&2
      exit 1
    }
    img="build/riscv64/kernel.elf"
    [[ -f "${img}" ]] || {
      echo "error: missing kernel ${img}. Run 'make riscv64'." >&2
      exit 1
    }
    exec qemu-system-riscv64 -machine virt -m 512M -bios default \
      -device loader,file="${img}",cpu-num=0 \
      -serial stdio -nographic -no-reboot \
      -monitor none "$@"
    ;;
  mips)
    command -v qemu-system-mips >/dev/null 2>&1 || {
      echo "error: qemu-system-mips not found. Install qemu-system-mips." >&2
      exit 1
    }
    img="build/mips/kernel.elf"
    [[ -f "${img}" ]] || {
      echo "error: missing kernel ${img}. Run 'make mips'." >&2
      exit 1
    }
    exec qemu-system-mips -machine malta -m 256M \
      -kernel "${img}" -serial stdio -nographic -no-reboot \
      -monitor none "$@"
    ;;
  sparc64)
    command -v qemu-system-sparc64 >/dev/null 2>&1 || {
      echo "error: qemu-system-sparc64 not found. Install qemu-system-sparc." >&2
      exit 1
    }
    img="build/sparc64/kernel.elf"
    [[ -f "${img}" ]] || {
      echo "error: missing kernel ${img}. Run 'make sparc64'." >&2
      exit 1
    }
    exec qemu-system-sparc64 -M sun4u -m 256M \
      -kernel "${img}" -serial stdio -nographic -no-reboot \
      -monitor none "$@"
    ;;
  *)
    echo "error: unsupported arch '${arch}'" >&2
    exit 1
    ;;
esac
