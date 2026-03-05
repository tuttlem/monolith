#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uintptr_t uptr;
typedef intptr_t sptr;

typedef size_t usize;
typedef ptrdiff_t isize;

_Static_assert(sizeof(u8) == 1, "u8 must be 8-bit");
_Static_assert(sizeof(u16) == 2, "u16 must be 16-bit");
_Static_assert(sizeof(u32) == 4, "u32 must be 32-bit");
_Static_assert(sizeof(u64) == 8, "u64 must be 64-bit");
_Static_assert(sizeof(uptr) == sizeof(void *), "uptr must match pointer width");

#endif
