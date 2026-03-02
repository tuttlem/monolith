#include "discovery_internal.h"

typedef struct {
  BOOT_U32 magic;
  BOOT_U32 totalsize;
  BOOT_U32 off_dt_struct;
  BOOT_U32 off_dt_strings;
  BOOT_U32 off_mem_rsvmap;
  BOOT_U32 version;
  BOOT_U32 last_comp_version;
  BOOT_U32 boot_cpuid_phys;
  BOOT_U32 size_dt_strings;
  BOOT_U32 size_dt_struct;
} fdt_header_t;

#define FDT_MAGIC 0xd00dfeedU
#define FDT_BEGIN_NODE 1U
#define FDT_END_NODE 2U
#define FDT_PROP 3U
#define FDT_NOP 4U
#define FDT_END 9U

static BOOT_U32 be32_at(const unsigned char *p) {
  return ((BOOT_U32)p[0] << 24) | ((BOOT_U32)p[1] << 16) | ((BOOT_U32)p[2] << 8) | (BOOT_U32)p[3];
}

static BOOT_U64 be64_at(const unsigned char *p) {
  BOOT_U64 hi = (BOOT_U64)be32_at(p);
  BOOT_U64 lo = (BOOT_U64)be32_at(p + 4);
  return (hi << 32) | lo;
}

static BOOT_U32 align4(BOOT_U32 v) { return (v + 3U) & ~3U; }

#define DTB_MAX_DEPTH 64U

typedef struct {
  BOOT_U64 last_reg_base;
  BOOT_U64 last_reg_size;
  BOOT_U64 last_irq;
  BOOT_U64 node_is_cpu;
  BOOT_U64 node_is_timer;
  BOOT_U64 node_is_uart;
  BOOT_U64 node_is_irqc;
  BOOT_U64 node_irqc_type;
  BOOT_U64 cpu_id;
} dtb_node_frame_t;

static dtb_node_frame_t g_dtb_frames[DTB_MAX_DEPTH];

static int str_eq(const char *a, const char *b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static int str_starts_with(const char *s, const char *prefix) {
  while (*prefix != '\0') {
    if (*s++ != *prefix++) {
      return 0;
    }
  }
  return 1;
}

static BOOT_U64 parse_hex_suffix_u64(const char *s) {
  BOOT_U64 v = 0;
  while (*s != '\0') {
    char c = *s++;
    if (c >= '0' && c <= '9') {
      v = (v << 4) | (BOOT_U64)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      v = (v << 4) | (BOOT_U64)(10 + c - 'a');
    } else if (c >= 'A' && c <= 'F') {
      v = (v << 4) | (BOOT_U64)(10 + c - 'A');
    } else {
      break;
    }
  }
  return v;
}

static BOOT_U64 read_reg64(const unsigned char *p, BOOT_U32 len, BOOT_U64 *out_size) {
  if (len >= 16U) {
    *out_size = be64_at(p + 8);
    return be64_at(p);
  }
  if (len >= 8U) {
    *out_size = (BOOT_U64)be32_at(p + 4);
    return (BOOT_U64)be32_at(p);
  }
  *out_size = 0;
  return 0;
}

static void add_cpu(hw_desc_t *desc, BOOT_U64 cpu_id) {
  if (desc->cpu_count >= HW_DESC_MAX_CPUS) {
    return;
  }
  desc->cpus[desc->cpu_count].cpu_id = cpu_id;
  desc->cpus[desc->cpu_count].flags = 0;
  desc->cpu_count += 1ULL;
}

static void add_timer(hw_desc_t *desc, BOOT_U64 irq) {
  if (desc->timer_count >= HW_DESC_MAX_TIMERS) {
    return;
  }
  desc->timers[desc->timer_count].mmio_base = 0;
  desc->timers[desc->timer_count].mmio_size = 0;
  desc->timers[desc->timer_count].irq = irq;
  desc->timers[desc->timer_count].freq_hz = 0;
  desc->timers[desc->timer_count].flags = 0;
  desc->timer_count += 1ULL;
}

static void add_irqc(hw_desc_t *desc, BOOT_U64 type, BOOT_U64 base, BOOT_U64 size) {
  if (desc->irq_controller_count >= HW_DESC_MAX_IRQ_CONTROLLERS) {
    return;
  }
  desc->irq_controllers[desc->irq_controller_count].type = type;
  desc->irq_controllers[desc->irq_controller_count].mmio_base = base;
  desc->irq_controllers[desc->irq_controller_count].mmio_size = size;
  desc->irq_controllers[desc->irq_controller_count].irq_base = 0;
  desc->irq_controllers[desc->irq_controller_count].irq_count = 0;
  desc->irq_controller_count += 1ULL;
}

static void add_uart(hw_desc_t *desc, BOOT_U64 base, BOOT_U64 irq) {
  if (desc->uart_count >= HW_DESC_MAX_UARTS) {
    return;
  }
  desc->uarts[desc->uart_count].base = base;
  desc->uarts[desc->uart_count].irq = irq;
  desc->uarts[desc->uart_count].flags = 0;
  desc->uart_count += 1ULL;
}

void hw_discovery_parse_dtb(const boot_info_t *boot_info, hw_desc_t *desc) {
  const fdt_header_t *hdr;
  const unsigned char *blob;
  const unsigned char *struct_base;
  const unsigned char *struct_end;
  const char *strings_base;
  const unsigned char *p;
  BOOT_U32 off_struct;
  BOOT_U32 off_strings;
  BOOT_U32 size_struct;
  BOOT_U32 depth = 0;
  dtb_node_frame_t *frames = g_dtb_frames;

  if (boot_info == (const boot_info_t *)0 || desc == (hw_desc_t *)0 || boot_info->dtb_ptr == 0) {
    return;
  }

  hdr = (const fdt_header_t *)(BOOT_UPTR)boot_info->dtb_ptr;
  blob = (const unsigned char *)hdr;
  if (be32_at((const unsigned char *)&hdr->magic) != FDT_MAGIC) {
    return;
  }

  off_struct = be32_at((const unsigned char *)&hdr->off_dt_struct);
  off_strings = be32_at((const unsigned char *)&hdr->off_dt_strings);
  size_struct = be32_at((const unsigned char *)&hdr->size_dt_struct);
  struct_base = blob + off_struct;
  struct_end = struct_base + size_struct;
  strings_base = (const char *)(blob + off_strings);
  p = struct_base;
  for (depth = 0; depth < DTB_MAX_DEPTH; ++depth) {
    frames[depth].last_reg_base = 0;
    frames[depth].last_reg_size = 0;
    frames[depth].last_irq = 0;
    frames[depth].node_is_cpu = 0;
    frames[depth].node_is_timer = 0;
    frames[depth].node_is_uart = 0;
    frames[depth].node_is_irqc = 0;
    frames[depth].node_irqc_type = HW_IRQ_CONTROLLER_UNKNOWN;
    frames[depth].cpu_id = 0;
  }
  depth = 0;

  while (p + 4 <= struct_end) {
    BOOT_U32 token = be32_at(p);
    p += 4;

    if (token == FDT_BEGIN_NODE) {
      const char *name = (const char *)p;
      while (p < struct_end && *p != '\0') {
        ++p;
      }
      if (p >= struct_end) {
        break;
      }
      ++p;
      p = struct_base + align4((BOOT_U32)(p - struct_base));
      if (depth >= DTB_MAX_DEPTH) {
        break;
      }
      frames[depth].node_is_cpu = (str_eq(name, "cpu") || str_starts_with(name, "cpu@")) ? 1ULL : 0ULL;
      frames[depth].node_is_timer = (str_eq(name, "timer") || str_starts_with(name, "timer@")) ? 1ULL : 0ULL;
      frames[depth].node_is_uart = (str_starts_with(name, "uart") || str_starts_with(name, "serial")) ? 1ULL : 0ULL;
      frames[depth].node_is_irqc = 0ULL;
      frames[depth].node_irqc_type = HW_IRQ_CONTROLLER_UNKNOWN;
      if (str_starts_with(name, "plic")) {
        frames[depth].node_irqc_type = HW_IRQ_CONTROLLER_RISCV_PLIC;
      } else if (str_starts_with(name, "gic")) {
        frames[depth].node_irqc_type = HW_IRQ_CONTROLLER_ARM_GIC;
      }
      if (str_starts_with(name, "cpu@")) {
        frames[depth].cpu_id = parse_hex_suffix_u64(name + 4);
      }
      frames[depth].last_reg_base = 0;
      frames[depth].last_reg_size = 0;
      frames[depth].last_irq = 0;
      ++depth;
      continue;
    }

    if (token == FDT_END_NODE) {
      if (depth == 0U) {
        break;
      }
      --depth;
      if (frames[depth].node_is_cpu != 0ULL) {
        add_cpu(desc, frames[depth].cpu_id);
      }
      if (frames[depth].node_is_timer != 0ULL) {
        add_timer(desc, frames[depth].last_irq);
      }
      if (frames[depth].node_is_irqc != 0ULL) {
        add_irqc(desc,
                 frames[depth].node_irqc_type == HW_IRQ_CONTROLLER_UNKNOWN ? HW_IRQ_CONTROLLER_UNKNOWN : frames[depth].node_irqc_type,
                 frames[depth].last_reg_base, frames[depth].last_reg_size);
      }
      if (frames[depth].node_is_uart != 0ULL && frames[depth].last_reg_base != 0ULL) {
        add_uart(desc, frames[depth].last_reg_base, frames[depth].last_irq);
      }
      continue;
    }

    if (token == FDT_PROP) {
      BOOT_U32 len;
      BOOT_U32 nameoff;
      const char *prop_name;
      if (p + 8 > struct_end) {
        break;
      }
      len = be32_at(p);
      nameoff = be32_at(p + 4);
      p += 8;
      prop_name = strings_base + nameoff;

      if (depth == 0U) {
        break;
      }
      if (str_eq(prop_name, "reg")) {
        frames[depth - 1U].last_reg_base = read_reg64(p, len, &frames[depth - 1U].last_reg_size);
        if (frames[depth - 1U].node_is_cpu != 0ULL) {
          frames[depth - 1U].cpu_id = frames[depth - 1U].last_reg_base;
        }
      } else if (str_eq(prop_name, "interrupts")) {
        if (len >= 4U) {
          frames[depth - 1U].last_irq = (BOOT_U64)be32_at(p);
        }
      } else if (str_eq(prop_name, "interrupt-controller")) {
        frames[depth - 1U].node_is_irqc = 1ULL;
      } else if (str_eq(prop_name, "compatible")) {
        if (len >= 4U) {
          const char *s = (const char *)p;
          if (str_starts_with(s, "riscv,plic")) {
            frames[depth - 1U].node_irqc_type = HW_IRQ_CONTROLLER_RISCV_PLIC;
          } else if (str_starts_with(s, "arm,gic")) {
            frames[depth - 1U].node_irqc_type = HW_IRQ_CONTROLLER_ARM_GIC;
          }
        }
      }
      p += align4(len);
      continue;
    }

    if (token == FDT_NOP) {
      continue;
    }
    if (token == FDT_END) {
      break;
    }
    break;
  }

  desc->source_mask |= HW_DESC_SOURCE_DTB;
}
