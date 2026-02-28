OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(efi_main)

SECTIONS {
  . = 0;

  .text : { *(.text*) }
  .sdata : { *(.sdata*) }
  .data : { *(.data*) }
  .dynamic : { *(.dynamic*) }
  .dynsym : { *(.dynsym*) }
  .rel : { *(.rel*) }
  .rela : { *(.rela*) }
  .reloc : { *(.reloc) }

  /DISCARD/ : {
    *(.eh_frame)
    *(.note*)
    *(.comment)
  }
}
