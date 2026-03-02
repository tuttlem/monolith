# Standard Capability Domains

This page defines the baseline capability contract that downstream OS writers can rely on.

## Domain Sets

Core always-on domains:
- CPU
- MMU
- interrupts
- timer/timebase
- hardware discovery
- device reporting

Optional domains (feature-gated):
- storage (`DEVICE_CLASS_BLOCK`)
- network (`DEVICE_CLASS_NET`)
- input (`DEVICE_CLASS_INPUT`)
- display (`DEVICE_CLASS_DISPLAY`)
- audio (`DEVICE_CLASS_AUDIO`)

## Feature Gates

Optional domain gates in `kernel/include/config.h`:

- `MONOLITH_CAP_STORAGE`
- `MONOLITH_CAP_NETWORK`
- `MONOLITH_CAP_INPUT`
- `MONOLITH_CAP_DISPLAY`
- `MONOLITH_CAP_AUDIO`

Profiles:
- `MONOLITH_CAP_PROFILE_MINIMAL`
- `MONOLITH_CAP_PROFILE_HEADLESS`
- `MONOLITH_CAP_PROFILE_DESKTOP`

## Runtime Contract API

Header: `kernel/include/capability_profile.h`

- `const char *capability_profile_name(void)`
- `int capability_domain_enabled(device_class_t class_id)`
- `const char *capability_domain_name(device_class_t class_id)`
- `status_t capability_domain_state(device_class_t class_id)`
- `void capability_domain_dump_matrix(void)`

`capability_domain_state()` returns:
- `STATUS_OK`: domain enabled
- `STATUS_DEFERRED`: domain intentionally disabled

## Profile Matrix

| Domain  | minimal | headless | desktop |
|---------|---------|----------|---------|
| storage | off     | on       | on      |
| network | off     | on       | on      |
| input   | off     | off      | on      |
| display | off     | off      | on      |
| audio   | off     | off      | on      |

## Init Order Position

Optional domains currently initialize in `kmain` after bus-level enumeration:

1. `device_domains_enumerate()` for block/input/display classification
2. `net_enumerate()` for network classification
3. `audio_enumerate()` for audio classification

## Disabled Behavior

When an optional domain gate is disabled:
- domain init returns `STATUS_DEFERRED`
- no class nodes are registered for that domain
- unrelated domains and core bring-up continue normally

## Test Expectations

For each valid profile build:
- kernel builds on `x86_64`, `arm64`, `riscv64`
- smoke test passes on all supported run targets
- boot log prints profile + matrix
- disabled domains report deferred status and zero discovered devices
