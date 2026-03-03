#!/usr/bin/env bash
set -euo pipefail

hello="HELLO FROM CORE KERNEL"
start_marker="Starting Monolith (riscv64)"
log="build/riscv64/smoke.log"
serial_log="build/riscv64/smoke.serial.log"
clean_log="build/riscv64/smoke.clean.log"

command -v timeout >/dev/null 2>&1 || { echo "error: timeout command not found." >&2; exit 1; }

make riscv64 >/dev/null

rm -f "${serial_log}"

set +e
QEMU_HEADLESS=1 QEMU_SERIAL="file:${serial_log}" timeout 60s ./scripts/run-qemu.sh riscv64 >"${log}" 2>&1
rc=$?
set -e

# Normalize carriage returns and strip ANSI CSI escapes from firmware output.
if [[ -s "${serial_log}" ]]; then
  tr -d '\r' <"${serial_log}" | sed -E 's/\x1b\[[0-9;?]*[[:alpha:]]//g' >"${clean_log}"
else
  tr -d '\r' <"${log}" | sed -E 's/\x1b\[[0-9;?]*[[:alpha:]]//g' >"${clean_log}"
fi

if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
  echo "smoke-riscv64: qemu failed with rc=${rc}" >&2
  tail -n 120 "${log}" >&2 || true
  exit 1
fi

if grep -Fq "${hello}" "${clean_log}" || grep -Fq "${start_marker}" "${clean_log}"; then
  if ! grep -Fq "device: init class=timer" "${clean_log}"; then
    echo "smoke-riscv64: FAIL (timer init missing)" >&2
    tail -n 140 "${clean_log}" >&2 || true
    exit 1
  fi
  if grep -Eq "interrupt: unhandled|exception: " "${clean_log}"; then
    echo "smoke-riscv64: FAIL (unexpected interrupt/exception)" >&2
    tail -n 140 "${clean_log}" >&2 || true
    exit 1
  fi
  echo "smoke-riscv64: PASS"
  exit 0
fi

echo "smoke-riscv64: FAIL (missing hello string)" >&2
echo "smoke-riscv64: rc=${rc} log_size=$(wc -c <"${log}") serial_size=$(wc -c <"${serial_log}" 2>/dev/null || echo 0)" >&2
tail -n 120 "${clean_log}" >&2 || true
exit 1
