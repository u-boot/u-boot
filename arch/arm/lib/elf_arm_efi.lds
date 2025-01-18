/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * U-Boot ARM EFI linker script
 *
 * Modified from elf_arm_efi.lds in gnu-efi
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

INCLUDE lib/efi_loader/elf_efi.ldsi
