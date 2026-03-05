#!/usr/bin/env bash
set -euo pipefail

log="build/riscv64/smoke-input.log"
serial_log="build/riscv64/smoke-input.serial.log"
clean_log="build/riscv64/smoke-input.clean.log"
boot_marker="Starting Monolith (riscv64)"
hello_marker="HELLO FROM CORE KERNEL (riscv64)"
input_marker="input: backend ready"
prompt_marker="caps: matrix profile="

make riscv64 >/dev/null
rm -f "${serial_log}"

set +e
QEMU_HEADLESS=1 QEMU_SERIAL="file:${serial_log}" timeout 60s ./scripts/run-qemu.sh riscv64 >"${log}" 2>&1
rc=$?
set -e

if [[ -s "${serial_log}" ]]; then
  tr -d '\r' <"${serial_log}" | sed -E 's/\x1b\[[0-9;?]*[[:alpha:]]//g' >"${clean_log}"
else
  tr -d '\r' <"${log}" | sed -E 's/\x1b\[[0-9;?]*[[:alpha:]]//g' >"${clean_log}"
fi

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-input-riscv64: qemu failed rc=${rc}" >&2
  tail -n 200 "${clean_log}" >&2 || true
  exit 1
fi

if ! grep -Fq "${boot_marker}" "${clean_log}" && ! grep -Fq "${hello_marker}" "${clean_log}"; then
  echo "smoke-input-riscv64: FAIL (missing boot marker)" >&2
  tail -n 200 "${clean_log}" >&2 || true
  exit 1
fi
if ! grep -Fq "${input_marker}" "${clean_log}"; then
  echo "smoke-input-riscv64: FAIL (missing input marker)" >&2
  tail -n 200 "${clean_log}" >&2 || true
  exit 1
fi
if ! grep -Fq "${prompt_marker}" "${clean_log}"; then
  echo "smoke-input-riscv64: FAIL (missing prompt marker)" >&2
  tail -n 200 "${clean_log}" >&2 || true
  exit 1
fi
if grep -Eq "interrupt: unhandled|exception: " "${clean_log}"; then
  echo "smoke-input-riscv64: FAIL (unexpected interrupt/exception)" >&2
  tail -n 220 "${clean_log}" >&2 || true
  exit 1
fi

echo "smoke-input-riscv64: PASS"
