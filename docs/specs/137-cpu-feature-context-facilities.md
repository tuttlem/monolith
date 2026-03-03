# 137 CPU Feature and Context Facilities

## Goal

Expose architecture-neutral CPU capability and context helper APIs needed for future process/task runtimes without defining scheduler policy.

## In Scope

- CPU capability query table (`simd`, `fp`, `virt`, `atomic`, timer features).
- Context save/restore helper contracts for trap/task switch code paths.
- User-return preparation hooks (register sanitization, flags policy hook points).
- Barrier/ordering helper consolidation.

## Out of Scope

- Full task scheduler policy.
- Userspace ABI policy.

## Public Interfaces

- Headers: `kernel/include/arch_cpu.h`, `kernel/include/cpu_caps.h`, `kernel/include/cpu_context.h`
- APIs:
  - `status_t cpu_caps_query(cpu_caps_t *out)`
  - `status_t cpu_context_init(cpu_context_t *ctx, void (*entry)(void *), void *arg, void *stack_top)`
  - `status_t cpu_context_switch(cpu_context_t *from, cpu_context_t *to)`

## Architecture Backends

- `x86_64`: CPUID-based capabilities, XSAVE/FXSAVE hooks as available.
- `arm64`: ID register-derived capabilities, FP/SIMD context hooks.
- `riscv64`: misa/senvcfg-dependent capability reporting and context hooks.

## Tests

- capability sanity tests.
- context init/swap smoke in kernel-only harness.

## Acceptance Criteria

1. Generic code can query capabilities without architecture conditionals.
2. Context helpers exist and validate structures even before full scheduler integration.
