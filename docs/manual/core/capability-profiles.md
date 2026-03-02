# Capability Profiles and Feature Gating

Capability profiles define which optional device domains are enabled for a kernel build.

## Profiles

Configured by `MONOLITH_CAP_PROFILE` in `kernel/include/config.h`:

- `MONOLITH_CAP_PROFILE_MINIMAL`
  - core hardware substrate only
- `MONOLITH_CAP_PROFILE_HEADLESS` (default)
  - minimal + storage/network-oriented defaults
- `MONOLITH_CAP_PROFILE_DESKTOP`
  - headless + input/display/audio defaults

## Domain Gates

Per-domain gates (override profile defaults if needed):

- `MONOLITH_CAP_STORAGE`
- `MONOLITH_CAP_NETWORK`
- `MONOLITH_CAP_INPUT`
- `MONOLITH_CAP_DISPLAY`
- `MONOLITH_CAP_AUDIO`

## Runtime API

Header: `kernel/include/capability_profile.h`

- `const char *capability_profile_name(void)`
- `int capability_domain_enabled(device_class_t class_id)`
- `const char *capability_domain_name(device_class_t class_id)`
- `status_t capability_domain_state(device_class_t class_id)`
- `void capability_domain_dump_matrix(void)`
- `void capability_profile_print(void)`

## Current Integration

- `kmain` prints the active profile and domain gates at boot.
- `kmain` prints a standardized capability matrix at boot.
- `device_domains_enumerate()` respects gating for block/input/display classification.
- Additional domain subsystems (network/audio) use the same gating model.
- Full contract and matrix are documented in `core/standard-capability-domains.md`.
