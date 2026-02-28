#!/usr/bin/env bash
set -euo pipefail

hello="HELLO FROM CORE KERNEL (riscv64)"
log="build/riscv64/smoke.log"

command -v timeout >/dev/null 2>&1 || { echo "error: timeout command not found." >&2; exit 1; }

make riscv64 >/dev/null

set +e
timeout 12s ./scripts/run-qemu.sh riscv64 >"${log}" 2>&1
rc=$?
set -e

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-riscv64: qemu failed with rc=${rc}" >&2
  tail -n 120 "${log}" >&2 || true
  exit 1
fi

if tr -d '\r' <"${log}" | grep -Fq "${hello}"; then
  echo "smoke-riscv64: PASS"
  exit 0
fi

echo "smoke-riscv64: FAIL (missing hello string)" >&2
tail -n 120 "${log}" >&2 || true
exit 1
