# Driver Helper Kits

This layer provides reusable helper primitives for driver code paths.

## Purpose

- Remove repeated queue/IRQ/retry boilerplate in transport drivers.
- Keep helpers generic and policy-neutral.
- Provide stable utility APIs for future block/net/audio backends.

## Public API

Headers:
- `kernel/include/driver_queue.h`
- `kernel/include/driver_irq_helper.h`
- `kernel/include/driver_retry.h`

Key helpers:
- `driver_ring_init`, `driver_ring_push`, `driver_ring_pop`
- `driver_irq_complete`
- `driver_retry_begin`, `driver_retry_next`

## Current Integration

- Net discovery path uses `driver_ring_t` as a neutral probe queue guard in `kernel/net.c`.
- Block domain discovery path uses `driver_ring_t` in `kernel/device_domains.c`.
- The helpers stay policy-neutral; OS-specific protocol ownership remains above this layer.
