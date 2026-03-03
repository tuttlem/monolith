#!/usr/bin/env bash
set -euo pipefail

log="build/arm64/smoke-usermode.log"
boot_marker="Starting Monolith (arm64)"
user_marker="usermode: launching init task"

make arm64-uefi >/dev/null

set +e
QEMU_SERIAL="file:${log}" timeout 50s ./scripts/run-qemu.sh arm64 -display none -monitor none >/dev/null 2>&1
rc=$?
set -e

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-usermode-arm64: qemu failed rc=${rc}" >&2
  tail -n 160 "${log}" >&2 || true
  exit 1
fi

if ! tr -d '\r' <"${log}" | grep -Fq "${boot_marker}"; then
  echo "smoke-usermode-arm64: FAIL (missing boot marker)" >&2
  tail -n 160 "${log}" >&2 || true
  exit 1
fi
if ! tr -d '\r' <"${log}" | grep -Fq "${user_marker}"; then
  echo "smoke-usermode-arm64: FAIL (missing user marker)" >&2
  tail -n 160 "${log}" >&2 || true
  exit 1
fi
if tr -d '\r' <"${log}" | grep -Eq "interrupt: unhandled|exception: "; then
  echo "smoke-usermode-arm64: FAIL (unexpected interrupt/exception)" >&2
  tail -n 180 "${log}" >&2 || true
  exit 1
fi

echo "smoke-usermode-arm64: PASS"
