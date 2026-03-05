#include "input.h"

#define INPUT_QUEUE_CAPACITY 256U

typedef struct {
  u64 initialized;
  char queue[INPUT_QUEUE_CAPACITY];
  u64 head;
  u64 tail;
  u64 count;
  u64 dropped;
} input_state_t;

static input_state_t g_input;

status_t input_init(void) {
  g_input.initialized = 1ULL;
  g_input.head = 0ULL;
  g_input.tail = 0ULL;
  g_input.count = 0ULL;
  g_input.dropped = 0ULL;
  return STATUS_OK;
}

static status_t input_push_common(char ch) {
  if (g_input.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (g_input.count >= INPUT_QUEUE_CAPACITY) {
    g_input.dropped += 1ULL;
    return STATUS_BUSY;
  }
  g_input.queue[g_input.tail] = ch;
  g_input.tail = (g_input.tail + 1ULL) % INPUT_QUEUE_CAPACITY;
  g_input.count += 1ULL;
  return STATUS_OK;
}

status_t input_push_char(char ch) { return input_push_common(ch); }

status_t input_push_char_from_irq(char ch) { return input_push_common(ch); }

int input_try_pop_char(char *out_ch) {
  if (out_ch == (char *)0 || g_input.initialized == 0ULL || g_input.count == 0ULL) {
    return -1;
  }
  *out_ch = g_input.queue[g_input.head];
  g_input.head = (g_input.head + 1ULL) % INPUT_QUEUE_CAPACITY;
  g_input.count -= 1ULL;
  return 0;
}

u64 input_drop_count(void) { return g_input.dropped; }
