# API Cheatsheet

## Boot and Core Entry

- `kmain(const boot_info_t *boot_info)`
- `diag_boot_info_print(const boot_info_t *boot_info)`

## Status

- `status_is_ok(status_t)`
- `status_str(status_t)`
- `panic(const char *reason)`
- `panicf(const char *fmt, ...)`
- `panic_from_exception(const exception_info_t *info)`

## CPU Layer

- `status_t arch_cpu_early_init(const boot_info_t *boot_info)`
- `status_t arch_cpu_late_init(void)`
- `u64 arch_cpu_id(void)`
- `u64 arch_cpu_count_hint(void)`
- `void arch_cpu_relax(void)`
- `void arch_cpu_halt(void)`
- `void arch_cpu_reboot(void)`
- `u64 arch_cycle_counter(void)`
- `status_t arch_cpu_set_local_base(u64 base)`
- `u64 arch_cpu_get_local_base(void)`

## Memory and MMU

- `status_t arch_memory_init(boot_info_t *boot_info)`
- `status_t arch_mm_early_init(boot_info_t *boot_info)`
- `status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, u64 size, u64 prot_flags)`
- `status_t mm_unmap(mm_virt_addr_t va, u64 size)`
- `status_t mm_protect(mm_virt_addr_t va, u64 size, u64 prot_flags)`
- `status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, u64 *out_flags)`
- `status_t mm_sync_tlb(mm_virt_addr_t va, u64 size)`
- `u64 mm_page_size(void)`
- `status_t page_alloc_init(boot_info_t *boot_info)`
- `u64 alloc_page(void)`
- `void free_page(u64 phys_addr)`
- `void page_alloc_stats(page_alloc_stats_t *out)`
- `status_t kmalloc_init(boot_info_t *boot_info)`
- `void *kmalloc(u64 size)`
- `void kfree(void *ptr)`
- `void kmalloc_stats(kmalloc_stats_t *out)`

## Per-CPU

- `status_t percpu_init_boot_cpu(const boot_info_t *boot_info)`
- `status_t percpu_register_current_cpu(u64 cpu_id)`
- `percpu_t *percpu_current(void)`
- `percpu_t *percpu_by_id(u64 cpu_id)`
- `u64 percpu_online_count(void)`

## SMP

- `status_t smp_init(const boot_info_t *boot_info)`
- `status_t smp_cpu_start(u64 cpu_id)`
- `u64 smp_cpu_count_online(void)`
- `u64 smp_cpu_count_possible(void)`
- `void smp_secondary_entry(u64 cpu_id)`
- `status_t arch_smp_bootstrap(const boot_info_t *boot_info, u64 *out_possible_cpus, u64 *out_started_cpus)`
- `status_t arch_smp_cpu_start(u64 cpu_id)`
- `status_t arch_smp_ipi_send(u64 cpu_id, u64 kind)`
- `status_t arch_smp_tlb_shootdown(u64 mask, u64 va, u64 len)`
- `status_t ipi_send(u64 cpu_id, ipi_kind_t kind)`
- `status_t tlb_shootdown(cpu_mask_t mask, virt_addr_t va, u64 len)`

## Scheduler

- `status_t sched_init(void)`
- `task_t *sched_current(void)`
- `status_t sched_add(task_t *task)`
- `void sched_tick(void)`
- `status_t arch_context_switch(task_t *from, task_t *to)`

## Observability

- `status_t trace_init(const boot_info_t *boot_info)`
- `void trace_emit(trace_class_t cls, u64 a0, u64 a1, u64 a2)`
- `status_t trace_dump(trace_sink_t sink)`
- `void trace_sink_kprintf(const trace_record_t *record)`

## Device Discovery

- `status_t hw_discovery_init(const boot_info_t *boot_info)`
- `const hw_desc_t *hw_desc_get(void)`
- `u64 hw_desc_cpu_count_hint(void)`

## Device Model

- `status_t driver_set_boot_info(const boot_info_t *boot_info)`
- `void driver_registry_reset(void)`
- `status_t driver_register(const driver_t *drv)`
- `status_t driver_probe_all(const hw_desc_t *hw)`
- `status_t driver_class_last_status(const char *class_name)`
- `status_t device_model_register_builtin_drivers(void)`

## Device Bus

- `status_t device_bus_init(const boot_info_t *boot_info, const hw_desc_t *hw)`
- `void device_bus_reset(void)`
- `status_t device_bus_register_bus(const bus_t *bus_template, u64 *out_bus_id)`
- `status_t device_bus_register_device(const device_t *dev_template, u64 *out_device_id)`
- `status_t device_bus_remove_device(u64 device_id)`
- `status_t device_bus_register_hotplug(device_hotplug_fn_t on_add, device_hotplug_fn_t on_remove, void *ctx)`
- `const bus_t *device_bus_get_bus(u64 bus_id)`
- `const device_t *device_bus_get_device(u64 device_id)`
- `u64 device_bus_count(void)`
- `const device_t *device_bus_device_at(u64 index)`
- `u64 device_bus_find_first_by_class(device_class_t class_id)`
- `u64 device_bus_find_next_by_class(device_class_t class_id, u64 after_id)`
- `void device_bus_dump(void)`

## PCI

- `status_t pci_enumerate(const boot_info_t *boot_info)`
- `u64 pci_device_count(void)`

## USB

- `status_t usb_enumerate(const boot_info_t *boot_info)`
- `u64 usb_host_count(void)`
- `u64 usb_device_count(void)`

## Device Domains

- `status_t device_domains_enumerate(const boot_info_t *boot_info)`
- `u64 block_device_count(void)`
- `u64 input_device_count(void)`
- `u64 display_device_count(void)`

## Device Reporting

- `u64 device_report_count(void)`
- `status_t device_report_get(u64 index, device_report_entry_t *out_entry)`
- `void device_report_dump_all(void)`
- `void device_report_dump_class(device_class_t class_id)`

## Capability Profiles

- `const char *capability_profile_name(void)`
- `int capability_domain_enabled(device_class_t class_id)`
- `const char *capability_domain_name(device_class_t class_id)`
- `status_t capability_domain_state(device_class_t class_id)`
- `void capability_domain_dump_matrix(void)`
- `void capability_profile_print(void)`

## Syscall Transport

- `status_t arch_syscall_init(const boot_info_t *boot_info)`
- `status_t arch_syscall_get_vector(u64 *out_vector)`
- `status_t arch_syscall_trigger(void)`
- `status_t syscall_init(const boot_info_t *boot_info)`
- `status_t syscall_register(u64 op, syscall_handler_t handler, const char *owner)`
- `status_t syscall_dispatch(const syscall_request_t *req, syscall_response_t *resp)`
- `status_t syscall_invoke_kernel(..., syscall_response_t *resp)`
- `status_t syscall_invoke_trap(..., syscall_response_t *resp)`
- `u64 syscall_abi_info_word(void)`
- `int syscall_trap_entry_ready(void)`
- `void syscall_dump_table(void)`

## Network Domain

- `status_t net_enumerate(const boot_info_t *boot_info)`
- `u64 net_device_count(void)`
- `status_t net_device_info_at(u64 index, net_device_info_t *out_info)`
- `void net_dump_diagnostics(void)`

## Audio Domain

- `status_t audio_enumerate(const boot_info_t *boot_info)`
- `u64 audio_device_count(void)`
- `status_t audio_device_info_at(u64 index, audio_device_info_t *out_info)`
- `void audio_dump_diagnostics(void)`

## Interrupts

- `status_t arch_irq_init(const boot_info_t *boot_info)`
- `void arch_irq_enable(void)`
- `void arch_irq_disable(void)`
- `status_t interrupts_init(const boot_info_t *boot_info)`
- `status_t interrupts_register_handler(... )`
- `status_t interrupts_register_handler_owned(... )`
- `status_t interrupts_unregister_handler(... )`
- `const char *interrupts_handler_owner(u64 vector)`
- `void interrupts_dispatch(const interrupt_frame_t *frame)`
- `void interrupts_enable(void)`
- `void interrupts_disable(void)`

## Interrupt Controller

- `status_t irq_controller_register(const char *name, const irq_controller_ops_t *ops)`
- `const char *irq_controller_name(void)`
- `status_t irq_controller_enable(u64 irq)`
- `status_t irq_controller_disable(u64 irq)`
- `void irq_controller_ack(u64 irq)`
- `void irq_controller_eoi(u64 irq)`
- `status_t irq_controller_map(u64 irq, u64 *out_vector)`
- `status_t irq_controller_vector_to_irq(u64 vector, u64 *out_irq)`

## Hardware Resources

- `status_t hw_resource_init(const boot_info_t *boot_info)`
- `status_t hw_resource_attach(u64 device_id, const hw_resource_t *list, u64 count)`
- `status_t hw_resource_get(u64 device_id, hw_resource_type_t type, u64 index, hw_resource_t *out)`
- `u64 hw_resource_count(u64 device_id, hw_resource_type_t type)`
- `status_t hw_resource_view(u64 device_id, device_resource_view_t *out)`

## IRQ Domain

- `status_t irq_domain_init(const boot_info_t *boot_info)`
- `status_t irq_alloc_line(u64 device_id, u64 hwirq, irq_desc_t *out)`
- `status_t irq_alloc_msi(u64 device_id, u64 nvec, irq_desc_t *out_vec)`
- `status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg)`
- `status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask)`
- `u64 irq_domain_alloc_count(void)`

## DMA

- `status_t dma_init(const boot_info_t *boot_info)`
- `status_t dma_map(u64 device_id, void *cpu_ptr, u64 len, dma_dir_t dir, dma_addr_t *out)`
- `status_t dma_unmap(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir)`
- `status_t dma_sync_for_device(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir)`
- `status_t dma_sync_for_cpu(u64 device_id, dma_addr_t addr, u64 len, dma_dir_t dir)`
- `status_t dma_set_constraints(u64 device_id, const dma_constraints_t *constraints)`
- `status_t dma_get_constraints(u64 device_id, dma_constraints_t *out_constraints)`

## IOMMU

- `status_t iommu_init(const boot_info_t *boot_info)`
- `status_t iommu_domain_create(iommu_domain_t *out_domain)`
- `status_t iommu_attach(iommu_domain_t domain, u64 device_id)`
- `status_t iommu_detach(iommu_domain_t domain, u64 device_id)`
- `status_t iommu_map(iommu_domain_t domain, iova_t iova, phys_addr_t pa, u64 len, iommu_perm_t perm)`
- `status_t iommu_unmap(iommu_domain_t domain, iova_t iova, u64 len)`
- `status_t iommu_set_passthrough(iommu_domain_t domain, int enabled)`

## Clock/Reset/Power

- `status_t clock_enable(clock_id_t clk)`
- `status_t clock_disable(clock_id_t clk)`
- `status_t clock_set_rate(clock_id_t clk, u64 hz)`
- `status_t clock_get_rate(clock_id_t clk, u64 *out_hz)`
- `status_t reset_assert(reset_id_t rst)`
- `status_t reset_deassert(reset_id_t rst)`
- `status_t reset_pulse(reset_id_t rst)`
- `status_t power_domain_on(power_domain_id_t pd)`
- `status_t power_domain_off(power_domain_id_t pd)`
- `status_t power_domain_status(power_domain_id_t pd, u64 *out_on)`

## CPU Caps/Context

- `status_t cpu_caps_query(cpu_caps_t *out_caps)`
- `status_t cpu_context_init(cpu_context_t *ctx, void (*entry)(void *), void *arg, void *stack_top)`
- `status_t cpu_context_switch(cpu_context_t *from, cpu_context_t *to)`

## Personality Hooks

- `status_t personality_register(const personality_ops_t *ops)`
- `status_t personality_activate(personality_id_t id)`
- `status_t personality_exec(const exec_image_t *img, exec_result_t *out)`
- `personality_id_t personality_active_id(void)`
- `const char *personality_active_name(void)`

## Driver Helpers

- `driver_ring_init(driver_ring_t *ring, u64 capacity)`
- `driver_ring_push(driver_ring_t *ring, u64 *out_slot)`
- `driver_ring_pop(driver_ring_t *ring, u64 *out_slot)`
- `driver_ring_count(const driver_ring_t *ring)`
- `driver_irq_complete(const interrupt_frame_t *frame)`
- `driver_retry_begin(driver_retry_t *ctx, u64 max_attempts)`
- `driver_retry_next(driver_retry_t *ctx)`

## Time

- `status_t timer_init(const boot_info_t *boot_info)`
- `u64 timer_ticks(void)`
- `u64 timer_hz(void)`
- `status_t time_init(const boot_info_t *boot_info)`
- `u64 time_now_ns(void)`
- `u64 time_ticks(void)`
- `u64 time_hz(void)`
- `u64 time_cycles_to_ns(u64 cycles)`
- `u64 time_ns_to_cycles(u64 ns)`
- `status_t time_quality(time_quality_t *out)`
- `const clocksource_t *time_clocksource(void)`
- `const clockevent_t *time_clockevent(void)`

## Printing

- `int kprintf(const char *fmt, ...)`
- `int ksnprintf(char *buf, unsigned long size, const char *fmt, ...)`
- `int kvsnprintf(char *buf, unsigned long size, const char *fmt, va_list args)`
