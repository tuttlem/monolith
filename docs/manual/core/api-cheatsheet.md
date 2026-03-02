# API Cheatsheet

## Boot and Core Entry

- `kmain(const boot_info_t *boot_info)`
- `diag_boot_info_print(const boot_info_t *boot_info)`

## Status

- `status_is_ok(status_t)`
- `status_str(status_t)`

## Memory Init and Allocation

- `status_t arch_memory_init(boot_info_t *boot_info)`
- `status_t page_alloc_init(boot_info_t *boot_info)`
- `BOOT_U64 alloc_page(void)`
- `void free_page(BOOT_U64 phys_addr)`
- `void page_alloc_stats(page_alloc_stats_t *out)`
- `status_t kmalloc_init(boot_info_t *boot_info)`
- `void *kmalloc(BOOT_U64 size)`
- `void kfree(void *ptr)`
- `void kmalloc_stats(kmalloc_stats_t *out)`

## Interrupts

- `status_t interrupts_init(const boot_info_t *boot_info)`
- `status_t interrupts_register_handler(... )`
- `status_t interrupts_register_handler_owned(... )`
- `status_t interrupts_unregister_handler(... )`
- `const char *interrupts_handler_owner(BOOT_U64 vector)`
- `void interrupts_dispatch(const interrupt_frame_t *frame)`
- `void interrupts_enable(void)`
- `void interrupts_disable(void)`

## Timer

- `status_t timer_init(const boot_info_t *boot_info)`
- `BOOT_U64 timer_ticks(void)`
- `BOOT_U64 timer_hz(void)`

## Printing

- `int kprintf(const char *fmt, ...)`
- `int ksnprintf(char *buf, unsigned long size, const char *fmt, ...)`
- `int kvsnprintf(char *buf, unsigned long size, const char *fmt, va_list args)`
