#ifndef KERNEL_DRIVER_QUEUE_H
#define KERNEL_DRIVER_QUEUE_H

#include "kernel.h"

typedef struct {
  BOOT_U64 capacity;
  BOOT_U64 head;
  BOOT_U64 tail;
} driver_ring_t;

static inline void driver_ring_init(driver_ring_t *ring, BOOT_U64 capacity) {
  if (ring == (driver_ring_t *)0) {
    return;
  }
  ring->capacity = capacity;
  ring->head = 0;
  ring->tail = 0;
}

static inline BOOT_U64 driver_ring_count(const driver_ring_t *ring) {
  if (ring == (const driver_ring_t *)0 || ring->capacity == 0ULL) {
    return 0ULL;
  }
  return (ring->tail + ring->capacity - ring->head) % ring->capacity;
}

static inline int driver_ring_push(driver_ring_t *ring, BOOT_U64 *out_slot) {
  BOOT_U64 next_tail;
  if (ring == (driver_ring_t *)0 || ring->capacity < 2ULL) {
    return 0;
  }
  next_tail = (ring->tail + 1ULL) % ring->capacity;
  if (next_tail == ring->head) {
    return 0;
  }
  if (out_slot != (BOOT_U64 *)0) {
    *out_slot = ring->tail;
  }
  ring->tail = next_tail;
  return 1;
}

static inline int driver_ring_pop(driver_ring_t *ring, BOOT_U64 *out_slot) {
  if (ring == (driver_ring_t *)0 || ring->capacity == 0ULL) {
    return 0;
  }
  if (ring->head == ring->tail) {
    return 0;
  }
  if (out_slot != (BOOT_U64 *)0) {
    *out_slot = ring->head;
  }
  ring->head = (ring->head + 1ULL) % ring->capacity;
  return 1;
}

#endif
