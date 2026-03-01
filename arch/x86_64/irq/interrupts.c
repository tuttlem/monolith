#include "arch_interrupts.h"
#include "interrupts.h"

typedef struct {
  unsigned short offset_lo;
  unsigned short selector;
  unsigned char ist;
  unsigned char type_attr;
  unsigned short offset_mid;
  unsigned int offset_hi;
  unsigned int reserved;
} __attribute__((packed)) x86_64_idt_entry_t;

typedef struct {
  unsigned short limit;
  BOOT_U64 base;
} __attribute__((packed)) x86_64_idtr_t;

static x86_64_idt_entry_t g_idt[256];
static x86_64_idtr_t g_idtr;

static unsigned short read_cs(void) {
  unsigned short cs;
  __asm__ volatile("mov %%cs, %0" : "=r"(cs));
  return cs;
}

static void lidt(const x86_64_idtr_t *idtr) { __asm__ volatile("lidt (%0)" : : "r"(idtr) : "memory"); }

static void set_gate(BOOT_U64 vector, void (*handler)(void), unsigned short cs) {
  BOOT_U64 addr = (BOOT_U64)(BOOT_UPTR)handler;
  x86_64_idt_entry_t *e;

  if (vector >= 256ULL) {
    return;
  }

  e = &g_idt[vector];
  e->offset_lo = (unsigned short)(addr & 0xFFFFULL);
  e->selector = cs;
  e->ist = 0;
  e->type_attr = 0x8E;
  e->offset_mid = (unsigned short)((addr >> 16) & 0xFFFFULL);
  e->offset_hi = (unsigned int)((addr >> 32) & 0xFFFFFFFFULL);
  e->reserved = 0;
}

__attribute__((used)) static void x86_64_exception_fatal(BOOT_U64 vector) {
  interrupt_frame_t frame;

  frame.arch_id = BOOT_INFO_ARCH_X86_64;
  frame.vector = vector;
  frame.error_code = 0;
  frame.fault_addr = 0;
  frame.ip = 0;
  frame.sp = 0;
  frame.flags = 0;
  interrupts_dispatch(&frame);

  for (;;) {
    __asm__ volatile("cli");
    arch_halt();
  }
}

#define X86_64_EXCEPTION_STUB(v)                                                                        \
  __attribute__((naked)) static void x86_64_exc_##v(void) {                                            \
    __asm__ volatile("movq $" #v ", %rcx\n\t"                                                          \
                     "subq $32, %rsp\n\t"                                                              \
                     "callq x86_64_exception_fatal\n\t"                                                \
                     "addq $32, %rsp\n\t"                                                              \
                     "1:\n\t"                                                                          \
                     "cli\n\t"                                                                         \
                     "hlt\n\t"                                                                         \
                     "jmp 1b\n\t");                                                                    \
  }

X86_64_EXCEPTION_STUB(0)
X86_64_EXCEPTION_STUB(1)
X86_64_EXCEPTION_STUB(2)
X86_64_EXCEPTION_STUB(3)
X86_64_EXCEPTION_STUB(4)
X86_64_EXCEPTION_STUB(5)
X86_64_EXCEPTION_STUB(6)
X86_64_EXCEPTION_STUB(7)
X86_64_EXCEPTION_STUB(8)
X86_64_EXCEPTION_STUB(9)
X86_64_EXCEPTION_STUB(10)
X86_64_EXCEPTION_STUB(11)
X86_64_EXCEPTION_STUB(12)
X86_64_EXCEPTION_STUB(13)
X86_64_EXCEPTION_STUB(14)
X86_64_EXCEPTION_STUB(15)
X86_64_EXCEPTION_STUB(16)
X86_64_EXCEPTION_STUB(17)
X86_64_EXCEPTION_STUB(18)
X86_64_EXCEPTION_STUB(19)
X86_64_EXCEPTION_STUB(20)
X86_64_EXCEPTION_STUB(21)
X86_64_EXCEPTION_STUB(22)
X86_64_EXCEPTION_STUB(23)
X86_64_EXCEPTION_STUB(24)
X86_64_EXCEPTION_STUB(25)
X86_64_EXCEPTION_STUB(26)
X86_64_EXCEPTION_STUB(27)
X86_64_EXCEPTION_STUB(28)
X86_64_EXCEPTION_STUB(29)
X86_64_EXCEPTION_STUB(30)
X86_64_EXCEPTION_STUB(31)

__attribute__((used)) static void x86_64_irq_dispatch_common(BOOT_U64 vector) {
  interrupt_frame_t frame;
  frame.arch_id = BOOT_INFO_ARCH_X86_64;
  frame.vector = vector;
  frame.error_code = 0;
  frame.fault_addr = 0;
  frame.ip = 0;
  frame.sp = 0;
  frame.flags = 0;
  interrupts_dispatch(&frame);
}

__attribute__((naked)) static void x86_64_irq_common_stub(void) {
  __asm__ volatile("pushq %rax\n\t"
                   "pushq %rbx\n\t"
                   "pushq %rcx\n\t"
                   "pushq %rdx\n\t"
                   "pushq %rsi\n\t"
                   "pushq %rdi\n\t"
                   "pushq %rbp\n\t"
                   "pushq %r8\n\t"
                   "pushq %r9\n\t"
                   "pushq %r10\n\t"
                   "pushq %r11\n\t"
                   "pushq %r12\n\t"
                   "pushq %r13\n\t"
                   "pushq %r14\n\t"
                   "pushq %r15\n\t"
                   "movq $255, %rcx\n\t"
                   "subq $32, %rsp\n\t"
                   "callq x86_64_irq_dispatch_common\n\t"
                   "addq $32, %rsp\n\t"
                   "popq %r15\n\t"
                   "popq %r14\n\t"
                   "popq %r13\n\t"
                   "popq %r12\n\t"
                   "popq %r11\n\t"
                   "popq %r10\n\t"
                   "popq %r9\n\t"
                   "popq %r8\n\t"
                   "popq %rbp\n\t"
                   "popq %rdi\n\t"
                   "popq %rsi\n\t"
                   "popq %rdx\n\t"
                   "popq %rcx\n\t"
                   "popq %rbx\n\t"
                   "popq %rax\n\t"
                   "iretq\n\t");
}

__attribute__((naked)) static void x86_64_irq_stub_32(void) {
  __asm__ volatile("pushq %rax\n\t"
                   "pushq %rbx\n\t"
                   "pushq %rcx\n\t"
                   "pushq %rdx\n\t"
                   "pushq %rsi\n\t"
                   "pushq %rdi\n\t"
                   "pushq %rbp\n\t"
                   "pushq %r8\n\t"
                   "pushq %r9\n\t"
                   "pushq %r10\n\t"
                   "pushq %r11\n\t"
                   "pushq %r12\n\t"
                   "pushq %r13\n\t"
                   "pushq %r14\n\t"
                   "pushq %r15\n\t"
                   "movq $32, %rcx\n\t"
                   "subq $32, %rsp\n\t"
                   "callq x86_64_irq_dispatch_common\n\t"
                   "addq $32, %rsp\n\t"
                   "popq %r15\n\t"
                   "popq %r14\n\t"
                   "popq %r13\n\t"
                   "popq %r12\n\t"
                   "popq %r11\n\t"
                   "popq %r10\n\t"
                   "popq %r9\n\t"
                   "popq %r8\n\t"
                   "popq %rbp\n\t"
                   "popq %rdi\n\t"
                   "popq %rsi\n\t"
                   "popq %rdx\n\t"
                   "popq %rcx\n\t"
                   "popq %rbx\n\t"
                   "popq %rax\n\t"
                   "iretq\n\t");
}

static void (*const g_exceptions[32])(void) = {
    x86_64_exc_0,  x86_64_exc_1,  x86_64_exc_2,  x86_64_exc_3,  x86_64_exc_4,  x86_64_exc_5,
    x86_64_exc_6,  x86_64_exc_7,  x86_64_exc_8,  x86_64_exc_9,  x86_64_exc_10, x86_64_exc_11,
    x86_64_exc_12, x86_64_exc_13, x86_64_exc_14, x86_64_exc_15, x86_64_exc_16, x86_64_exc_17,
    x86_64_exc_18, x86_64_exc_19, x86_64_exc_20, x86_64_exc_21, x86_64_exc_22, x86_64_exc_23,
    x86_64_exc_24, x86_64_exc_25, x86_64_exc_26, x86_64_exc_27, x86_64_exc_28, x86_64_exc_29,
    x86_64_exc_30, x86_64_exc_31};

status_t arch_interrupts_init(const boot_info_t *boot_info) {
  unsigned short cs;
  BOOT_U64 i;

  if (boot_info == (const boot_info_t *)0 || boot_info->arch_id != BOOT_INFO_ARCH_X86_64) {
    return STATUS_INVALID_ARG;
  }

  cs = read_cs();
  for (i = 0; i < 256ULL; ++i) {
    set_gate(i, x86_64_irq_common_stub, cs);
  }
  for (i = 0; i < 32ULL; ++i) {
    set_gate(i, g_exceptions[i], cs);
  }
  set_gate(32ULL, x86_64_irq_stub_32, cs);

  g_idtr.limit = (unsigned short)(sizeof(g_idt) - 1U);
  g_idtr.base = (BOOT_U64)(BOOT_UPTR)&g_idt[0];
  lidt(&g_idtr);
  __asm__ volatile("cli" : : : "memory");
  return STATUS_OK;
}

void arch_interrupts_enable(void) { __asm__ volatile("sti" : : : "memory"); }

void arch_interrupts_disable(void) { __asm__ volatile("cli" : : : "memory"); }

void arch_exception_selftest_trigger(void) {
  __asm__ volatile("ud2");
  for (;;) {
    arch_halt();
  }
}
