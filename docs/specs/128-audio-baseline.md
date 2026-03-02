# 128 Audio Baseline (Optional Domain)

## Goal

Add a minimal audio device discovery and initialization path suitable for OS experimentation, while remaining optional.

## Scope

- Enumerate audio-class devices from bus graph.
- Define generic audio device descriptor:
  - output/input capabilities
  - sample formats/rates summary
  - interrupt and DMA resources
- Implement one baseline virtualized backend where practical.
- Add diagnostic path for init and capability summary.

## Non-Goals

- Full mixer policy.
- End-user audio framework and UX policy.

## Acceptance Criteria

- Audio devices appear in device reporting when enabled.
- Audio domain can be disabled with no impact on unrelated domains.
