# 139 Driver Helper Kits (Block/Net/Common Queues)

## Goal

Provide reusable, policy-neutral helper libraries that remove repeated low-level boilerplate in storage/network/device drivers.

## In Scope

- Ring/queue helper library:
  - descriptor ring management
  - producer/consumer indexing
  - barriers
- Interrupt completion helper.
- Timeout/retry utility hooks.
- Capability negotiation helper for transport-style devices (e.g., virtio-like patterns).

## Out of Scope

- Device-specific protocol policy.
- Filesystem/network stack policy.

## Public Interfaces

- Headers:
  - `kernel/include/driver_queue.h`
  - `kernel/include/driver_irq_helper.h`
  - `kernel/include/driver_retry.h`

## Tests

- unit tests for ring wraparound, full/empty transitions.
- block/net baseline drivers migrated to helper APIs.

## Acceptance Criteria

1. At least one block and one net path use queue helpers.
2. No duplicated ad-hoc ring logic in multiple drivers.
