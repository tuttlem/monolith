# x86_64 Interrupts and Timer

## Interrupt Backend

File: `arch/x86_64/irq/interrupts.c`

### Setup

- Builds IDT entries (`g_idt[256]`).
- Loads IDT with `lidt`.
- Installs:
  - exception stubs for vectors `0..31`
  - generic IRQ stub for most vectors
  - dedicated IRQ stub for vector `32` (timer)

### Dispatch

Stubs build an `interrupt_frame_t` and call `interrupts_dispatch`.

Current low-level stubs do not yet populate full machine-context fields (`ip/sp/flags` are currently zero-filled in these paths).

### Self-test exception trigger

`arch_exception_selftest_trigger()` executes `ud2`.

## Timer Backend

File: `arch/x86_64/timer/timer.c`

### Hardware path

- remaps legacy PIC to vectors `0x20..0x2F`
- unmasks IRQ0 only
- programs PIT channel 0 for periodic `100 Hz`

### Timer contract

- returns `out_hz = 100`
- returns `out_irq_vector = 32`
- `arch_timer_ack` sends EOI to PIC(s)
