# Interrupt Controller Layer (Spec 060)

API: `kernel/include/irq_controller.h`  
Core implementation: `kernel/irq_controller.c`

## Purpose

This layer separates generic interrupt policy from hardware controller details.

Generic code uses this API to:
- enable/disable IRQ lines
- acknowledge/end IRQ handling
- translate IRQ numbers to vectors and back

## Controller Operations

`irq_controller_ops_t` includes:
- `enable_irq`
- `disable_irq`
- `ack_irq`
- `eoi_irq`
- `map_irq`
- `vector_to_irq`

Controller lifecycle:
1. `irq_controller_reset()`
2. architecture backend calls `irq_controller_register(name, &ops)`
3. generic subsystems use `irq_controller_*` calls

If no controller is registered:
- map/enable/disable calls return `STATUS_DEFERRED`

## Architecture Backends

- x86_64 PIC: `arch/x86_64/irq/pic.c`
- arm64 GICv2: `arch/arm64/irq/gicv2.c`
- riscv64 controller stub: `arch/riscv64/irq/controller_stub.c`

Timer bring-up uses this layer for vector<->IRQ translation and IRQ enable.
