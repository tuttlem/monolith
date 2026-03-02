# Core Data Structures and Enums

This page documents the key structures, typedefs, and enum/flag sets used by the kernel substrate.

## Scalar Types (`boot_info.h`)

### `BOOT_U64`, `BOOT_U32`, `BOOT_UPTR`
- Purpose: ABI-stable integer/pointer-width types used in boot and architecture-neutral contracts.
- Notes:
  - `BOOT_UPTR` tracks pointer width at compile time.
  - Keep these types at all public boundaries to avoid ABI drift across architectures.

## Boot ABI Constants (`boot_info.h`)

### `BOOT_INFO_ABI_VERSION`
- Purpose: handoff ABI version (`2` at current state).

### `BOOT_INFO_ARCH_*`
- `BOOT_INFO_ARCH_UNKNOWN`
- `BOOT_INFO_ARCH_X86_64`
- `BOOT_INFO_ARCH_ARM64`
- `BOOT_INFO_ARCH_RISCV64`
- Purpose: architecture ID values for runtime dispatch and diagnostics.

### `BOOT_INFO_HAS_*` bit flags
- Purpose: `boot_info_t.valid_mask` feature/presence bits.
- Key bits:
  - `BOOT_INFO_HAS_ENTRY_STATE`: `entry_pc`, `entry_sp` valid.
  - `BOOT_INFO_HAS_VM_STATE`: `vm_enabled`, `vm_root_table` valid.
  - `BOOT_INFO_HAS_UEFI_SYSTEM_TABLE` / `BOOT_INFO_HAS_UEFI_CONFIG_TABLE`.
  - `BOOT_INFO_HAS_MEMMAP`: UEFI memory map stream fields valid.
  - `BOOT_INFO_HAS_ACPI_RSDP`, `BOOT_INFO_HAS_DTB`.
  - `BOOT_INFO_HAS_BOOT_CPU_ID`, `BOOT_INFO_HAS_ARCH_DATA`, `BOOT_INFO_HAS_MEM_REGIONS`.

### Memory region kinds
- `BOOT_MEM_REGION_USABLE`
- `BOOT_MEM_REGION_RESERVED`
- `BOOT_MEM_REGION_ACPI_RECLAIM`
- `BOOT_MEM_REGION_ACPI_NVS`
- `BOOT_MEM_REGION_MMIO`
- Purpose: normalized region classification for allocators and diagnostics.

### Memory init status values
- `BOOT_MEM_INIT_STATUS_NONE`
- `BOOT_MEM_INIT_STATUS_DONE`
- `BOOT_MEM_INIT_STATUS_DEFERRED`
- `BOOT_MEM_INIT_STATUS_FAILED`
- Purpose: architecture extension reporting of early memory init progress.

## `boot_mem_region_t`

```c
typedef struct {
  BOOT_U64 base;
  BOOT_U64 size;
  BOOT_U32 kind;
  BOOT_U32 reserved;
} boot_mem_region_t;
```

- Struct purpose: one normalized physical memory region descriptor.
- Members:
  - `base`: physical base address.
  - `size`: length in bytes.
  - `kind`: one of `BOOT_MEM_REGION_*` values.
  - `reserved`: reserved for alignment/future metadata; currently zeroed.

## `boot_info_t` (`struct boot_info`)

```c
typedef struct boot_info boot_info_t;
```

- Struct purpose: single canonical boot handoff object from architecture boot path to generic kernel.
- Members:
  - `abi_version`: handoff ABI version.
  - `arch_id`: architecture identifier.
  - `valid_mask`: bitmask describing which fields are valid.
  - `entry_pc`: instruction pointer/program counter at handoff snapshot.
  - `entry_sp`: stack pointer at handoff snapshot.
  - `vm_enabled`: non-zero if paging/MMU was enabled at handoff.
  - `vm_root_table`: architecture VM root register value (CR3/TTBR0/SATP-style).
  - `uefi_system_table`: EFI system table pointer when UEFI boot path is active.
  - `uefi_configuration_table`: EFI configuration table pointer.
  - `memory_map`: raw UEFI memory descriptor array pointer.
  - `memory_map_size`: bytes in memory map stream.
  - `memory_map_descriptor_size`: bytes per UEFI descriptor.
  - `memory_map_descriptor_version`: UEFI descriptor format version.
  - `memory_region_count`: populated entries in `memory_regions`.
  - `memory_region_capacity`: maximum entries available.
  - `memory_regions`: normalized fixed-capacity region array.
  - `acpi_rsdp`: ACPI RSDP physical/virtual pointer if available.
  - `dtb_ptr`: Flattened Device Tree pointer if available.
  - `boot_cpu_id`: ID of CPU/hart executing `kmain`.
  - `arch_data_ptr`: pointer to architecture extension payload.
  - `arch_data_size`: size of extension payload bytes.
  - `framebuffer_base`: framebuffer base address for early graphics output.
  - `framebuffer_width`: pixel width.
  - `framebuffer_height`: pixel height.
  - `framebuffer_pixels_per_scanline`: line stride in pixels.
  - `framebuffer_format`: firmware pixel format code.
  - `serial_port`: early serial MMIO/PIO base.

- Usage notes:
  - `boot_info_t` is the only required input to `kmain`.
  - Every field read must be gated by `valid_mask` bit checks.

## `boot_info_ext_uefi_t`

- Struct purpose: UEFI-specific extension payload referenced by `boot_info_t.arch_data_ptr` on UEFI boots.
- Members:
  - `image_handle`: EFI image handle.
  - `system_table`: EFI system table pointer mirror.
  - `configuration_table`: EFI configuration table pointer mirror.
  - `boot_services`: EFI boot services table pointer.
  - `runtime_services`: EFI runtime services pointer.
  - `con_out`: EFI standard output protocol pointer.
  - `std_err`: EFI standard error protocol pointer.
  - `firmware_vendor`: firmware vendor string pointer.
  - `firmware_revision`: firmware revision value.
  - `mem_init_status`: early arch memory-init status.
  - `mem_old_root`: pre-takeover VM root value.
  - `mem_new_root`: post-takeover VM root value.
  - `mem_mapped_bytes`: identity-mapped byte span after takeover.
  - `paging_old_cr3`: x86 naming compatibility field.
  - `paging_new_cr3`: x86 naming compatibility field.
  - `paging_identity_bytes`: x86 naming compatibility field.

## `boot_info_ext_riscv64_t`

- Struct purpose: riscv64-specific extension payload.
- Members:
  - `hart_id`: boot hart ID.
  - `dtb_ptr`: resolved DTB pointer seen by riscv64 boot path.
  - `satp`: SATP register value captured at entry.
  - `uart_base`: UART base used by early console.
  - `ram_base`: discovered RAM base.
  - `ram_size`: discovered RAM size.
  - `entry_a0`: original `a0` entry register snapshot.
  - `entry_a1`: original `a1` entry register snapshot.
  - `mem_init_status`: early memory-init status.
  - `mem_old_root`: old SATP/root before takeover.
  - `mem_new_root`: new SATP/root after takeover.
  - `mem_mapped_bytes`: bytes identity mapped in takeover.

## Status Model (`status.h`)

### `typedef int status_t`
- Purpose: single shared status/error type across all kernel subsystems.

### Status constants
- `STATUS_OK`: success.
- `STATUS_INVALID_ARG`: invalid argument.
- `STATUS_NOT_FOUND`: resource not found.
- `STATUS_NO_MEMORY`: allocation failure.
- `STATUS_NOT_SUPPORTED`: operation unavailable on this backend.
- `STATUS_BUSY`: resource currently busy.
- `STATUS_FAULT`: hardware/protection fault.
- `STATUS_INTERNAL`: internal invariant failure.
- `STATUS_TRY_AGAIN`: transient failure; retry later.
- `STATUS_DEFERRED`: intentionally deferred work, non-fatal.

## Interrupt Types (`interrupts.h`, `panic.h`)

### `interrupt_frame_t`
- Struct purpose: architecture-neutral interrupt/trap frame passed into generic dispatch.
- Members:
  - `arch_id`: originating architecture ID.
  - `vector`: vector number.
  - `error_code`: architecture error code/syndrome subset.
  - `fault_addr`: faulting address (when applicable).
  - `ip`: interrupted instruction pointer.
  - `sp`: interrupted stack pointer.
  - `flags`: architecture flags/status register snapshot.

### `interrupt_handler_t`
- Signature: `void (*)(const interrupt_frame_t *frame, void *ctx)`.
- Purpose: registered handler callback type.

### `exception_info_t`
- Struct purpose: expanded panic-facing exception metadata.
- Members:
  - `class_id`: coarse class (`FAULT/TRAP/ABORT/IRQ/UNKNOWN`).
  - `arch_id`: source architecture.
  - `vector`: vector/cause.
  - `error_code`: architecture-defined code.
  - `raw_syndrome`: unparsed syndrome bits.
  - `fault_addr`: related fault address.
  - `ip`: instruction pointer.
  - `sp`: stack pointer.
  - `flags`: status/flags register snapshot.
  - `reason`: decoded readable reason string.

### `EXCEPTION_CLASS_*`
- Purpose: normalized exception class taxonomy for panic/logging.

## MMU Types (`arch_mm.h`)

### `mm_phys_addr_t`, `mm_virt_addr_t`
- Purpose: physical and virtual address scalar types.

### `mmu_prot_t`
- Values:
  - `MMU_PROT_READ`
  - `MMU_PROT_WRITE`
  - `MMU_PROT_EXEC`
  - `MMU_PROT_USER`
  - `MMU_PROT_DEVICE`
  - `MMU_PROT_GLOBAL`
- Purpose: generic protection flag set consumed by `mm_map`/`mm_protect`.

## Allocator Statistics (`page_alloc.h`, `kmalloc.h`)

### `page_alloc_stats_t`
- Struct purpose: physical page allocator summary.
- Members:
  - `available`: allocator initialized and available (`0/1`).
  - `total_pages`: total managed pages.
  - `free_pages`: currently free pages.

### `kmalloc_stats_t`
- Struct purpose: heap allocator health and accounting snapshot.
- Members:
  - `available`: allocator initialized and available.
  - `pages`: backing pages currently owned.
  - `bytes_total`: total bytes in managed pages.
  - `bytes_used`: bytes allocated to live blocks.
  - `bytes_free`: bytes free in managed pages.
  - `alloc_count`: successful allocation operations.
  - `free_count`: successful free operations.
  - `failed_alloc_count`: failed allocation operations.
  - `invalid_free_count`: frees rejected as invalid pointers.
  - `double_free_count`: frees rejected as double-free.

## Per-CPU Runtime (`percpu.h`)

### `percpu_t`
- Struct purpose: CPU-local runtime record.
- Members:
  - `cpu_id`: logical CPU/hart identifier.
  - `online`: whether CPU is currently marked online.
  - `irq_nesting`: interrupt nesting depth counter.
  - `preempt_disable_depth`: preemption disable nesting level.
  - `local_tick_count`: per-CPU tick accumulator.
  - `current_task`: scheduler-owned task pointer placeholder.
  - `arch_local`: architecture-private per-CPU pointer.

## Timebase Types (`timebase.h`)

### `clocksource_t`
- Struct purpose: monotonic cycle source descriptor.
- Members:
  - `name`: backend name string.
  - `freq_hz`: source frequency.
  - `read_cycles`: callback reading current cycle counter.

### `clockevent_t`
- Struct purpose: programmable timer event source descriptor.
- Members:
  - `name`: backend name string.
  - `set_periodic`: configure periodic interrupts.
  - `set_oneshot_ns`: configure one-shot interrupt after nanoseconds.
  - `ack`: acknowledge delivered event.

## IRQ Controller Ops (`irq_controller.h`)

### `irq_controller_ops_t`
- Struct purpose: backend vtable for generic IRQ controller facade.
- Members:
  - `enable_irq`: unmask/enable hardware IRQ line.
  - `disable_irq`: mask/disable hardware IRQ line.
  - `ack_irq`: acknowledge reception.
  - `eoi_irq`: end-of-interrupt completion.
  - `map_irq`: map hardware IRQ to CPU vector.
  - `vector_to_irq`: reverse mapping vector -> IRQ.

## Device Model Types (`device_model.h`)

### `driver_probe_fn`
- Signature:
  - `typedef status_t (*driver_probe_fn)(const void *hw_node);`
- Purpose: probe callback used to claim or reject candidate hardware nodes.

### `driver_init_fn`
- Signature:
  - `typedef status_t (*driver_init_fn)(void *dev);`
- Purpose: init callback invoked after probe success.

### `driver_t`
- Struct purpose: static driver descriptor registered in the early device model.
- Members:
  - `name`: driver label used in diagnostics.
  - `class_name`: initialization class (`irqc`, `timer`, `console`, `early`).
  - `probe`: probe callback.
  - `init`: initialization callback.

## Hardware Discovery Structures (`hw_desc.h`)

### `hw_cpu_desc_t`
- Members:
  - `cpu_id`: discovered CPU identifier.
  - `flags`: backend-specific attributes.

### `hw_irq_controller_desc_t`
- Members:
  - `type`: controller type (`HW_IRQ_CONTROLLER_*`).
  - `mmio_base`: MMIO base.
  - `mmio_size`: MMIO span.
  - `irq_base`: first IRQ managed.
  - `irq_count`: count of managed IRQ lines.

### `hw_timer_desc_t`
- Members:
  - `mmio_base`, `mmio_size`: timer MMIO window.
  - `irq`: timer interrupt line.
  - `freq_hz`: timer frequency when known.
  - `flags`: backend-specific timer attributes.

### `hw_mmio_region_desc_t`
- Members:
  - `base`: MMIO base.
  - `size`: MMIO span.

### `hw_uart_desc_t`
- Members:
  - `base`: UART base address.
  - `irq`: UART interrupt line.
  - `flags`: backend-specific UART attributes.

### `hw_desc_t`
- Struct purpose: normalized hardware inventory view used by generic kernel subsystems.
- Members:
  - `source_mask`: bitmask of discovery sources used (`FALLBACK/ACPI/DTB`).
  - `cpu_count`: valid entries in `cpus`.
  - `timer_count`: valid entries in `timers`.
  - `irq_controller_count`: valid entries in `irq_controllers`.
  - `mmio_region_count`: valid entries in `mmio_regions`.
  - `uart_count`: valid entries in `uarts`.
  - `cpus`, `timers`, `irq_controllers`, `mmio_regions`, `uarts`: fixed-capacity descriptor arrays.

## UEFI Protocol Structures (`uefi.h`)

These structures intentionally mirror firmware ABI layouts used during UEFI bring-up.

### `EFI_TABLE_HEADER`
- Common UEFI table header.
- Members: `Signature`, `Revision`, `HeaderSize`, `Crc32`, `Reserved`.

### `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL`
- Console output protocol used by `arch_puts` in UEFI paths.
- Members include callable pointers `Reset`, `OutputString` and additional optional protocol slots.

### `EFI_SYSTEM_TABLE`
- Top-level firmware table.
- Important members:
  - `FirmwareVendor`, `FirmwareRevision`.
  - `ConOut`, `StdErr`.
  - `RuntimeServices`, `BootServices`.
  - `NumberOfTableEntries`, `ConfigurationTable`.

### `EFI_GUID`, `EFI_CONFIGURATION_TABLE`
- GUID type and configuration table entry tuple.

### `EFI_MEMORY_DESCRIPTOR`
- Firmware memory map descriptor consumed during boot handoff capture.

### GOP structures
- `EFI_PIXEL_BITMASK`
- `EFI_GRAPHICS_OUTPUT_MODE_INFORMATION`
- `EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE`
- `EFI_GRAPHICS_OUTPUT_PROTOCOL`
- Purpose: framebuffer mode discovery.

### MP Services structures
- `EFI_MP_SERVICES_PROTOCOL`
- Function typedefs:
  - `EFI_MP_GET_NUMBER_OF_PROCESSORS`
  - `EFI_MP_STARTUP_ALL_APS`
  - `EFI_MP_WHOAMI`
- Purpose: CPU enumeration and AP startup on UEFI-capable platforms.

### `EFI_BOOT_SERVICES`
- Firmware boot services dispatch table used for memory map, protocol lookup, and allocation operations.

## Architecture-Specific Structure Types

These are documented in architecture API reference pages:
- `x86_64_early_paging_result_t`
- `arm64_early_paging_result_t`
- `riscv64_early_paging_result_t`

See:
- `docs/manual/x86_64/api-reference.md`
- `docs/manual/arm64/api-reference.md`
- `docs/manual/riscv64/api-reference.md`
