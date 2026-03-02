# 050 Exception and Interrupt Framework

## Goal

Deliver robust trap entry, metadata normalization, and generic dispatch policy.

## Required Generic Types

Create/extend types:
- `interrupt_frame_t` (existing)
- `exception_info_t` (new, normalized decode)

`exception_info_t` should include:
- architecture id
- vector/cause code
- class (fault/trap/abort/irq)
- fault address
- instruction pointer
- stack pointer
- machine flags/status
- optional architecture raw syndrome value

## Generic Dispatch Policy

1. architecture trap entry builds minimal frame
2. architecture shim enriches data if possible
3. generic dispatcher routes by vector table
4. unhandled exceptions call unified panic
5. unhandled IRQs can be logged once + counted

## Handler Registration Requirements

- keep owner-based registration model
- add priority model only if needed later (not required now)
- maintain deterministic one-handler-per-vector default

## Fault Decode Requirements

Per architecture, add cause decode tables:
- page fault variants
- illegal instruction
- breakpoint/trap
- general protection / access faults
- timer interrupt class

## Testing

- architecture self-test trigger for illegal instruction path
- verify decoded panic output includes cause string
- verify dispatch of registered timer handler across architectures with implemented timer backend

## Acceptance Criteria

- no fatal path directly loops/halt outside panic backend
- exceptions produce normalized structured metadata
- vector ownership rules remain enforced
