#!/usr/bin/env bash
set -euo pipefail

hello="HELLO FROM CORE KERNEL (UEFI arm64)"
log="build/arm64/smoke.log"

command -v timeout >/dev/null 2>&1 || {
  echo "error: timeout command not found." >&2
  exit 1
}

make arm64-uefi >/dev/null

set +e
QEMU_SERIAL="file:${log}" timeout 40s ./scripts/run-qemu.sh arm64 -display none -monitor none >/dev/null 2>&1
rc=$?
set -e

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-arm64: qemu failed with rc=${rc}" >&2
  tail -n 120 "${log}" >&2 || true
  exit 1
fi

if tr -d '\r' <"${log}" | grep -Fq "${hello}"; then
  if ! tr -d '\r' <"${log}" | grep -Fq "device: init class=timer"; then
    echo "smoke-arm64: FAIL (timer init missing)" >&2
    tail -n 140 "${log}" >&2 || true
    exit 1
  fi
  if tr -d '\r' <"${log}" | grep -Eq "interrupt: unhandled|exception: "; then
    echo "smoke-arm64: FAIL (unexpected interrupt/exception)" >&2
    tail -n 140 "${log}" >&2 || true
    exit 1
  fi
  echo "smoke-arm64: PASS"
  exit 0
fi

echo "smoke-arm64: FAIL (missing hello string)" >&2
tail -n 120 "${log}" >&2 || true
exit 1
