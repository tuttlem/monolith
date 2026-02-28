ENTRY(_start)

SECTIONS {
  . = 0x80100000;

  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  __global_pointer$ = . + 0x7ff0;
  .data : { *(.data*) }
  .bss : {
    *(.bss*)
    *(COMMON)
  }
}
