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
- `BOOT_U64 arch_cpu_id(void)`
- `BOOT_U64 arch_cpu_count_hint(void)`
- `void arch_cpu_relax(void)`
- `void arch_cpu_halt(void)`
- `void arch_cpu_reboot(void)`
- `BOOT_U64 arch_cycle_counter(void)`
- `status_t arch_cpu_set_local_base(BOOT_U64 base)`
- `BOOT_U64 arch_cpu_get_local_base(void)`

## Memory and MMU

- `status_t arch_memory_init(boot_info_t *boot_info)`
- `status_t arch_mm_early_init(boot_info_t *boot_info)`
- `status_t mm_map(mm_virt_addr_t va, mm_phys_addr_t pa, BOOT_U64 size, BOOT_U64 prot_flags)`
- `status_t mm_unmap(mm_virt_addr_t va, BOOT_U64 size)`
- `status_t mm_protect(mm_virt_addr_t va, BOOT_U64 size, BOOT_U64 prot_flags)`
- `status_t mm_translate(mm_virt_addr_t va, mm_phys_addr_t *out_pa, BOOT_U64 *out_flags)`
- `status_t mm_sync_tlb(mm_virt_addr_t va, BOOT_U64 size)`
- `BOOT_U64 mm_page_size(void)`
- `status_t page_alloc_init(boot_info_t *boot_info)`
- `BOOT_U64 alloc_page(void)`
- `void free_page(BOOT_U64 phys_addr)`
- `void page_alloc_stats(page_alloc_stats_t *out)`
- `status_t kmalloc_init(boot_info_t *boot_info)`
- `void *kmalloc(BOOT_U64 size)`
- `void kfree(void *ptr)`
- `void kmalloc_stats(kmalloc_stats_t *out)`

## Per-CPU

- `status_t percpu_init_boot_cpu(const boot_info_t *boot_info)`
- `status_t percpu_register_current_cpu(BOOT_U64 cpu_id)`
- `percpu_t *percpu_current(void)`
- `percpu_t *percpu_by_id(BOOT_U64 cpu_id)`
- `BOOT_U64 percpu_online_count(void)`

## SMP

- `status_t smp_init(const boot_info_t *boot_info)`
- `BOOT_U64 smp_cpu_count_online(void)`
- `BOOT_U64 smp_cpu_count_possible(void)`
- `void smp_secondary_entry(BOOT_U64 cpu_id)`
- `status_t arch_smp_bootstrap(const boot_info_t *boot_info, BOOT_U64 *out_possible_cpus, BOOT_U64 *out_started_cpus)`

## Device Discovery

- `status_t hw_discovery_init(const boot_info_t *boot_info)`
- `const hw_desc_t *hw_desc_get(void)`
- `BOOT_U64 hw_desc_cpu_count_hint(void)`

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
- `status_t device_bus_register_bus(const bus_t *bus_template, BOOT_U64 *out_bus_id)`
- `status_t device_bus_register_device(const device_t *dev_template, BOOT_U64 *out_device_id)`
- `const bus_t *device_bus_get_bus(BOOT_U64 bus_id)`
- `const device_t *device_bus_get_device(BOOT_U64 device_id)`
- `BOOT_U64 device_bus_count(void)`
- `const device_t *device_bus_device_at(BOOT_U64 index)`
- `BOOT_U64 device_bus_find_first_by_class(device_class_t class_id)`
- `BOOT_U64 device_bus_find_next_by_class(device_class_t class_id, BOOT_U64 after_id)`
- `void device_bus_dump(void)`

## PCI

- `status_t pci_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 pci_device_count(void)`

## USB

- `status_t usb_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 usb_host_count(void)`
- `BOOT_U64 usb_device_count(void)`

## Device Domains

- `status_t device_domains_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 block_device_count(void)`
- `BOOT_U64 input_device_count(void)`
- `BOOT_U64 display_device_count(void)`

## Device Reporting

- `BOOT_U64 device_report_count(void)`
- `status_t device_report_get(BOOT_U64 index, device_report_entry_t *out_entry)`
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
- `status_t arch_syscall_get_vector(BOOT_U64 *out_vector)`
- `status_t arch_syscall_trigger(void)`
- `status_t syscall_init(const boot_info_t *boot_info)`
- `status_t syscall_register(BOOT_U64 op, syscall_handler_t handler, const char *owner)`
- `status_t syscall_dispatch(const syscall_request_t *req, syscall_response_t *resp)`
- `status_t syscall_invoke_kernel(..., syscall_response_t *resp)`
- `status_t syscall_invoke_trap(..., syscall_response_t *resp)`
- `BOOT_U64 syscall_abi_info_word(void)`
- `int syscall_trap_entry_ready(void)`
- `void syscall_dump_table(void)`

## Network Domain

- `status_t net_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 net_device_count(void)`
- `status_t net_device_info_at(BOOT_U64 index, net_device_info_t *out_info)`
- `void net_dump_diagnostics(void)`

## Audio Domain

- `status_t audio_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 audio_device_count(void)`
- `status_t audio_device_info_at(BOOT_U64 index, audio_device_info_t *out_info)`
- `void audio_dump_diagnostics(void)`

## Interrupts

- `status_t arch_irq_init(const boot_info_t *boot_info)`
- `void arch_irq_enable(void)`
- `void arch_irq_disable(void)`
- `status_t interrupts_init(const boot_info_t *boot_info)`
- `status_t interrupts_register_handler(... )`
- `status_t interrupts_register_handler_owned(... )`
- `status_t interrupts_unregister_handler(... )`
- `const char *interrupts_handler_owner(BOOT_U64 vector)`
- `void interrupts_dispatch(const interrupt_frame_t *frame)`
- `void interrupts_enable(void)`
- `void interrupts_disable(void)`

## Interrupt Controller

- `status_t irq_controller_register(const char *name, const irq_controller_ops_t *ops)`
- `const char *irq_controller_name(void)`
- `status_t irq_controller_enable(BOOT_U64 irq)`
- `status_t irq_controller_disable(BOOT_U64 irq)`
- `void irq_controller_ack(BOOT_U64 irq)`
- `void irq_controller_eoi(BOOT_U64 irq)`
- `status_t irq_controller_map(BOOT_U64 irq, BOOT_U64 *out_vector)`
- `status_t irq_controller_vector_to_irq(BOOT_U64 vector, BOOT_U64 *out_irq)`

## Hardware Resources

- `status_t hw_resource_init(const boot_info_t *boot_info)`
- `status_t hw_resource_attach(BOOT_U64 device_id, const hw_resource_t *list, BOOT_U64 count)`
- `status_t hw_resource_get(BOOT_U64 device_id, hw_resource_type_t type, BOOT_U64 index, hw_resource_t *out)`
- `BOOT_U64 hw_resource_count(BOOT_U64 device_id, hw_resource_type_t type)`
- `status_t hw_resource_view(BOOT_U64 device_id, device_resource_view_t *out)`

## IRQ Domain

- `status_t irq_domain_init(const boot_info_t *boot_info)`
- `status_t irq_alloc_line(BOOT_U64 device_id, BOOT_U64 hwirq, irq_desc_t *out)`
- `status_t irq_alloc_msi(BOOT_U64 device_id, BOOT_U64 nvec, irq_desc_t *out_vec)`
- `status_t irq_configure(const irq_desc_t *irq, irq_cfg_t cfg)`
- `status_t irq_set_affinity(const irq_desc_t *irq, cpu_mask_t mask)`
- `BOOT_U64 irq_domain_alloc_count(void)`

## Time

- `status_t timer_init(const boot_info_t *boot_info)`
- `BOOT_U64 timer_ticks(void)`
- `BOOT_U64 timer_hz(void)`
- `status_t time_init(const boot_info_t *boot_info)`
- `BOOT_U64 time_now_ns(void)`
- `BOOT_U64 time_ticks(void)`
- `BOOT_U64 time_hz(void)`
- `const clocksource_t *time_clocksource(void)`
- `const clockevent_t *time_clockevent(void)`

## Printing

- `int kprintf(const char *fmt, ...)`
- `int ksnprintf(char *buf, unsigned long size, const char *fmt, ...)`
- `int kvsnprintf(char *buf, unsigned long size, const char *fmt, va_list args)`
