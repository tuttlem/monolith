# Device Model Baseline

The device model is a minimal static driver framework used during early kernel bring-up.

## Why This Exists

- Removes ad-hoc init sequencing from `kmain`.
- Makes initialization order explicit and deterministic.
- Allows existing architecture backends (IRQ and timer) to be represented as drivers.

## Init Order Policy

Driver probing and init run in this fixed class order:

1. `irqc` (interrupt controllers / trap backend hook)
2. `timer` (periodic time source)
3. `console` (serial/framebuffer handoff)
4. `early` (other early MMIO-class devices)

This order is enforced in `driver_probe_all()`.

## Public API

Header: `kernel/include/device_model.h`

### `status_t driver_set_boot_info(const boot_info_t *boot_info)`
- Stores the active boot handoff pointer used by builtin drivers.

### `void driver_registry_reset(void)`
- Clears all registered drivers and class status state.
- Use before registering a new static driver set.

### `status_t driver_register(const driver_t *drv)`
- Registers one static driver descriptor.
- Registration order is preserved and affects probe order within a class.

### `status_t driver_probe_all(const hw_desc_t *hw)`
- Probes and initializes drivers over discovered hardware nodes in class order.
- Returns:
  - `STATUS_OK` if no hard failures occurred.
  - last hard failure code when any class has an unrecoverable init/probe error.

### `status_t driver_class_last_status(const char *class_name)`
- Reports the last status produced for a class (`irqc`, `timer`, `console`, `early`).

### `status_t device_model_register_builtin_drivers(void)`
- Registers built-in baseline drivers:
  - `irq-backend`
  - `timer-backend`
  - `early-console`

## Core Types

### `driver_probe_fn`
```c
typedef status_t (*driver_probe_fn)(const void *hw_node);
```
- Called for each candidate node in matching class.
- Return `STATUS_OK` to claim node, `STATUS_NOT_FOUND` to decline, `STATUS_DEFERRED` to skip safely.

### `driver_init_fn`
```c
typedef status_t (*driver_init_fn)(void *dev);
```
- Called after successful probe to perform actual initialization.

### `driver_t`
```c
typedef struct {
  const char *name;
  const char *class_name;
  driver_probe_fn probe;
  driver_init_fn init;
} driver_t;
```
- `name`: stable diagnostics label.
- `class_name`: one of current class names (`irqc`, `timer`, `console`, `early`).
- `probe`: node claim callback.
- `init`: node init callback.

## Built-in Driver Behavior

- `irq-backend` (`class_name="irqc"`)
  - First successful node triggers `interrupts_init(boot_info)` once.
- `timer-backend` (`class_name="timer"`)
  - First successful node triggers `timer_init(boot_info)` once.
- `early-console` (`class_name="console"`)
  - Claims UART/framebuffer nodes and marks console path as driver-managed.

## Runtime Diagnostics

`driver_probe_all()` prints clear logs for probe/init outcomes:

- successful init:
  - `device: init class=<class> drv=<name> idx=<n> st=<status> (<code>)`
- probe/init failures:
  - `device: probe fail ...`
  - `device: init fail ...`

## Usage in `kmain`

Current bring-up sequence uses the device model after hardware discovery:

```c
driver_registry_reset();
driver_set_boot_info(boot_info);
device_model_register_builtin_drivers();
driver_probe_all(hw_desc_get());

irq_status = driver_class_last_status("irqc");
timer_status = driver_class_last_status("timer");
```

This keeps architecture-specific IRQ/timer code behind existing HAL interfaces while exposing deterministic initialization through one generic subsystem.
