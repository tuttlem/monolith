#!/usr/bin/env bash
set -euo pipefail

hello="HELLO FROM CORE KERNEL (mips)"
log="build/mips/smoke.log"

command -v timeout >/dev/null 2>&1 || { echo "error: timeout command not found." >&2; exit 1; }

make mips >/dev/null

set +e
timeout 12s ./scripts/run-qemu.sh mips >"${log}" 2>&1
rc=$?
set -e

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-mips: qemu failed with rc=${rc}" >&2
  tail -n 120 "${log}" >&2 || true
  exit 1
fi

if tr -d '\r' <"${log}" | grep -Fq "${hello}"; then
  echo "smoke-mips: PASS"
  exit 0
fi

echo "smoke-mips: FAIL (missing hello string)" >&2
tail -n 120 "${log}" >&2 || true
exit 1
