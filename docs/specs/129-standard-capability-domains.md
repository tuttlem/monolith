# 129 Standard Capability Domains for OS Writers

## Goal

Define the "standard pieces" expected from this base so downstream OS developers know what they can rely on.

## Standard Domains

- Core always-on domains:
  - CPU, MMU, interrupts, timer, discovery, device reporting
- Common optional domains:
  - storage
  - network
  - input
  - display
  - audio

## Contract

For each domain, document:
- feature gate name
- required core dependencies
- expected device classes and descriptors
- init order position
- disabled behavior

## Developer Guidance

- OS writers should choose a capability profile first.
- Then enable only needed domains.
- The base must remain stable and deterministic across any valid profile.

## Acceptance Criteria

- Manual contains one matrix mapping domains to profiles.
- All domains have explicit enable/disable behavior and test expectations.
