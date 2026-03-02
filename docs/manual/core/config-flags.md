# Config and Feature Flags (Spec 030)

Central configuration lives in `kernel/include/config.h`.

## Why It Exists

- keeps feature toggles out of random source files
- prevents ad-hoc `#ifdef` growth
- provides one consistent bring-up profile across architectures

## API Version

- `MONOLITH_CONFIG_API_VERSION_MAJOR`
- `MONOLITH_CONFIG_API_VERSION_MINOR`

## Flag Classes

Diagnostics:
- `MONOLITH_BOOTINFO_DEBUG`
- `MONOLITH_IRQ_LOG_UNHANDLED`

Self-tests:
- `MONOLITH_KMALLOC_SELFTEST`
- `MONOLITH_KMALLOC_DEBUG_EXERCISE`
- `MONOLITH_TIMER_SELFTEST`
- `MONOLITH_TIMER_SELFTEST_SPINS`
- `MONOLITH_EXCEPTION_SELFTEST`

Assert policy:
- `MONOLITH_ASSERT_ENABLE`
- `MONOLITH_ASSERT_PANIC`

Feature toggles:
- `MONOLITH_FEATURE_SMP`
- `MONOLITH_FEATURE_MMU_STRICT`
- `MONOLITH_FEATURE_DISCOVERY_ACPI`
- `MONOLITH_FEATURE_DISCOVERY_DTB`

Capability profile and domain gates:
- `MONOLITH_CAP_PROFILE_MINIMAL`
- `MONOLITH_CAP_PROFILE_HEADLESS`
- `MONOLITH_CAP_PROFILE_DESKTOP`
- `MONOLITH_CAP_PROFILE`
- `MONOLITH_CAP_STORAGE`
- `MONOLITH_CAP_NETWORK`
- `MONOLITH_CAP_INPUT`
- `MONOLITH_CAP_DISPLAY`
- `MONOLITH_CAP_AUDIO`

## Operational Rule

Default settings should boot cleanly across x86_64, arm64, and riscv64.

Use `make print-config` to print active defaults.
