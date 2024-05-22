/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * crt0-efi-loongarch.S - PE/COFF header for LoongArch EFI applications
 *
 * Copright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <asm-generic/pe.h>
#include <asm/asm.h>

#if __loongarch_grlen == 64
#define PE_MACHINE	IMAGE_FILE_MACHINE_LOONGARCH64
#define PE_MAGIC    IMAGE_NT_OPTIONAL_HDR64_MAGIC
#define IMG_CHARACTERISTICS \
	(IMAGE_FILE_EXECUTABLE_IMAGE | \
	 IMAGE_FILE_LINE_NUMS_STRIPPED | \
	 IMAGE_FILE_LOCAL_SYMS_STRIPPED | \
	 IMAGE_FILE_LARGE_ADDRESS_AWARE | \
	 IMAGE_FILE_DEBUG_STRIPPED)
#else
#define PE_MACHINE	IMAGE_FILE_MACHINE_LOONGARCH32
#define PE_MAGIC    IMAGE_NT_OPTIONAL_HDR32_MAGIC
#define IMG_CHARACTERISTICS \
	(IMAGE_FILE_EXECUTABLE_IMAGE | \
	 IMAGE_FILE_LINE_NUMS_STRIPPED | \
	 IMAGE_FILE_LOCAL_SYMS_STRIPPED | \
	 IMAGE_FILE_DEBUG_STRIPPED)
#endif

	.section	.text.head

	/*
	 * Magic "MZ" signature for PE/COFF
	 */
	.globl	ImageBase
ImageBase:
	.short	IMAGE_DOS_SIGNATURE		/* 'MZ' */
	.skip	58
	.long	pe_header - ImageBase		/* Offset to the PE header */
pe_header:
	.long	IMAGE_NT_SIGNATURE		/* 'PE' */
coff_header:
	.short	PE_MACHINE			/* LoongArch 64/32-bit */
	.short	3				/* nr_sections */
	.long	0				/* TimeDateStamp */
	.long	0				/* PointerToSymbolTable */
	.long	0				/* NumberOfSymbols */
	.short	section_table - optional_header	/* SizeOfOptionalHeader */
	.short	IMG_CHARACTERISTICS		/* Characteristics */
optional_header:
	.short	PE_MAGIC			/* PE32(+) format */
	.byte	0x02				/* MajorLinkerVersion */
	.byte	0x14				/* MinorLinkerVersion */
	.long	_edata - _start			/* SizeOfCode */
	.long	0				/* SizeOfInitializedData */
	.long	0				/* SizeOfUninitializedData */
	.long	_start - ImageBase		/* AddressOfEntryPoint */
	.long	_start - ImageBase		/* BaseOfCode */
#if __loongarch_grlen != 64
	.long	0				/* BaseOfData */
#endif

extra_header_fields:
	LONG	0
	.long	0x200				/* SectionAlignment */
	.long	0x200				/* FileAlignment */
	.short	0				/* MajorOperatingSystemVersion */
	.short	0				/* MinorOperatingSystemVersion */
	.short	1				/* MajorImageVersion */
	.short	0				/* MinorImageVersion */
	.short	0				/* MajorSubsystemVersion */
	.short	0				/* MinorSubsystemVersion */
	.long	0				/* Win32VersionValue */

	.long	_edata - ImageBase		/* SizeOfImage */

	/*
	 * Everything before the kernel image is considered part of the header
	 */
	.long	_start - ImageBase		/* SizeOfHeaders */
	.long	0				/* CheckSum */
	.short	IMAGE_SUBSYSTEM_EFI_APPLICATION /* Subsystem */
#if CONFIG_VENDOR_EFI
	.short	0				/* DllCharacteristics */
#else
	.short	IMAGE_DLLCHARACTERISTICS_NX_COMPAT
#endif
	LONG	0				/* SizeOfStackReserve */
	LONG	0				/* SizeOfStackCommit */
	LONG	0				/* SizeOfHeapReserve */
	LONG	0				/* SizeOfHeapCommit */

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
	.long	0x42100040		/* Characteristics (section flags) */


	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	_etext - _start		/* VirtualSize */
	.long	_start - ImageBase	/* VirtualAddress */
	.long	_etext - _start		/* SizeOfRawData */
	.long	_start - ImageBase	/* PointerToRawData */
	.long	0			/* PointerToRelocations (0 for executables) */
	.long	0			/* PointerToLineNumbers (0 for executables) */
	.short	0			/* NumberOfRelocations  (0 for executables) */
	.short	0			/* NumberOfLineNumbers  (0 for executables) */
	/* Characteristics (section flags) */
	.long	(IMAGE_SCN_MEM_READ | \
		 IMAGE_SCN_MEM_EXECUTE | \
		 IMAGE_SCN_CNT_CODE)

	.ascii	".data"
	.byte	0
	.byte	0
	.byte	0			/* end of 0 padding of section name */
	.long	_edata - _data		/* VirtualSize */
	.long	_data - ImageBase	/* VirtualAddress */
	.long	_edata - _data		/* SizeOfRawData */
	.long	_data - ImageBase	/* PointerToRawData */
	.long	0			/* PointerToRelocations */
	.long	0			/* PointerToLineNumbers */
	.short	0			/* NumberOfRelocations */
	.short	0			/* NumberOfLineNumbers */
	/* Characteristics (section flags) */
	.long	(IMAGE_SCN_MEM_WRITE | \
		 IMAGE_SCN_MEM_READ | \
		 IMAGE_SCN_CNT_INITIALIZED_DATA)

	.align		12
	.globl	_start
	.type	_start, @function
_start:
	PTR_ADDI	sp, sp, -(3 * LONGSIZE)
	LONG_S		ra, sp, 0
	LONG_S		a0, sp, (1 * LONGSIZE)
	LONG_S		a1, sp, (2 * LONGSIZE)

	move		a2, a0		/* a2: ImageHandle */
	move		a3, a1		/* a3: SystemTable */
	la.local	a0, ImageBase	/* a0: ImageBase */
	la.local	a1, _DYNAMIC	/* a1: DynamicSection */
	bl 	  	_relocate
	bnez		a0, 0f

	LONG_L		a0, sp, (1 * LONGSIZE)
	LONG_L		a1, sp, (2 * LONGSIZE)
	bl		efi_main

0:	LONG_L		ra, sp, 0
	PTR_ADDI	sp, sp, (3 * LONGSIZE)
	jr			ra
	.end	  _start
