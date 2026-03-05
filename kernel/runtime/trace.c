#include "trace.h"
#include "arch_cpu.h"
#include "config.h"
#include "print.h"
#include "timebase.h"

#define TRACE_RING_CAPACITY 256U

typedef struct {
  u64 initialized;
  u64 enabled;
  u64 write_seq;
  u64 dropped;
  trace_record_t ring[TRACE_RING_CAPACITY];
} trace_state_t;

static trace_state_t g_trace;

status_t trace_init(const boot_info_t *boot_info) {
  u64 i;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  g_trace.initialized = 1ULL;
#if MONOLITH_FEATURE_TRACE
  g_trace.enabled = 1ULL;
#else
  g_trace.enabled = 0ULL;
#endif
  g_trace.write_seq = 0ULL;
  g_trace.dropped = 0ULL;
  for (i = 0; i < TRACE_RING_CAPACITY; ++i) {
    g_trace.ring[i].seq = 0ULL;
    g_trace.ring[i].timestamp_ns = 0ULL;
    g_trace.ring[i].cpu_id = 0ULL;
    g_trace.ring[i].cls = 0;
    g_trace.ring[i].a0 = 0ULL;
    g_trace.ring[i].a1 = 0ULL;
    g_trace.ring[i].a2 = 0ULL;
  }
  return STATUS_OK;
}

void trace_emit(trace_class_t cls, u64 a0, u64 a1, u64 a2) {
  u64 idx;
  trace_record_t rec;

  if (g_trace.initialized == 0ULL || g_trace.enabled == 0ULL) {
    return;
  }

  rec.seq = __atomic_add_fetch(&g_trace.write_seq, 1ULL, __ATOMIC_RELAXED);
  rec.timestamp_ns = time_now_ns();
  rec.cpu_id = arch_cpu_id();
  rec.cls = cls;
  rec.a0 = a0;
  rec.a1 = a1;
  rec.a2 = a2;

  idx = (rec.seq - 1ULL) % TRACE_RING_CAPACITY;
  g_trace.ring[idx].seq = rec.seq;
  g_trace.ring[idx].timestamp_ns = rec.timestamp_ns;
  g_trace.ring[idx].cpu_id = rec.cpu_id;
  g_trace.ring[idx].cls = rec.cls;
  g_trace.ring[idx].a0 = rec.a0;
  g_trace.ring[idx].a1 = rec.a1;
  g_trace.ring[idx].a2 = rec.a2;
}

void trace_sink_kprintf(const trace_record_t *record) {
  if (record == (const trace_record_t *)0) {
    return;
  }
  kprintf("trace: seq=%llu ns=%llu cpu=%llu cls=%llu a0=0x%llx a1=0x%llx a2=0x%llx\n", record->seq,
          record->timestamp_ns, record->cpu_id, (u64)record->cls, record->a0, record->a1, record->a2);
}

status_t trace_dump(trace_sink_t sink) {
  u64 total;
  u64 start;
  u64 i;

  if (g_trace.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (sink == (trace_sink_t)0) {
    return STATUS_INVALID_ARG;
  }
  if (g_trace.enabled == 0ULL) {
    return STATUS_DEFERRED;
  }

  total = g_trace.write_seq;
  if (total > TRACE_RING_CAPACITY) {
    start = total - TRACE_RING_CAPACITY + 1ULL;
  } else {
    start = 1ULL;
  }

  for (i = start; i <= total; ++i) {
    u64 idx = (i - 1ULL) % TRACE_RING_CAPACITY;
    if (g_trace.ring[idx].seq != i) {
      g_trace.dropped += 1ULL;
      continue;
    }
    sink(&g_trace.ring[idx]);
  }
  return STATUS_OK;
}
