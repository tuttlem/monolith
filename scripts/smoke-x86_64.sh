#!/usr/bin/env bash
set -euo pipefail

hello="HELLO FROM CORE KERNEL (UEFI x86_64)"
log="build/x86_64/smoke.log"

command -v timeout >/dev/null 2>&1 || {
  echo "error: timeout command not found." >&2
  exit 1
}

make x86_64-uefi >/dev/null

set +e
QEMU_SERIAL="file:${log}" timeout 40s ./scripts/run-qemu.sh x86_64 -display none -monitor none >/dev/null 2>&1
rc=$?
set -e

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-x86_64: qemu failed with rc=${rc}" >&2
  tail -n 120 "${log}" >&2 || true
  exit 1
fi

if tr -d '\r' <"${log}" | grep -Fq "${hello}"; then
  echo "smoke-x86_64: PASS"
  exit 0
fi

echo "smoke-x86_64: FAIL (missing hello string)" >&2
tail -n 120 "${log}" >&2 || true
exit 1
