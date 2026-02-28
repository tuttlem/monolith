X64_UEFI_CFLAGS := --target=x86_64-windows-msvc -ffreestanding -fshort-wchar -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-ident -Wall -Wextra -Werror -Ikernel/include -DCORE_ARCH_NAME=\"UEFI\ x86_64\"
X64_UEFI_LDFLAGS := /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64
