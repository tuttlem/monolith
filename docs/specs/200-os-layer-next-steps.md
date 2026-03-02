# 200 OS Layer Next Steps (After Base Milestone)

This document captures what comes after the hardware substrate is complete.

Base milestone ends at stable hardware/platform foundations (CPU, MMU mapping, IRQ/timer, discovery, per-CPU/SMP skeleton, device baseline, test discipline).

Then OS-specific design begins.

## Why These Are Not Base-Layer Requirements

The following are strongly tied to OS policy and product goals:
- syscall ABI
- scheduler model
- VFS contract

Different operating systems will intentionally make different choices here.

## Recommended Next Steps

1. Syscall ABI skeleton
- start from [130-syscall-abi.md](130-syscall-abi.md)
- freeze ABI versioning and syscall-number policy early
- implement minimal `write` and `exit` path to validate userspace boundary

2. Scheduler scaffolding
- start from [140-scheduler-scaffolding.md](140-scheduler-scaffolding.md)
- define `task` model and context-switch hooks
- begin with single-task/idle and timer-driven hooks

3. VFS contract
- start from [150-vfs-contract.md](150-vfs-contract.md)
- define vnode/file interfaces before large filesystem work
- wire `sys_write` through the file layer to avoid syscall redesign later

## Suggested Implementation Order

1. syscall ABI (`130`)
2. scheduler scaffold (`140`)
3. VFS contract (`150`)

Rationale:
- you need a stable trap/ABI boundary first
- scheduler state and task context shape syscall execution model
- VFS design should align with FD/syscall semantics

## Branch Workflow

Use one branch per spec:
- `130-syscall-abi`
- `140-scheduler-scaffolding`
- `150-vfs-contract`

Per branch:
1. implement only scoped work
2. compile/test
3. merge when green

## Exit Criteria for "OS Off the Ground"

- syscall entry/dispatch exists and is versioned
- one kernel task model exists with deterministic execution control
- file abstraction exists and is callable from syscall path
- integration tests cover syscall dispatch + basic write path
