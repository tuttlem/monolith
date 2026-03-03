# How to use this repository

This repository is the Monolith starter project: a reusable hardware/bootstrap base for building operating systems.

It is not a complete OS. It gives you stable boot and hardware-facing primitives so you can focus on OS policy and behavior.

## What this gives you already

At a high level, this project already provides:

1. Boot handoff ABI (`boot_info_t`) across supported architectures.
2. Early CPU and memory bring-up (MMU baseline, page allocator, `kmalloc`).
3. Interrupt/exception framework and timer/timebase layer.
4. Device discovery + bus graph + baseline device-domain classification.
5. Syscall transport layer (minimal neutral calls, dispatch table).
6. Status/error model (`status_t`) and panic/assert policy.
7. Diagnostics/observability and architecture manuals.

Use this as your hardware/platform layer. Build your OS personality above it.

## Supported architectures

- `x86_64`
- `arm64`
- `riscv64`

See architecture-specific manuals under:
- `docs/manual/x86_64`
- `docs/manual/arm64`
- `docs/manual/riscv64`

## Start your own OS repo

1. Copy this directory to a new project name.
2. Remove inherited history.
3. Initialize your own history.

```bash
cp -a monolith toyos
cd toyos
rm -rf .git
git init
git add .
git commit -m "initial Monolith starter import"
```

## First orientation pass

Read these first:

1. `docs/manual/core/boot-sequence.md`
2. `docs/manual/core/boot-info-abi.md`
3. `docs/manual/core/status-system.md`
4. `docs/manual/core/api-reference.md`
5. `docs/manual/core/api-cheatsheet.md`

Then inspect `kernel/kmain.c` to see exact initialization order and integration points.

## Build and run

```bash
make smoke-x86_64 smoke-arm64 smoke-riscv64
make run-x86_64
make run-arm64
make run-riscv64
```

## What to build next (toy OS roadmap)

Devices, file system, and scheduler are important, but they are only part of the OS layer.

Recommended order:

1. Define your OS identity and ABI
- Choose your syscall namespace and semantics.
- Decide process/thread model (or no-process model for a simpler OS).
- Write this down in `docs/OS_DESIGN.md`.

2. Execution model
- Replace/extend scheduler policy (`kernel/include/scheduler.h` hooks are present).
- Define task/thread structs for your OS layer.
- Add context-switch policy and run-queue logic appropriate for your design.

3. Virtual memory policy
- Keep existing map/unmap hardware API.
- Add your OS address-space model:
  - kernel/user split (if applicable)
  - region mapping policy
  - page-fault policy

4. Process and program model
- Decide: kernel-only tasks, user mode, or mixed.
- Add program/image loader format (ELF, custom, flat binary, etc.).
- Add lifecycle primitives (spawn, exit, wait, signaling model if desired).

5. VFS and storage stack
- Define vnode/file interfaces for your OS.
- Implement an in-memory filesystem first.
- Add block-device backed FS later.

6. I/O model and driver policy
- Build device-manager policy on top of existing device graph.
- Define how drivers expose handles/resources to the rest of your OS.
- Add async I/O/event model if desired.

7. IPC and object model
- Pick message passing, shared memory, or handle-based model.
- Add permission and ownership model.

8. User-facing environment
- Add init task.
- Add shell/monitor or command interface.
- Add basic utilities and test programs.

9. Reliability and debug
- Add stronger panic dumps and post-mortem tooling.
- Add subsystem self-tests and boot test matrix.

## Suggested first toy milestone (minimal but real)

1. Boot to init loop.
2. Create 2 kernel tasks and switch between them.
3. Add one syscall beyond starter transport calls (for example `yield` or `print`).
4. Add simple in-memory VFS with `open/read/write/close` semantics for one pseudo-file.
5. Add a tiny shell command dispatcher.

If this works, you have a genuine OS core and can branch into your own design.

## Where to put your OS code

Suggested pattern:

1. Keep Monolith starter code mostly intact initially.
2. Add OS policy modules under a dedicated namespace (for example `kernel/os/` or `kernel/toyos/`).
3. Keep architecture specifics behind existing `arch_*`/HAL style interfaces.
4. Avoid scattering policy into arch files.

## Common pitfalls

1. Modifying boot ABI too early.
- Keep `boot_info_t` stable unless absolutely required.

2. Over-coupling policy to architecture.
- Keep architecture code as mechanisms, OS code as policy.

3. Skipping design docs.
- Write decisions down early: syscall model, scheduler policy, memory model, IPC model.

4. Expanding driver scope too fast.
- Start with serial/timer/storage baseline, then grow.

## “Done enough” checklist for a first OS prototype

- Boots cleanly on at least one architecture.
- Can run and switch multiple tasks.
- Has a defined syscall ABI and at least a few implemented calls.
- Has a minimal VFS and one backing storage path.
- Has repeatable tests/smoke checks.

## Mapping to manuals

Use this index while implementing:

- Boot and startup: `docs/manual/core/boot-sequence.md`
- CPU/interrupt/timer: `docs/manual/core/cpu-layer.md`, `exceptions.md`, `time-system.md`
- Memory: `docs/manual/core/memory.md`, `dma.md`, `iommu.md`
- Devices: `docs/manual/core/device-discovery.md`, `device-bus.md`, `device-model.md`, `pci-enumeration.md`, `usb-enumeration.md`
- Runtime: `docs/manual/core/scheduler.md`, `percpu.md`, `smp-bootstrap.md`
- Syscalls: `docs/manual/core/syscall-transport.md`
- Diagnostics: `docs/manual/core/observability.md`, `status-system.md`
