# 123 USB Enumeration Skeleton

## Goal

Establish a USB device discovery baseline through host controller abstraction, sufficient to enumerate downstream USB devices.

## Scope

- Add USB bus model elements:
  - host controller device
  - root hub / ports
  - USB device descriptors
- Define host controller interface layer and implement xHCI-first skeleton.
- Enumerate connected USB devices (address, class, VID/PID where available).
- Publish discovered USB devices into the generic device graph.

## Non-Goals

- Full USB transfer stack for all endpoint types.
- Complete HID/storage class drivers in this phase.

## Acceptance Criteria

- USB host controller appears as a managed device.
- Connected devices are enumerated and printed with stable identity fields.
- Device model can match USB class/vendor/product descriptors.
