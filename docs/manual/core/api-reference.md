# Core API Reference

This page documents the stable, public kernel interfaces in `kernel/include`.

## Conventions

- Return type `status_t`:
  - `STATUS_OK (0)` means success.
  - Positive values such as `STATUS_DEFERRED` are non-fatal state signals.
  - Negative values are failures.
- Pointers are raw early-kernel pointers; no ownership transfer unless explicitly stated.
- Most init functions are intended to be called once from `kmain` in bring-up order.

## Kernel Entry and Console (`kernel.h`, `print.h`)

### `void kmain(const boot_info_t *boot_info)`
- Purpose: top-level generic kernel bring-up entry.
- Parameters:
  - `boot_info`: immutable handoff structure populated by architecture boot code.
- Returns: none.
- Use when: architecture boot code has switched to C and must transfer control to generic kernel startup.
- Example:
```c
extern void kmain(const boot_info_t *boot_info);

/* in arch entry */
kmain(&boot_info);
```

### `int kprintf(const char *fmt, ...)`
- Purpose: formatted console output through `arch_puts`.
- Parameters:
  - `fmt`: printf-style format string.
  - variadic arguments: values consumed by format specifiers.
- Returns: number of characters emitted.
- Use when: early diagnostics and bring-up logs are needed.
- Example:
```c
kprintf("smp: possible=%llu online=%llu\n", possible, online);
```

### `int ksnprintf(char *buf, unsigned long size, const char *fmt, ...)`
### `int kvsnprintf(char *buf, unsigned long size, const char *fmt, va_list args)`
- Purpose: bounded formatting into caller-provided buffers.
- Parameters:
  - `buf`: output destination.
  - `size`: destination capacity in bytes, including trailing `\0`.
  - `fmt`: format string.
  - `args`: varargs list for `kvsnprintf`.
- Returns: number of characters that would have been written (excluding `\0`).
- Use when: prepare formatted strings before log routing.
- Example:
```c
char line[128];
ksnprintf(line, sizeof(line), "cpu=%llu", arch_cpu_id());
```

### `void arch_puts(const char *s)`
### `void arch_halt(void)`
### `void arch_panic_stop(void)`
### `void arch_exception_selftest_trigger(void)`
- Purpose: architecture-provided primitives used by generic core.
- Parameters:
  - `s`: null-terminated string for `arch_puts`.
- Returns: none.
- Use when: generic core needs console output, final halt path, or exception self-test trigger.

## Status and Panic (`status.h`, `panic.h`, `assert.h`)

### `const char *status_str(status_t status)`
- Purpose: convert status code to readable constant string.
- Parameters:
  - `status`: status value.
- Returns: static string name.
- Use when: diagnostics should print symbolic status.
- Example:
```c
status_t st = page_alloc_init(info);
kprintf("page_alloc_init=%s\n", status_str(st));
```

### `void panic(const char *reason)`
### `void panicf(const char *fmt, ...)`
- Purpose: unrecoverable failure path with terminal diagnostics.
- Parameters:
  - `reason` or `fmt` + variadic args: panic message.
- Returns: none; does not return.
- Use when: kernel cannot safely continue.

### `void panic_from_exception(const exception_info_t *info)`
- Purpose: unified panic path for decoded exceptions.
- Parameters:
  - `info`: exception metadata.
- Returns: none.
- Use when: trap dispatch determines exception is unhandled.

### `void panic_set_context(const boot_info_t *boot_info)`
- Purpose: provide boot context used in panic reporting.
- Parameters:
  - `boot_info`: handoff structure pointer.
- Returns: none.

### `ASSERT(expr)`
- Purpose: policy-controlled assertion (`panicf` when enabled).
- Parameters:
  - `expr`: boolean expression.
- Use when: invariant must hold during bring-up.

## CPU HAL (`arch_cpu.h`)

### `status_t arch_cpu_early_init(const boot_info_t *boot_info)`
### `status_t arch_cpu_late_init(void)`
- Purpose: initialize per-architecture CPU layer state in two phases.
- Parameters:
  - `boot_info`: boot handoff pointer for early phase.
- Returns:
  - `STATUS_OK` on success.
  - `STATUS_INVALID_ARG` for bad inputs.
- Use when: `kmain` initializes CPU substrate before higher-level services.

### `BOOT_U64 arch_cpu_id(void)`
### `BOOT_U64 arch_cpu_count_hint(void)`
- Purpose: query boot CPU ID and an architecture-local CPU count hint.
- Returns:
  - numeric CPU id or count hint (may be `1` if unknown).
- Use when: initializing per-CPU tables and fallback SMP sizing.

### `void arch_cpu_relax(void)`
### `void arch_cpu_halt(void)`
### `void arch_cpu_reboot(void)`
- Purpose: low-level wait/halt/reset primitives.
- Use when: spin-wait loops, idle loops, or reboot path.

### `BOOT_U64 arch_cycle_counter(void)`
- Purpose: read an architecture cycle counter (best-effort monotonic source).
- Returns: cycle count.

### `status_t arch_cpu_set_local_base(BOOT_U64 base)`
### `BOOT_U64 arch_cpu_get_local_base(void)`
- Purpose: set/get architecture register used for CPU-local base.
- Parameters:
  - `base`: per-CPU base address to publish.
- Returns:
  - set: `STATUS_OK` or error.
  - get: current base register value.
- Use when: `percpu` publishes/retrieves current CPU record.

### `void arch_barrier_full(void)`
### `void arch_barrier_read(void)`
### `void arch_barrier_write(void)`
### `void arch_tlb_sync_local(void)`
### `void arch_icache_sync_range(BOOT_U64 addr, BOOT_U64 size)`
- Purpose: architecture ordering and cache/TLB synchronization operations.
- Parameters:
  - `addr`, `size`: target range for i-cache sync (backend may sync globally).
- Use when: page table and executable memory changes require ordering.

## IRQ HAL and Generic Interrupt Layer (`arch_irq.h`, `interrupts.h`)

### `status_t arch_interrupts_init(const boot_info_t *boot_info)`
### `void arch_interrupts_enable(void)`
### `void arch_interrupts_disable(void)`
- Purpose: architecture trap hardware init and global interrupt mask control.
- Parameters:
  - `boot_info`: platform state for backend initialization.
- Returns: status from backend init.

### `status_t interrupts_init(const boot_info_t *boot_info)`
- Purpose: initialize generic interrupt dispatch and backend trap layer.
- Parameters:
  - `boot_info`: handoff data.
- Returns: status.

### `status_t interrupts_register_handler(BOOT_U64 vector, interrupt_handler_t handler, void *ctx)`
### `status_t interrupts_register_handler_owned(BOOT_U64 vector, interrupt_handler_t handler, void *ctx, const char *owner)`
- Purpose: register interrupt vector handlers.
- Parameters:
  - `vector`: interrupt vector index (`< INTERRUPT_MAX_VECTORS`).
  - `handler`: callback invoked on dispatch.
  - `ctx`: opaque pointer passed to callback.
  - `owner`: ownership token for controlled replacement/unregister.
- Returns: status.
- Use when: timer/device IRQ handlers are installed.
- Example:
```c
static void tick_handler(const interrupt_frame_t *f, void *ctx) { (void)f; (void)ctx; }
interrupts_register_handler_owned(32, tick_handler, 0, "timer");
```

### `status_t interrupts_unregister_handler(BOOT_U64 vector, const char *owner)`
### `const char *interrupts_handler_owner(BOOT_U64 vector)`
- Purpose: remove/query vector ownership.
- Parameters:
  - `vector`: vector index.
  - `owner`: expected owner string for safe unregister.
- Returns: status or owner string.

### `void interrupts_dispatch(const interrupt_frame_t *frame)`
### `void interrupts_enable(void)`
### `void interrupts_disable(void)`
- Purpose: generic dispatch entry and generic wrappers for mask control.
- Parameters:
  - `frame`: architecture trap frame translated to generic layout.

## IRQ Controller Layer (`irq_controller.h`)

### `void irq_controller_reset(void)`
- Purpose: clear registered controller backend.

### `status_t irq_controller_register(const char *name, const irq_controller_ops_t *ops)`
- Purpose: bind active interrupt-controller ops implementation.
- Parameters:
  - `name`: controller display name.
  - `ops`: callback table.
- Returns: status.

### `const char *irq_controller_name(void)`
- Purpose: query active controller name.

### `status_t irq_controller_enable(BOOT_U64 irq)`
### `status_t irq_controller_disable(BOOT_U64 irq)`
### `void irq_controller_ack(BOOT_U64 irq)`
### `void irq_controller_eoi(BOOT_U64 irq)`
### `status_t irq_controller_map(BOOT_U64 irq, BOOT_U64 *out_vector)`
### `status_t irq_controller_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq)`
- Purpose: generic IRQ operations routed to backend controller.
- Parameters:
  - `irq`: hardware IRQ number.
  - `vector`: CPU vector.
  - `out_vector`, `out_irq`: output mappings.
- Returns: status for operations returning `status_t`.

## Timer and Timebase (`arch_timer.h`, `timer.h`, `timebase.h`)

### `status_t arch_timer_init(const boot_info_t *boot_info, BOOT_U64 *out_hz, BOOT_U64 *out_irq_vector)`
### `void arch_timer_ack(BOOT_U64 vector)`
### `BOOT_U64 arch_timer_clocksource_hz(const boot_info_t *boot_info)`
- Purpose: architecture timer backend initialization and acknowledgment.
- Parameters:
  - `boot_info`: handoff data.
  - `out_hz`: programmed periodic rate.
  - `out_irq_vector`: vector used by periodic timer IRQ.
  - `vector`: delivered interrupt vector for ack path.
- Returns: status or discovered frequency.

### `status_t timer_init(const boot_info_t *boot_info)`
### `BOOT_U64 timer_ticks(void)`
### `BOOT_U64 timer_hz(void)`
- Purpose: compatibility timer facade backed by timebase system.

### `status_t time_init(const boot_info_t *boot_info)`
### `BOOT_U64 time_now_ns(void)`
### `BOOT_U64 time_ticks(void)`
### `BOOT_U64 time_hz(void)`
### `const clocksource_t *time_clocksource(void)`
### `const clockevent_t *time_clockevent(void)`
- Purpose: monotonic time initialization/query API.
- Parameters:
  - `boot_info`: handoff pointer during init.
- Returns:
  - init status.
  - current monotonic nanoseconds/ticks/hz or backend descriptors.
- Use when: scheduling, timeout accounting, and profiling need monotonic time.
- Example:
```c
status_t st = time_init(info);
if (status_is_ok(st)) {
  kprintf("t=%llu ns\n", time_now_ns());
}
```

## Memory Initialization and MMU API (`memory_init.h`, `arch_mm.h`)

### `status_t arch_memory_init(boot_info_t *boot_info)`
### `static inline status_t arch_mm_early_init(boot_info_t *boot_info)`
- Purpose: architecture early memory takeover and baseline mapping setup.
- Parameters:
  - `boot_info`: mutable handoff for backend result publication.
- Returns: status.

### `status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, BOOT_U64 size, BOOT_U64 prot_flags)`
- Purpose: map `size` bytes from virtual `va` to physical `pa` using backend page granule.
- Parameters:
  - `va`: virtual base.
  - `pa`: physical base.
  - `size`: bytes to map; must be non-zero and page-aligned.
  - `prot_flags`: bitwise OR of `mmu_prot_t` flags.
- Returns: status.
- Remarks: rolls back pages mapped in the same call on partial failure.
- Example:
```c
mm_map(0xffff800000200000ULL, 0x00200000ULL, mm_page_size(), MMU_PROT_READ | MMU_PROT_WRITE);
```

### `status_t mm_unmap(mm_virt_addr_t va, BOOT_U64 size)`
### `status_t mm_protect(mm_virt_addr_t va, BOOT_U64 size, BOOT_U64 prot_flags)`
### `status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, BOOT_U64 *out_flags)`
### `status_t mm_sync_tlb(mm_virt_addr_t va, BOOT_U64 size)`
### `BOOT_U64 mm_page_size(void)`
- Purpose: generic unmap/protect/translate/TLB sync/page-size query.
- Parameters:
  - `va`, `size`: target range.
  - `out_pa`, `out_flags`: translation outputs.
  - `prot_flags`: new protection flags.
- Returns: status or page size.

### Architecture MM backend entry points
- `BOOT_U64 arch_mm_page_size(void)`
- `status_t arch_mm_map_page(mm_virt_addr_t va, mm_phys_addr_t pa, BOOT_U64 prot_flags)`
- `status_t arch_mm_unmap_page(mm_virt_addr_t va)`
- `status_t arch_mm_protect_page(mm_virt_addr_t va, BOOT_U64 prot_flags)`
- `status_t arch_mm_translate_page(mm_virt_addr_t va, mm_phys_addr_t *out_pa, BOOT_U64 *out_flags)`
- `status_t arch_mm_sync_tlb(mm_virt_addr_t va, BOOT_U64 size)`

These are implemented per architecture and consumed by generic `mm_*` APIs.

## Physical Page and Heap Allocators (`page_alloc.h`, `kmalloc.h`)

### `status_t page_alloc_init(boot_info_t *boot_info)`
- Purpose: initialize physical page free list from handoff memory regions.
- Parameters:
  - `boot_info`: mutable boot data used for region scanning.
- Returns: status.

### `BOOT_U64 alloc_page(void)`
### `void free_page(BOOT_U64 phys_addr)`
### `void page_alloc_stats(page_alloc_stats_t *out_stats)`
- Purpose: allocate/free single physical pages and query allocator stats.
- Parameters:
  - `phys_addr`: page-aligned physical address to return.
  - `out_stats`: output stats pointer.
- Returns:
  - `alloc_page`: physical address or `0` on exhaustion.
- Example:
```c
BOOT_U64 p = alloc_page();
if (p != 0) {
  free_page(p);
}
```

### `status_t kmalloc_init(boot_info_t *boot_info)`
### `void *kmalloc(BOOT_U64 size)`
### `void kfree(void *ptr)`
### `void kmalloc_stats(kmalloc_stats_t *out_stats)`
### `int kmalloc_self_test(void)`
- Purpose: initialize and use kernel heap allocator.
- Parameters:
  - `size`: requested bytes.
  - `ptr`: pointer previously returned by `kmalloc`.
  - `out_stats`: output stats structure.
- Returns:
  - init status.
  - `kmalloc`: pointer or `0` on failure.
  - self-test result (`0` pass, non-zero failure).
- Example:
```c
void *buf = kmalloc(256);
if (buf != 0) {
  /* use buffer */
  kfree(buf);
}
```

## Per-CPU Runtime and SMP (`percpu.h`, `arch_smp.h`, `smp.h`)

### `status_t percpu_init_boot_cpu(const boot_info_t *boot_info)`
### `status_t percpu_register_current_cpu(BOOT_U64 cpu_id)`
### `percpu_t *percpu_current(void)`
### `percpu_t *percpu_by_id(BOOT_U64 cpu_id)`
### `BOOT_U64 percpu_online_count(void)`
- Purpose: initialize/register and query per-CPU records.
- Parameters:
  - `boot_info`: boot handoff for initial CPU identity.
  - `cpu_id`: logical CPU identifier.
- Returns: status, pointer, or count.
- Use when: per-CPU ownership and CPU-local state are needed.

### `status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus)`
- Purpose: architecture backend for secondary CPU bring-up.
- Parameters:
  - `boot_info`: handoff pointer.
  - `out_possible_cpus`: discovered total CPUs.
  - `out_started_cpus`: successfully started AP CPUs.
- Returns:
  - `STATUS_OK`, `STATUS_DEFERRED`, or failure.

### `status_t smp_init(const boot_info_t *boot_info)`
### `BOOT_U64 smp_cpu_count_online(void)`
### `BOOT_U64 smp_cpu_count_possible(void)`
### `void smp_secondary_entry(BOOT_U64 cpu_id)`
- Purpose: generic SMP orchestration and secondary entry.
- Parameters:
  - `boot_info`: handoff pointer.
  - `cpu_id`: CPU ID for secondary entry registration.
- Returns: status or counts.
- Remarks: current secondary policy is register-online then park.

## Hardware Discovery (`hw_desc.h`)

### `status_t hw_discovery_init(const boot_info_t *boot_info)`
### `const hw_desc_t *hw_desc_get(void)`
### `BOOT_U64 hw_desc_cpu_count_hint(void)`
- Purpose: normalize ACPI/DTB/fallback hardware data into `hw_desc_t`.
- Parameters:
  - `boot_info`: handoff context with ACPI/DTB pointers.
- Returns: status, descriptor pointer, or CPU count hint.
- Use when: generic subsystems need platform inventory without firmware format coupling.
- Example:
```c
const hw_desc_t *d = hw_desc_get();
if (d != 0) {
  kprintf("hw cpus=%llu timers=%llu\n", d->cpu_count, d->timer_count);
}
```

## Device Bus (`device_bus.h`)

### `status_t device_bus_init(const boot_info_t *boot_info, const hw_desc_t *hw)`
- Purpose: initialize the bus/device graph from normalized hardware discovery data.
- Parameters:
  - `boot_info`: handoff context.
  - `hw`: normalized hardware descriptor (`hw_desc_t`).
- Returns: status.

### `void device_bus_reset(void)`
- Purpose: clear all buses/devices and reset bus graph state.

### `status_t device_bus_register_bus(const bus_t *bus_template, BOOT_U64 *out_bus_id)`
### `status_t device_bus_register_device(const device_t *dev_template, BOOT_U64 *out_device_id)`
- Purpose: append bus or device nodes to graph.
- Parameters:
  - `bus_template` / `dev_template`: source descriptor.
  - `out_bus_id` / `out_device_id`: optional output assigned ID.

### `const bus_t *device_bus_get_bus(BOOT_U64 bus_id)`
### `const device_t *device_bus_get_device(BOOT_U64 device_id)`
### `BOOT_U64 device_bus_count(void)`
### `const device_t *device_bus_device_at(BOOT_U64 index)`
- Purpose: query bus/device entries.

### `BOOT_U64 device_bus_find_first_by_class(device_class_t class_id)`
### `BOOT_U64 device_bus_find_next_by_class(device_class_t class_id, BOOT_U64 after_id)`
- Purpose: iterate devices by class.

### `void device_bus_dump(void)`
- Purpose: print full bus/device inventory for diagnostics.

## PCI (`pci.h`)

### `status_t pci_enumerate(const boot_info_t *boot_info)`
- Purpose: discover PCI devices and add them into bus graph.
- Parameters:
  - `boot_info`: handoff context, used for architecture selection.
- Returns:
  - `STATUS_OK` when enumeration backend ran successfully.
  - `STATUS_DEFERRED` when backend is not implemented on this architecture.

### `BOOT_U64 pci_device_count(void)`
- Purpose: report number of PCI functions discovered in last enumeration pass.

## Device Model (`device_model.h`)

### `status_t driver_set_boot_info(const boot_info_t *boot_info)`
- Purpose: bind the active boot handoff used by driver init callbacks.
- Parameters:
  - `boot_info`: handoff pointer from architecture entry.
- Returns: status.

### `void driver_registry_reset(void)`
- Purpose: clear static driver registry and class-status tracking.
- Returns: none.

### `status_t driver_register(const driver_t *drv)`
- Purpose: register one static driver descriptor.
- Parameters:
  - `drv`: immutable driver descriptor.
- Returns: status.

### `status_t driver_probe_all(const hw_desc_t *hw)`
- Purpose: probe/init all drivers in deterministic class order (`irqc` -> `timer` -> `console` -> `early`).
- Parameters:
  - `hw`: normalized hardware discovery descriptor.
- Returns: status.

### `status_t driver_class_last_status(const char *class_name)`
- Purpose: query last status for a class.
- Parameters:
  - `class_name`: class name string.
- Returns: status value for class, or `STATUS_NOT_FOUND`.

### `status_t device_model_register_builtin_drivers(void)`
- Purpose: register built-in baseline driver hooks for IRQ, timer, and early console.
- Returns: status.
- Example:
```c
driver_registry_reset();
driver_set_boot_info(boot_info);
device_model_register_builtin_drivers();
driver_probe_all(hw_desc_get());
```

## Boot Diagnostics (`diag/boot_info.h`)

### `void diag_boot_info_print(const boot_info_t *boot_info)`
- Purpose: print complete boot handoff and architecture extension fields.
- Parameters:
  - `boot_info`: handoff structure.
- Returns: none.
- Use when: validating boot contract and architecture fill paths.

## Architecture-Specific Public APIs

These symbols are public and documented in architecture manual references:
- x86_64: `x86_64_early_paging_takeover`, `x86_64_pic_controller_init`
- arm64: `arm64_early_paging_takeover`, `arm64_gicv2_controller_init`, `arm64_gicv2_claim_irq`, `arm64_gicv2_eoi_irq`
- riscv64: `riscv64_early_paging_takeover`, `riscv64_irq_controller_init`

See:
- `docs/manual/x86_64/api-reference.md`
- `docs/manual/arm64/api-reference.md`
- `docs/manual/riscv64/api-reference.md`
