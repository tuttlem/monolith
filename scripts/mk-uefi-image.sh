#!/usr/bin/env bash
set -euo pipefail

arch="${1:-}"
efi_binary="${2:-}"
out_img="${3:-}"

if [[ -z "${arch}" || -z "${efi_binary}" || -z "${out_img}" ]]; then
  echo "usage: $0 <x86_64|arm64> <efi-binary> <output-image>" >&2
  exit 1
fi

if [[ ! -f "${efi_binary}" ]]; then
  echo "error: EFI binary not found: ${efi_binary}" >&2
  exit 1
fi

command -v truncate >/dev/null 2>&1 || {
  echo "error: truncate not found." >&2
  exit 1
}

command -v sfdisk >/dev/null 2>&1 || {
  echo "error: sfdisk not found." >&2
  exit 1
}

if command -v mformat >/dev/null 2>&1 && command -v mmd >/dev/null 2>&1 && command -v mcopy >/dev/null 2>&1; then
  case "${arch}" in
    x86_64)
      boot_name="BOOTX64.EFI"
      ;;
    arm64)
      boot_name="BOOTAA64.EFI"
      ;;
    *)
      echo "error: unsupported arch '${arch}'" >&2
      exit 1
      ;;
  esac

  mkdir -p "$(dirname "${out_img}")"
  truncate -s 64M "${out_img}"
  sfdisk "${out_img}" <<'EOF'
label: gpt
unit: sectors

start=2048,size=128000,type=uefi
EOF
  mformat -i "${out_img}@@1M" -F ::
  mmd -i "${out_img}@@1M" ::/EFI
  mmd -i "${out_img}@@1M" ::/EFI/BOOT
  mcopy -i "${out_img}@@1M" "${efi_binary}" "::/EFI/BOOT/${boot_name}"
  exit 0
fi

echo "error: mtools commands (mformat, mmd, mcopy) not found." >&2
echo "install mtools, or adapt this script for mkfs.fat flow." >&2
exit 1
