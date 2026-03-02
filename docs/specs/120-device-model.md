# 120 Device Model Baseline

## Goal

Provide a tiny driver framework with deterministic registration and init order.

## Core Concepts

- driver descriptor
- device instance descriptor
- probe/attach hooks

## Minimal API

```c
typedef status_t (*driver_probe_fn)(const void *hw_node);
typedef status_t (*driver_init_fn)(void *dev);

typedef struct {
  const char *name;
  const char *class_name; // timer, irqc, serial, fb, etc.
  driver_probe_fn probe;
  driver_init_fn init;
} driver_t;

status_t driver_register(const driver_t *drv);
status_t driver_probe_all(const hw_desc_t *hw);
```

## Init Order Policy

1. interrupt controllers
2. timer sources
3. console devices
4. remaining early devices

## Requirements

- static registration first (no dynamic module loading in this phase)
- clear status and logs for probe failures
- no hidden global ordering dependencies

## Acceptance Criteria

- current timer/irq backends can be represented through device model hooks
- serial/framebuffer early console can transition to driver-managed path
