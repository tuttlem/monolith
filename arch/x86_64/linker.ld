OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
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
