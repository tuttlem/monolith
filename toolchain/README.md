# Toolchain Notes

UEFI-first v1 uses:
- host `clang` + `lld-link` for `x86_64`
- host `clang` + `lld-link` for `arm64` via `--target=aarch64-windows-msvc`

No custom cross-toolchain build is required.
