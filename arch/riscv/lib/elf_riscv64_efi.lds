/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * U-Boot riscv64 EFI linker script
 *
 * Modified from arch/arm/lib/elf_aarch64_efi.lds
 */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)

INCLUDE lib/efi_loader/elf_efi.ldsi
