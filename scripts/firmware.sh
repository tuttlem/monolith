#!/usr/bin/env bash
set -euo pipefail

find_first_existing() {
  local candidate
  for candidate in "$@"; do
    if [[ -f "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done
  return 1
}

find_uefi_code_firmware() {
  local arch="$1"
  case "${arch}" in
    x86_64)
      find_first_existing \
        /usr/share/OVMF/OVMF_CODE.fd \
        /usr/share/OVMF/OVMF_CODE.4m.fd \
        /usr/share/OVMF/OVMF_CODE_4M.fd \
        /usr/share/OVMF/x64/OVMF_CODE.fd \
        /usr/share/OVMF/x64/OVMF_CODE.4m.fd \
        /usr/share/OVMF/x64/QEMU_CODE.fd \
        /usr/share/edk2/ovmf/OVMF_CODE.fd
      ;;
    arm64)
      find_first_existing \
        /usr/share/AAVMF/AAVMF_CODE.fd \
        /usr/share/AAVMF/AAVMF_CODE.ms.fd \
        /usr/share/edk2/aarch64/QEMU_EFI.fd
      ;;
    *)
      return 1
      ;;
  esac
}

find_uefi_vars_firmware() {
  local arch="$1"
  case "${arch}" in
    x86_64)
      find_first_existing \
        /usr/share/OVMF/OVMF_VARS.fd \
        /usr/share/OVMF/OVMF_VARS.4m.fd \
        /usr/share/OVMF/OVMF_VARS_4M.fd \
        /usr/share/OVMF/x64/OVMF_VARS.fd \
        /usr/share/OVMF/x64/OVMF_VARS.4m.fd \
        /usr/share/OVMF/x64/QEMU_VARS.fd \
        /usr/share/edk2/ovmf/OVMF_VARS.fd
      ;;
    arm64)
      find_first_existing \
        /usr/share/AAVMF/AAVMF_VARS.fd \
        /usr/share/AAVMF/AAVMF_VARS.ms.fd \
        /usr/share/edk2/aarch64/vars-template-pflash.raw
      ;;
    *)
      return 1
      ;;
  esac
}

prepare_vars_copy() {
  local template="$1"
  local destination="$2"
  mkdir -p "$(dirname "${destination}")"
  cp "${template}" "${destination}"
}
