A64_UEFI_CFLAGS := --target=aarch64-windows-msvc -ffreestanding -fshort-wchar -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-ident -Wall -Wextra -Werror -Ikernel/include -DCORE_ARCH_NAME=\"UEFI\ arm64\"
A64_UEFI_LDFLAGS := /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:arm64
