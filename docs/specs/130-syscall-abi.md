# 130 Syscall ABI Skeleton

## Goal

Define a minimal, versioned syscall boundary early so higher-level kernel/user experiments stay stable.

## Phase Scope

- ABI version tag
- syscall number table
- dispatch path
- two baseline calls: `write`, `exit`

## Required Components

1. architecture trap/entry glue for syscalls
2. generic syscall dispatch table
3. per-call validation and status return conventions

## ABI Design Rules

- explicit calling convention per architecture documented
- stable syscall number assignments
- error return mapping policy defined (`status_t` to userspace error model later)

## Minimal Call Set

- `sys_write(fd, buf, len)` (initially can target debug console only)
- `sys_exit(code)`

## Acceptance Criteria

- syscall entry path tested on all architectures
- invalid syscall id returns deterministic error
- ABI version exposed to userland bootstrap code
