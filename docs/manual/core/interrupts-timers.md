# Interrupts and Timers

## Interrupt Core

API: `kernel/include/interrupts.h`
Implementation: `kernel/interrupts.c`

### Core concepts

- vector space: 0..255 (`INTERRUPT_MAX_VECTORS`)
- per-vector handler slot: function + context + owner string
- architecture backend installs low-level trap/IRQ entry logic
- core dispatcher applies policy and routes to registered handlers
- dispatcher updates per-CPU IRQ nesting (`percpu_current()->irq_nesting`)

### Registration API

- `interrupts_register_handler(vector, fn, ctx)`
- `interrupts_register_handler_owned(vector, fn, ctx, owner)`
- `interrupts_unregister_handler(vector, owner)`
- `interrupts_handler_owner(vector)`

Ownership prevents accidental handler takeover: if a vector already has a different owner, registration returns `STATUS_BUSY`.

### Dispatch policy

- vectors `< 32`: treated as exceptions
  - unhandled exceptions panic/halt
- vectors `>= 32`: external interrupts
  - unhandled IRQs log once per vector (suppresses repeated spam)

## Timer Core

API: `kernel/include/timer.h`
Implementation: `kernel/timer.c`

### Flow

1. `timer_init(boot_info)` calls `arch_timer_init(boot_info, &hz, &vector)`.
2. If arch returns `STATUS_OK`, timer core registers IRQ handler for that vector.
3. Handler increments:
   - per-CPU local tick counter (`percpu_current()->local_tick_count`)
   - global time tick counter
   then calls `arch_timer_ack(vector)`.
4. Timer init enables interrupts via `interrupts_enable()`.

### Public API

- `timer_init(const boot_info_t *boot_info)`
- `timer_ticks(void)`
- `timer_hz(void)`

## Architecture Backend Interfaces

### Interrupt backend

From `kernel/include/arch_interrupts.h`:
- `arch_interrupts_init`
- `arch_interrupts_enable`
- `arch_interrupts_disable`

### Timer backend

From `kernel/include/arch_timer.h`:
- `arch_timer_init`
- `arch_timer_ack`

These backend hooks are where platform-specific controller logic lives (IDT/PIC/GIC/CSR programming).
