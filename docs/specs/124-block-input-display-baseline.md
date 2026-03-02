# 124 Block, Input, and Display Baseline Drivers

## Goal

Provide practical baseline external device support paths needed by OS writers to bootstrap storage/input/display experiments.

## Scope

- Block:
  - baseline block device abstraction
  - at least one concrete backend (prefer `virtio-blk` first)
- Input:
  - baseline input event abstraction
  - keyboard path (PS/2 on x86_64 and/or USB HID path where available)
- Display:
  - promote framebuffer into managed display device node
  - expose basic mode metadata and linear buffer mapping descriptor

## Non-Goals

- Full graphics stack, compositor, accelerated GPU drivers.
- Advanced input gesture/IME policy.

## Acceptance Criteria

- Kernel logs show discovered block/input/display devices as first-class managed devices.
- Block device can be opened and basic read path sanity-checked in-kernel test mode.
- Keyboard input events can be observed in diagnostics path.
