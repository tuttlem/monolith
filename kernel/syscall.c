#include "syscall.h"
#include "arch_syscall.h"
#include "interrupts.h"
#include "personality.h"
#include "print.h"
#include "timebase.h"

#define SYSCALL_MAX_HANDLERS 64U
#define SYSCALL_DEBUG_LOG_MAX 255U

typedef struct {
  BOOT_U64 op;
  syscall_handler_t handler;
  const char *owner;
} syscall_slot_t;

static struct {
  BOOT_U64 initialized;
  BOOT_U64 arch_id;
  BOOT_U64 abi_version;
  BOOT_U64 feature_bits;
  syscall_slot_t slots[SYSCALL_MAX_HANDLERS];
  BOOT_U64 slot_count;
  BOOT_U64 trap_vector;
} g_syscall;

static const char g_syscall_owner_core[] = "core";
static const char g_syscall_owner_trap[] = "syscall-trap";

static const personality_ops_t *g_personalities[PERSONALITY_MAX];
static BOOT_U64 g_personality_count;
static personality_id_t g_active_personality = (personality_id_t)(~0ULL);

static struct {
  BOOT_U64 active;
  syscall_request_t req;
  syscall_response_t resp;
} g_syscall_trap_mailbox;

status_t syscall_trap_mailbox_consume(void) {
  status_t st;
  if (g_syscall_trap_mailbox.active == 0ULL) {
    return STATUS_DEFERRED;
  }
  st = syscall_dispatch(&g_syscall_trap_mailbox.req, &g_syscall_trap_mailbox.resp);
  if (st != STATUS_OK && g_syscall_trap_mailbox.resp.status == STATUS_OK) {
    g_syscall_trap_mailbox.resp.status = st;
  }
  g_syscall_trap_mailbox.active = 0ULL;
  return st;
}

static void syscall_trap_handler(const interrupt_frame_t *frame, void *ctx) {
  (void)frame;
  (void)ctx;
  (void)syscall_trap_mailbox_consume();
}

static status_t syscall_handle_abi_info(const syscall_request_t *req, syscall_response_t *resp) {
  (void)req;
  resp->status = STATUS_OK;
  resp->value = syscall_abi_info_word();
  return STATUS_OK;
}

static status_t syscall_handle_debug_log(const syscall_request_t *req, syscall_response_t *resp) {
  const char *src;
  BOOT_U64 i;
  BOOT_U64 len;
  char line[SYSCALL_DEBUG_LOG_MAX + 1U];

  if (req == (const syscall_request_t *)0 || resp == (syscall_response_t *)0) {
    return STATUS_INVALID_ARG;
  }

  src = (const char *)(BOOT_UPTR)req->args[0];
  len = req->args[1];
  if (src == (const char *)0) {
    resp->status = STATUS_INVALID_ARG;
    resp->value = 0;
    return STATUS_INVALID_ARG;
  }
  if (len > SYSCALL_DEBUG_LOG_MAX) {
    len = SYSCALL_DEBUG_LOG_MAX;
  }
  for (i = 0; i < len; ++i) {
    char ch = src[i];
    line[i] = ch;
    if (ch == '\0') {
      break;
    }
  }
  if (i == len) {
    line[len] = '\0';
  } else if (line[i] != '\0') {
    line[i + 1U] = '\0';
  }

  kprintf("sys_debug: %s\n", line);
  resp->status = STATUS_OK;
  resp->value = i;
  return STATUS_OK;
}

static status_t syscall_handle_time_now(const syscall_request_t *req, syscall_response_t *resp) {
  (void)req;
  resp->status = STATUS_OK;
  resp->value = time_now_ns();
  return STATUS_OK;
}

void syscall_reset(void) {
  BOOT_U64 i;
  g_syscall.initialized = 0;
  g_syscall.arch_id = BOOT_INFO_ARCH_UNKNOWN;
  g_syscall.abi_version = SYSCALL_TRANSPORT_ABI_VERSION;
  g_syscall.feature_bits = 0;
  g_syscall.slot_count = 0;
  g_syscall.trap_vector = 0;
  g_syscall_trap_mailbox.active = 0;
  g_personality_count = 0;
  g_active_personality = (personality_id_t)(~0ULL);
  for (i = 0; i < SYSCALL_MAX_HANDLERS; ++i) {
    g_syscall.slots[i].op = 0;
    g_syscall.slots[i].handler = (syscall_handler_t)0;
    g_syscall.slots[i].owner = (const char *)0;
  }
  for (i = 0; i < PERSONALITY_MAX; ++i) {
    g_personalities[i] = (const personality_ops_t *)0;
  }
}

status_t syscall_register(BOOT_U64 op, syscall_handler_t handler, const char *owner) {
  BOOT_U64 i;

  if (g_syscall.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (handler == (syscall_handler_t)0) {
    return STATUS_INVALID_ARG;
  }
  for (i = 0; i < g_syscall.slot_count; ++i) {
    if (g_syscall.slots[i].op == op) {
      return STATUS_BUSY;
    }
  }
  if (g_syscall.slot_count >= SYSCALL_MAX_HANDLERS) {
    return STATUS_NO_MEMORY;
  }

  g_syscall.slots[g_syscall.slot_count].op = op;
  g_syscall.slots[g_syscall.slot_count].handler = handler;
  g_syscall.slots[g_syscall.slot_count].owner = (owner == (const char *)0) ? g_syscall_owner_core : owner;
  g_syscall.slot_count += 1ULL;
  return STATUS_OK;
}

status_t syscall_init(const boot_info_t *boot_info) {
  status_t st;
  BOOT_U64 trap_vector = 0;

  if (boot_info == (const boot_info_t *)0) {
    return STATUS_INVALID_ARG;
  }

  syscall_reset();
  g_syscall.arch_id = boot_info->arch_id;
  g_syscall.initialized = 1ULL;

  st = arch_syscall_init(boot_info);
  if (st != STATUS_OK && st != STATUS_DEFERRED) {
    g_syscall.initialized = 0ULL;
    return st;
  }
  if (st == STATUS_OK) {
    st = arch_syscall_get_vector(&trap_vector);
    if (st != STATUS_OK) {
      g_syscall.initialized = 0ULL;
      return st;
    }
    st = interrupts_register_handler_owned(trap_vector, syscall_trap_handler, (void *)0, g_syscall_owner_trap);
    if (st != STATUS_OK && st != STATUS_DEFERRED) {
      g_syscall.initialized = 0ULL;
      return st;
    }
    if (st == STATUS_OK) {
      g_syscall.feature_bits |= SYSCALL_ABI_FEATURE_TRAP_ENTRY;
      g_syscall.trap_vector = trap_vector;
    }
  }

  st = syscall_register(SYSCALL_OP_ABI_INFO, syscall_handle_abi_info, g_syscall_owner_core);
  if (st != STATUS_OK) {
    return st;
  }
  st = syscall_register(SYSCALL_OP_DEBUG_LOG, syscall_handle_debug_log, g_syscall_owner_core);
  if (st != STATUS_OK) {
    return st;
  }
  st = syscall_register(SYSCALL_OP_TIME_NOW, syscall_handle_time_now, g_syscall_owner_core);
  if (st != STATUS_OK) {
    return st;
  }

  return STATUS_OK;
}

status_t syscall_dispatch(const syscall_request_t *req, syscall_response_t *resp) {
  BOOT_U64 i;

  if (g_syscall.initialized == 0ULL) {
    return STATUS_DEFERRED;
  }
  if (req == (const syscall_request_t *)0 || resp == (syscall_response_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (req->abi_version != g_syscall.abi_version) {
    resp->status = STATUS_NOT_SUPPORTED;
    resp->value = g_syscall.abi_version;
    return STATUS_NOT_SUPPORTED;
  }

  for (i = 0; i < g_syscall.slot_count; ++i) {
    if (g_syscall.slots[i].op == req->op) {
      status_t st = g_syscall.slots[i].handler(req, resp);
      return st;
    }
  }

  resp->status = STATUS_NOT_SUPPORTED;
  resp->value = 0;
  return STATUS_NOT_SUPPORTED;
}

status_t syscall_invoke_kernel(BOOT_U64 op, BOOT_U64 arg0, BOOT_U64 arg1, BOOT_U64 arg2, BOOT_U64 arg3, BOOT_U64 arg4,
                               BOOT_U64 arg5, syscall_response_t *resp) {
  syscall_request_t req;

  if (resp == (syscall_response_t *)0) {
    return STATUS_INVALID_ARG;
  }

  req.abi_version = g_syscall.abi_version;
  req.op = op;
  req.args[0] = arg0;
  req.args[1] = arg1;
  req.args[2] = arg2;
  req.args[3] = arg3;
  req.args[4] = arg4;
  req.args[5] = arg5;
  req.arch_id = g_syscall.arch_id;
  req.flags = 0;
  return syscall_dispatch(&req, resp);
}

status_t syscall_invoke_trap(BOOT_U64 op, BOOT_U64 arg0, BOOT_U64 arg1, BOOT_U64 arg2, BOOT_U64 arg3, BOOT_U64 arg4,
                             BOOT_U64 arg5, syscall_response_t *resp) {
  BOOT_U64 spin;

  if (resp == (syscall_response_t *)0) {
    return STATUS_INVALID_ARG;
  }
  if (!syscall_trap_entry_ready()) {
    return STATUS_DEFERRED;
  }
  if (g_syscall_trap_mailbox.active != 0ULL) {
    return STATUS_BUSY;
  }

  g_syscall_trap_mailbox.active = 1ULL;
  g_syscall_trap_mailbox.req.abi_version = g_syscall.abi_version;
  g_syscall_trap_mailbox.req.op = op;
  g_syscall_trap_mailbox.req.args[0] = arg0;
  g_syscall_trap_mailbox.req.args[1] = arg1;
  g_syscall_trap_mailbox.req.args[2] = arg2;
  g_syscall_trap_mailbox.req.args[3] = arg3;
  g_syscall_trap_mailbox.req.args[4] = arg4;
  g_syscall_trap_mailbox.req.args[5] = arg5;
  g_syscall_trap_mailbox.req.arch_id = g_syscall.arch_id;
  g_syscall_trap_mailbox.req.flags = 0;
  g_syscall_trap_mailbox.resp.status = STATUS_DEFERRED;
  g_syscall_trap_mailbox.resp.value = 0;

  if (arch_syscall_trigger() != STATUS_OK) {
    g_syscall_trap_mailbox.active = 0ULL;
    return STATUS_FAULT;
  }
  for (spin = 0; spin < 1000000ULL; ++spin) {
    if (g_syscall_trap_mailbox.active == 0ULL) {
      break;
    }
    __asm__ volatile("" : : : "memory");
  }
  if (g_syscall_trap_mailbox.active != 0ULL) {
    g_syscall_trap_mailbox.active = 0ULL;
    return STATUS_TRY_AGAIN;
  }
  resp->status = g_syscall_trap_mailbox.resp.status;
  resp->value = g_syscall_trap_mailbox.resp.value;
  return resp->status;
}

const char *syscall_owner(BOOT_U64 op) {
  BOOT_U64 i;
  if (g_syscall.initialized == 0ULL) {
    return (const char *)0;
  }
  for (i = 0; i < g_syscall.slot_count; ++i) {
    if (g_syscall.slots[i].op == op) {
      return g_syscall.slots[i].owner;
    }
  }
  return (const char *)0;
}

const char *syscall_op_name(BOOT_U64 op) {
  switch (op) {
  case SYSCALL_OP_ABI_INFO:
    return "abi_info";
  case SYSCALL_OP_DEBUG_LOG:
    return "debug_log";
  case SYSCALL_OP_TIME_NOW:
    return "time_now";
  default:
    return "unknown";
  }
}

BOOT_U64 syscall_abi_info_word(void) {
  BOOT_U64 value = 0;
  value |= (g_syscall.abi_version & 0xFFFFULL);
  value |= ((g_syscall.arch_id & 0xFFULL) << 16);
  value |= ((g_syscall.feature_bits & 0xFFFFULL) << 24);
  return value;
}

int syscall_trap_entry_ready(void) {
  return (g_syscall.feature_bits & SYSCALL_ABI_FEATURE_TRAP_ENTRY) != 0ULL;
}

int syscall_trap_mailbox_active(void) { return g_syscall_trap_mailbox.active != 0ULL; }

status_t personality_register(const personality_ops_t *ops) {
  BOOT_U64 i;

  if (ops == (const personality_ops_t *)0 || ops->name == (const char *)0) {
    return STATUS_INVALID_ARG;
  }
  for (i = 0; i < g_personality_count; ++i) {
    if (g_personalities[i] != (const personality_ops_t *)0 && g_personalities[i]->id == ops->id) {
      return STATUS_BUSY;
    }
  }
  if (g_personality_count >= PERSONALITY_MAX) {
    return STATUS_NO_MEMORY;
  }
  g_personalities[g_personality_count] = ops;
  g_personality_count += 1ULL;
  return STATUS_OK;
}

status_t personality_activate(personality_id_t id) {
  BOOT_U64 i;

  for (i = 0; i < g_personality_count; ++i) {
    const personality_ops_t *ops = g_personalities[i];
    if (ops != (const personality_ops_t *)0 && ops->id == id) {
      if (ops->on_activate != (status_t (*)(void))0) {
        status_t st = ops->on_activate();
        if (st != STATUS_OK) {
          return st;
        }
      }
      g_active_personality = id;
      return STATUS_OK;
    }
  }
  return STATUS_NOT_FOUND;
}

status_t personality_exec(const exec_image_t *img, exec_result_t *out) {
  BOOT_U64 i;

  if (img == (const exec_image_t *)0 || out == (exec_result_t *)0) {
    return STATUS_INVALID_ARG;
  }
  for (i = 0; i < g_personality_count; ++i) {
    const personality_ops_t *ops = g_personalities[i];
    if (ops != (const personality_ops_t *)0 && ops->id == g_active_personality) {
      if (ops->on_exec == (status_t (*)(const exec_image_t *, exec_result_t *))0) {
        return STATUS_NOT_SUPPORTED;
      }
      return ops->on_exec(img, out);
    }
  }
  return STATUS_DEFERRED;
}

personality_id_t personality_active_id(void) { return g_active_personality; }

const char *personality_active_name(void) {
  BOOT_U64 i;
  for (i = 0; i < g_personality_count; ++i) {
    const personality_ops_t *ops = g_personalities[i];
    if (ops != (const personality_ops_t *)0 && ops->id == g_active_personality) {
      return ops->name;
    }
  }
  return "<none>";
}

void syscall_dump_table(void) {
  BOOT_U64 i;

  if (g_syscall.initialized == 0ULL) {
    kprintf("syscall: not initialized\n");
    return;
  }

  kprintf("syscall: abi=%llu trap=%u handlers=%llu\n", g_syscall.abi_version, (unsigned)syscall_trap_entry_ready(),
          g_syscall.slot_count);
  for (i = 0; i < g_syscall.slot_count; ++i) {
    kprintf("  op=0x%llx name=%s owner=%s\n", g_syscall.slots[i].op, syscall_op_name(g_syscall.slots[i].op),
            g_syscall.slots[i].owner == (const char *)0 ? "<none>" : g_syscall.slots[i].owner);
  }
}
