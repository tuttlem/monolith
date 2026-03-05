# User Memory Access (`uaccess`)

This module provides shared checked-copy helpers for user-pointer access from generic kernel code.

## APIs

- `status_t uaccess_set_user_window(u64 base, u64 size)`
- `status_t copy_from_user_checked(void *dst, u64 user_src, u64 len)`
- `status_t copy_to_user_checked(u64 user_dst, const void *src, u64 len)`

## Rules

- Range checks reject overflow and out-of-window pointers.
- Invalid user pointers fail with deterministic `STATUS_FAULT`.
- Callers should never dereference raw user pointers directly.

## Typical Flow

```c
uaccess_set_user_window(user_base, user_size);
st = copy_from_user_checked(buf, user_ptr, len);
if (st != STATUS_OK) {
  return st;
}
```
