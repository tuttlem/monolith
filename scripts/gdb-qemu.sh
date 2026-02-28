#!/usr/bin/env bash
set -euo pipefail

arch="${1:-}"

if [[ -z "${arch}" ]]; then
  echo "usage: $0 <x86_64|arm64|riscv64|mips|sparc64>" >&2
  exit 1
fi

echo "QEMU waiting for GDB on tcp::1234"
exec ./scripts/run-qemu.sh "${arch}" -S -s
