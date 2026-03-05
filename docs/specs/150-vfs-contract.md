# 150 VFS Contract (Minimal)

## Goal

Freeze basic vnode/file operation contracts early, even with a fake in-memory backend.

## Core Types

```c
typedef enum {
  VNODE_DIR,
  VNODE_REG,
  VNODE_DEV,
} vnode_type_t;

typedef struct vnode vnode_t;
typedef struct file file_t;

typedef struct {
  status_t (*open)(vnode_t *vn, file_t *out);
  status_t (*read)(file_t *f, void *buf, u64 len, u64 *out_read);
  status_t (*write)(file_t *f, const void *buf, u64 len, u64 *out_written);
  status_t (*close)(file_t *f);
} file_ops_t;
```

## Phase Scope

- one in-memory root filesystem stub
- vnode lookup skeleton
- integration path for `sys_write`

## Rules

- no filesystem-specific types in syscall layer
- file descriptor table abstraction must be separate from filesystem backend

## Acceptance Criteria

- `sys_write` can target a VFS-backed object path
- interfaces are stable and documented before full FS work
