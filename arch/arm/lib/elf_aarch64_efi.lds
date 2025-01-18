/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * U-Boot aarch64 EFI linker script
 *
 * Modified from elf_aarch64_efi.lds in gnu-efi
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

INCLUDE lib/efi_loader/elf_efi.ldsi
