#!/usr/bin/env bash
set -euo pipefail

log="build/riscv64/smoke-usermode.log"
serial_log="build/riscv64/smoke-usermode.serial.log"
clean_log="build/riscv64/smoke-usermode.clean.log"
boot_marker="Starting Monolith (riscv64)"
user_marker="usermode: launching init task"
probe_marker="sys_debug: usermode: probe trap ok"

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
  echo "smoke-usermode-riscv64: qemu failed rc=${rc}" >&2
  tail -n 180 "${clean_log}" >&2 || true
  exit 1
fi

if ! grep -Fq "${boot_marker}" "${clean_log}"; then
  echo "smoke-usermode-riscv64: FAIL (missing boot marker)" >&2
  tail -n 180 "${clean_log}" >&2 || true
  exit 1
fi
if ! grep -Fq "${user_marker}" "${clean_log}"; then
  echo "smoke-usermode-riscv64: FAIL (missing user marker)" >&2
  tail -n 180 "${clean_log}" >&2 || true
  exit 1
fi
if ! grep -Fq "${probe_marker}" "${clean_log}"; then
  echo "smoke-usermode-riscv64: FAIL (missing probe marker)" >&2
  tail -n 180 "${clean_log}" >&2 || true
  exit 1
fi
if grep -Eq "interrupt: unhandled|exception: " "${clean_log}"; then
  echo "smoke-usermode-riscv64: FAIL (unexpected interrupt/exception)" >&2
  tail -n 200 "${clean_log}" >&2 || true
  exit 1
fi

echo "smoke-usermode-riscv64: PASS"
