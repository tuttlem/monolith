# 127 Network Baseline (Optional Domain)

## Goal

Provide a baseline network device substrate so OS writers can discover and initialize common NIC hardware without mandating a full network stack.

## Scope

- Enumerate NIC devices from bus graph (PCI first).
- Define generic net device descriptor:
  - MAC address
  - link status
  - queue/ring capability summary
  - MMIO/IRQ resources
- Implement one practical baseline backend first (recommended: `virtio-net`).
- Add basic in-kernel diagnostics:
  - device detected
  - link up/down
  - TX/RX queue init status

## Non-Goals

- Full TCP/IP stack.
- Userspace sockets ABI.

## Acceptance Criteria

- At least one NIC is discoverable on supported virtual platforms.
- Driver probe/init status is visible through device reporting.
- Domain can be disabled by feature gate without side effects.
