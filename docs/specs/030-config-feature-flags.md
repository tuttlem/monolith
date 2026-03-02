# 030 Configuration and Feature Flag Model

## Goal

Replace ad-hoc compile switches with a coherent, documented configuration surface.

## Required Structure

- central config root header, e.g. `kernel/include/config.h`
- per-domain config headers optional (`config_irq.h`, `config_mm.h`)
- architecture capability headers allowed, but only consumed by config layer

## Flag Classes

1. Build profile flags:
- debug
- release
- sanitizer/test modes

2. Bring-up diagnostics:
- boot info dump
- allocator self-tests
- timer self-test
- exception self-test

3. Substrate feature toggles:
- SMP enable
- MMU map API strict checks
- device discovery sources (ACPI/DT)

## Policy

- default config should boot cleanly on all supported architectures.
- debug flags should increase diagnostics but not change core correctness.
- avoid architecture branches in generic code for feature selection.

## Required Tooling

- one generated config summary in build output
- one runtime banner with key enabled diagnostics (optional)

## Acceptance Criteria

- all existing `MONOLITH_*` style toggles are centralized
- no raw feature `#ifdef` spread in unrelated modules
- documented default profile for CI and local bring-up

## Implementation Notes (Current Repository)

`030-config-feature-flags` is implemented by:
- adding `kernel/include/config.h` as the central config root
- moving existing `MONOLITH_*` defaults out of local source files
- wiring modules to consume shared config:
  - `kernel/kmain.c`
  - `kernel/interrupts.c`
  - `kernel/include/assert.h`
- adding `make print-config` to surface current default toggles

Default profile remains behavior-compatible with existing bring-up flow.
