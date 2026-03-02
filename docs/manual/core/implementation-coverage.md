# Implementation Coverage

This page tells readers where each implemented hardware substrate area is documented and where architecture behavior diverges.

## Boot Handoff and Entry

- Common model:
  - `core/boot-info-abi.md`
  - `core/boot-sequence.md`
  - `core/data-structures.md`
- Architecture paths:
  - `x86_64/boot.md`
  - `arm64/boot.md`
  - `riscv64/boot.md`

## CPU and Per-CPU Runtime

- Common API and behavior:
  - `core/cpu-layer.md`
  - `core/percpu.md`
  - `core/api-reference.md`
- Architecture backends:
  - `x86_64/api-reference.md`
  - `arm64/api-reference.md`
  - `riscv64/api-reference.md`

## Exceptions, Interrupt Dispatch, and IRQ Controllers

- Common stack:
  - `core/exceptions.md`
  - `core/interrupts-timers.md`
  - `core/irq-controller.md`
  - `core/api-reference.md`
- Architecture backends:
  - `x86_64/interrupts-timer.md`
  - `arm64/interrupts-timer.md`
  - `riscv64/interrupts-timer.md`
  - `x86_64/api-reference.md`
  - `arm64/api-reference.md`
  - `riscv64/api-reference.md`

## Time and Timer

- Common behavior:
  - `core/time-system.md`
  - `core/interrupts-timers.md`
  - `core/api-reference.md`
- Architecture details:
  - `x86_64/interrupts-timer.md`
  - `arm64/interrupts-timer.md`
  - `riscv64/interrupts-timer.md`

## Memory and MMU

- Common stack:
  - `core/memory.md`
  - `core/api-reference.md`
  - `core/data-structures.md`
- Architecture details:
  - `x86_64/memory.md`
  - `arm64/memory.md`
  - `riscv64/memory.md`
  - `x86_64/api-reference.md`
  - `arm64/api-reference.md`
  - `riscv64/api-reference.md`

## SMP Bring-up

- Common API/orchestration:
  - `core/smp-bootstrap.md`
  - `core/api-reference.md`
- Architecture behavior:
  - `x86_64/api-reference.md`
  - `arm64/api-reference.md`
  - `riscv64/api-reference.md`

## Device Discovery

- Common normalized model:
  - `core/device-discovery.md`
  - `core/data-structures.md`
- Architecture divergence and source availability:
  - `core/architecture-divergence.md`

## Device Model

- Common model and API:
  - `core/device-model.md`
  - `core/device-bus.md`
  - `core/pci-enumeration.md`
  - `core/usb-enumeration.md`
  - `core/device-domains.md`
  - `core/device-reporting.md`
  - `core/capability-profiles.md`
  - `core/network-baseline.md`
  - `core/audio-baseline.md`
  - `core/api-reference.md`
  - `core/data-structures.md`
- Boot integration:
  - `core/boot-sequence.md`

## Syscall Transport

- Common transport contract:
  - `core/syscall-transport.md`
  - `core/api-reference.md`
  - `core/data-structures.md`
- Architecture hooks:
  - `x86_64/api-reference.md`
  - `arm64/api-reference.md`
  - `riscv64/api-reference.md`

## Operational Configuration and Diagnostics

- Config and flags:
  - `core/config-flags.md`
  - `core/standard-capability-domains.md`
- Status/panic policy:
  - `core/status-system.md`
- Boot diagnostics output:
  - `core/boot-sequence.md`
  - `core/api-reference.md`
