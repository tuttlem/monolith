ENTRY(_start)

SECTIONS {
  . = 0x80200000;

  .text : {
    KEEP(*(.text._start))
    *(.text*)
  }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : {
    *(.bss*)
    *(COMMON)
  }
}
