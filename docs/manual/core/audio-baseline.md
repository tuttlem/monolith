# Audio Baseline (Optional Domain)

The audio baseline layer creates generic audio-domain devices from discovered bus devices.

## Public API

Header: `kernel/include/audio.h`

- `status_t audio_enumerate(const boot_info_t *boot_info)`
- `BOOT_U64 audio_device_count(void)`
- `status_t audio_device_info_at(BOOT_U64 index, audio_device_info_t *out_info)`
- `void audio_dump_diagnostics(void)`

## `audio_device_info_t`

The descriptor provides OS writers a stable, early audio capability view.

- `device_id`: device-bus ID of the audio-domain node.
- `parent_id`: source bus node ID (usually the backing PCI device node).
- `vendor_id`: vendor identifier copied from source device.
- `pci_device_id`: PCI device identifier copied from source device.
- `class_code`/`subclass_code`/`prog_if`: copied PCI class tuple.
- `resource_count`: copied resource descriptor count.
- `has_output`: output path capability flag.
- `has_input`: input path capability flag.
- `min_sample_rate_hz`/`max_sample_rate_hz`: baseline sample-rate capability summary.

## Current Behavior

- Requires a non-null `boot_info`.
- Honors feature gating with `MONOLITH_CAP_AUDIO`.
- Scans `device_bus` for `DEVICE_CLASS_PCI_DEVICE` with class code `0x04` (multimedia/audio).
- Registers one `DEVICE_CLASS_AUDIO` node per match.
- Stores a fixed-capacity table of `audio_device_info_t` entries.
- Prints diagnostics through `audio_dump_diagnostics()`.

## Architecture Notes

- `x86_64`: can discover audio-domain devices when PCI enumeration finds audio-class functions.
- `arm64`/`riscv64`: path is available but discovery depends on future PCI backend enablement.

## Usage Example

```c
status_t st = audio_enumerate(boot_info);
if (st == STATUS_OK) {
  BOOT_U64 n = audio_device_count();
  BOOT_U64 i;
  for (i = 0; i < n; ++i) {
    audio_device_info_t info;
    if (audio_device_info_at(i, &info) == STATUS_OK) {
      kprintf("audio[%llu]: out=%llu in=%llu\n", i, info.has_output, info.has_input);
    }
  }
}
```
