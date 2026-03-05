# Device Reporting and Introspection

The reporting layer exposes a stable way to inspect discovered devices from generic kernel code.

## Public API

Header: `kernel/include/device_report.h`

- `u64 device_report_count(void)`
- `status_t device_report_get(u64 index, device_report_entry_t *out_entry)`
- `void device_report_dump_all(void)`
- `void device_report_dump_class(device_class_t class_id)`

## `device_report_entry_t`

Fields provided to callers:
- identity: `id`, `parent_id`, `bus_id`, `name`
- classing: `class_id`, `class_code`, `subclass_code`, `prog_if`
- vendor identity: `vendor_id`, `device_id`
- resources: `resource_count`

## Intended Usage

- Bring-up diagnostics.
- Architecture-agnostic subsystem probing by class.
- Test assertions that expected classes exist.

Example:

```c
device_report_entry_t e;
if (device_report_get(0, &e) == STATUS_OK) {
  kprintf("first device class=%llu name=%s\n", e.class_id, e.name);
}
```
