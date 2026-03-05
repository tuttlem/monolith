# Network Baseline (Optional Domain)

The network baseline layer creates generic network-domain devices from already enumerated bus nodes.

## Public API

Header: `kernel/include/net.h`

- `status_t net_enumerate(const boot_info_t *boot_info)`
- `u64 net_device_count(void)`
- `status_t net_device_info_at(u64 index, net_device_info_t *out_info)`
- `void net_dump_diagnostics(void)`

## `net_device_info_t`

This descriptor gives OS writers a stable, architecture-agnostic summary for each discovered network device.

- `device_id`: device-bus ID of the network-domain node.
- `parent_id`: source bus node ID (usually the backing PCI device node).
- `vendor_id`: vendor identifier copied from source device.
- `pci_device_id`: device identifier copied from source device.
- `class_code`/`subclass_code`/`prog_if`: class tuple of source function.
- `resource_count`: copied resource descriptor count.
- `mac_hi`/`mac_lo`: MAC placeholder (currently unknown until driver-specific init).
- `link_up`: baseline link state (`0` until controller driver brings link up).
- `tx_queue_count`/`rx_queue_count`: queue capability summary placeholder (currently `1` each).

## Current Behavior

- Requires a non-null `boot_info`.
- Honors feature gating with `MONOLITH_CAP_NETWORK`.
- Scans `device_bus` for `DEVICE_CLASS_PCI_DEVICE` with PCI class code `0x02` (network controller).
- Registers one `DEVICE_CLASS_NET` device node per match.
- Stores a `net_device_info_t` entry for diagnostics/introspection.
- Prints diagnostics through `net_dump_diagnostics()`.

## Architecture Notes

- `x86_64`: can discover NICs when PCI enumeration is active.
- `arm64`/`riscv64`: enumeration path is available but currently depends on future PCI backend enablement.

## Usage Example

```c
status_t st = net_enumerate(boot_info);
if (st == STATUS_OK) {
  u64 n = net_device_count();
  u64 i;
  for (i = 0; i < n; ++i) {
    net_device_info_t info;
    if (net_device_info_at(i, &info) == STATUS_OK) {
      kprintf("net[%llu]: ven=0x%llx dev=0x%llx\n", i, info.vendor_id, info.pci_device_id);
    }
  }
}
```
