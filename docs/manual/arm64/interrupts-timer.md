# arm64 Interrupts and Timer

## Interrupt Backend

Files:
- `arch/arm64/irq/entry.S`
- `arch/arm64/irq/interrupts.c`

### Setup

- sets `VBAR_EL1` to `arm64_vector_table`.
- initializes GICv2 distributor + CPU interface (MMIO base addresses currently hardcoded for target platform).
- enables virtual timer PPI (INTID 27) at distributor level.

### Trap flow

Assembly entry passes trap metadata to `arm64_trap_c`.

`arm64_trap_c` behavior:
- IRQ traps: reads GICC_IAR, maps INTID -> vector (`32 + intid`), dispatches, writes EOIR.
- non-IRQ traps: maps to exception-class vectors and dispatches.
- fills `interrupt_frame_t` with ESR/FAR/ELR/SP/SPSR details.

### Self-test exception trigger

`arch_exception_selftest_trigger()` executes `brk #0`.

## Timer Backend

File: `arch/arm64/timer/timer.c`

### Hardware path

- reads counter frequency from `CNTFRQ_EL0`
- computes reload for target `100 Hz`
- programs virtual timer (`CNTV_TVAL_EL0`, `CNTV_CTL_EL0`)

### Timer contract

- `out_hz = 100`
- `out_irq_vector = 32 + 27 = 59`
- `arch_timer_ack` rearms timer by rewriting `CNTV_TVAL_EL0`
