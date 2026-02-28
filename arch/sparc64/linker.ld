ENTRY(_start)

SECTIONS {
  . = 0x00004000;

  .text : {
    KEEP(*(.text.start))
    *(.text*)
  }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : {
    *(.bss*)
    *(COMMON)
  }
}
