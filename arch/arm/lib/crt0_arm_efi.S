/* SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause */
/*
 * crt0-efi-arm.S - PE/COFF header for ARM EFI applications
 *
 * Copright (C) 2014 Linaro Ltd. <ard.biesheuvel@linaro.org>
 *
 * This file is taken and modified from the gnu-efi project.
 */

#include <asm-generic/pe.h>

	.section	.text.head

	/*
	 * Magic "MZ" signature for PE/COFF
	 */
	.globl	image_base
image_base:
	.short	IMAGE_DOS_SIGNATURE		/* 'MZ' */
	.skip	58				/* 'MZ' + pad + offset == 64 */
	.long	pe_header - image_base		/* Offset to the PE header */
pe_header:
	.long	IMAGE_NT_SIGNATURE		/* 'PE' */
coff_header:
	.short	IMAGE_FILE_MACHINE_THUMB	/* Mixed ARM/Thumb */
	.short	3				/* nr_sections */
	.long	0				/* TimeDateStamp */
	.long	0				/* PointerToSymbolTable */
	.long	0				/* NumberOfSymbols */
	.short	section_table - optional_header	/* SizeOfOptionalHeader */
	/* Characteristics */
	.short	(IMAGE_FILE_EXECUTABLE_IMAGE | \
		 IMAGE_FILE_LINE_NUMS_STRIPPED | \
		 IMAGE_FILE_LOCAL_SYMS_STRIPPED | \
		 IMAGE_FILE_32BIT_MACHINE | \
		 IMAGE_FILE_DEBUG_STRIPPED)
optional_header:
	.short	IMAGE_NT_OPTIONAL_HDR32_MAGIC	/* PE32 format */
	.byte	0x02				/* MajorLinkerVersion */
	.byte	0x14				/* MinorLinkerVersion */
	.long	_etext - _start			/* SizeOfCode */
	.long	_data_size			/* SizeOfInitializedData */
	.long	0				/* SizeOfUninitializedData */
	.long	_start - image_base		/* AddressOfEntryPoint */
	.long	_start - image_base		/* BaseOfCode */
	.long	0				/* BaseOfData */

extra_header_fields:
	.long	0				/* ImageBase */
	.long	0x1000				/* SectionAlignment */
	.long	0x200				/* FileAlignment */
	.short	0				/* MajorOperatingSystemVersion */
	.short	0				/* MinorOperatingSystemVersion */
	.short	0				/* MajorImageVersion */
	.short	0				/* MinorImageVersion */
	.short	0				/* MajorSubsystemVersion */
	.short	0				/* MinorSubsystemVersion */
	.long	0				/* Win32VersionValue */

	.long	_edata - image_base		/* SizeOfImage */

	/*
	 * Everything before the kernel image is considered part of the header
	 */
	.long	_start - image_base		/* SizeOfHeaders */
	.long	0				/* CheckSum */
	.short	IMAGE_SUBSYSTEM_EFI_APPLICATION	/* Subsystem */
#if CONFIG_VENDOR_EFI
	.short	0				/* DllCharacteristics */
#else
	.short	IMAGE_DLLCHARACTERISTICS_NX_COMPAT
#endif
	.long	0				/* SizeOfStackReserve */
	.long	0				/* SizeOfStackCommit */
	.long	0				/* SizeOfHeapReserve */
	.long	0				/* SizeOfHeapCommit */
	.long	0				/* LoaderFlags */
	.long	0x6				/* NumberOfRvaAndSizes */

	.quad	0				/* ExportTable */
	.quad	0				/* ImportTable */
	.quad	0				/* ResourceTable */
	.quad	0				/* ExceptionTable */
	.quad	0				/* CertificationTable */
	.quad	0				/* BaseRelocationTable */

	/* Section table */
section_table:

	/*
	 * The EFI application loader requires a relocation section
	 * because EFI applications must be relocatable.  This is a
	 * dummy section as far as we are concerned.
	 */
	.ascii	".reloc"
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	0
	.long	0
	.long	0			/* SizeOfRawData */
	.long	0			/* PointerToRawData */
	.long	0			/* PointerToRelocations */
	.long	0			/* PointerToLineNumbers */
	.short	0			/* NumberOfRelocations */
	.short	0			/* NumberOfLineNumbers */
	/* Characteristics (section flags) */
	.long	(IMAGE_SCN_MEM_READ | \
		 IMAGE_SCN_MEM_DISCARDABLE | \
		 IMAGE_SCN_CNT_INITIALIZED_DATA)

	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	_etext - _start		/* VirtualSize */
	.long	_start - image_base	/* VirtualAddress */
	.long	_etext - _start		/* SizeOfRawData */
	.long	_start - image_base	/* PointerToRawData */
	.long	0			/* PointerToRelocations */
	.long	0			/* PointerToLineNumbers */
	.short	0			/* NumberOfRelocations */
	.short	0			/* NumberOfLineNumbers */
	/* Characteristics (section flags) */
	.long	(IMAGE_SCN_MEM_READ | \
		 IMAGE_SCN_MEM_EXECUTE | \
		 IMAGE_SCN_CNT_CODE)

	.ascii	".data"
	.byte	0
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	_data_size		/* VirtualSize */
	.long	_data - image_base	/* VirtualAddress */
	.long	_data_size		/* SizeOfRawData */
	.long	_data - image_base	/* PointerToRawData */
	.long	0			/* PointerToRelocations */
	.long	0			/* PointerToLineNumbers */
	.short	0			/* NumberOfRelocations */
	.short	0			/* NumberOfLineNumbers */
	/* Characteristics (section flags) */
	.long	(IMAGE_SCN_MEM_WRITE | \
		 IMAGE_SCN_MEM_READ | \
		 IMAGE_SCN_CNT_INITIALIZED_DATA)

	.align		12
	.globl		_start
_start:
	stmfd		sp!, {r0-r2, lr}

	adr		r1, .L_DYNAMIC
	ldr		r0, [r1]
	add		r1, r0, r1
	sub		r0, pc, #((.+8-image_base) & 0xff)
	sub		r0, r0, #((.+4-image_base) & 0xff00)
	bl		_relocate
	teq		r0, #0
	bne		0f

	ldmfd		sp, {r0-r1}
	bl		efi_main

0:	add		sp, sp, #12
	ldr		pc, [sp], #4

.L_DYNAMIC:
	.word		_DYNAMIC - .
