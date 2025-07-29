/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the SAMA7D65 SoC
 *
 * Copyright (C) 2024 Microchip Technology, Inc. and its subsidiaries
 */

#ifndef __SAMA7D65_H__
#define __SAMA7D65_H__

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FLEXCOM0	34
#define ATMEL_ID_FLEXCOM1	35
#define ATMEL_ID_FLEXCOM2	36
#define ATMEL_ID_FLEXCOM3	37
#define ATMEL_ID_FLEXCOM4	38
#define ATMEL_ID_FLEXCOM5	39
#define ATMEL_ID_FLEXCOM6	40
#define ATMEL_ID_FLEXCOM7	41
#define ATMEL_ID_FLEXCOM8	42

#define ATMEL_ID_SDMMC0		75
#define ATMEL_ID_SDMMC1		76
#define ATMEL_ID_SDMMC2		77

#define ATMEL_ID_PIT64B0	66
#define ATMEL_ID_PIT64B		ATMEL_ID_PIT64B0

#define ATMEL_CHIPID_CIDR	0xe0020000
#define ATMEL_CHIPID_EXID	0xe0020004
/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_PIOA		0xe0014000
#define ATMEL_BASE_PIOB		(ATMEL_BASE_PIOA + 0x40)
#define ATMEL_BASE_PIOC		(ATMEL_BASE_PIOB + 0x40)
#define ATMEL_BASE_PIOD		(ATMEL_BASE_PIOC + 0x40)
#define ATMEL_BASE_PIOE		(ATMEL_BASE_PIOD + 0x40)

#define ATMEL_PIO_PORTS		5

#define CPU_HAS_PCR

#define ATMEL_BASE_PMC		0xe0018000

#define ATMEL_BASE_WDT		0xe001c000
#define ATMEL_BASE_RSTC		0xe001d100
#define ATMEL_BASE_WDTS		0xe001d180
#define ATMEL_BASE_SCKCR	0xe001d500

#define ATMEL_BASE_SDMMC0	0xe1204000
#define ATMEL_BASE_SDMMC1	0xe1208000

#define ATMEL_BASE_PIT64B0	0xe1800000

#define ATMEL_BASE_FLEXCOM0	0xe1820000
#define ATMEL_BASE_FLEXCOM1	0xe1824000
#define ATMEL_BASE_FLEXCOM2	0xe1828000
#define ATMEL_BASE_FLEXCOM3	0xe182c000
#define ATMEL_BASE_FLEXCOM4	0xe2018000
#define ATMEL_BASE_FLEXCOM5	0xe201C000
#define ATMEL_BASE_FLEXCOM6	0xe2020000
#define ATMEL_BASE_FLEXCOM7	0xe2024000
#define ATMEL_BASE_FLEXCOM8	0xe281C000

#define ATMEL_BASE_TZC400	0xe3000000

#define ATMEL_BASE_UMCTL2	0xe3800000
#define ATMEL_BASE_UMCTL2_MP	0xe38003f8
#define ATMEL_BASE_PUBL		0xe3804000

#define ATMEL_NUM_FLEXCOM	11
#define ATMEL_PIO_PORTS		5

#define ATMEL_BASE_PIT64BC	ATMEL_BASE_PIT64B0

#define ARCH_ID_SAMA7D65	0x80262100
#define ARCH_EXID_SAMA7D65	0x00000080
#define ARCH_EXID_SAMA7D65_DD2	0x00000010
#define ARCH_EXID_SAMA7D65_D1G	0x00000018
#define ARCH_EXID_SAMA7D65_D2G	0x00000020
#define ARCH_EXID_SAMA7D65_D4G	0x00000028
#define ARCH_EXID_SAMA7D65_TA	0x00000040

#define cpu_is_sama7d65()	(get_chip_id() == ARCH_ID_SAMA7D65)
#define cpu_is_sama7d65_S()	(cpu_is_sama7d65() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7D65))
#define cpu_is_sama7d65_DD2()	(cpu_is_sama7d65() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7D65_DD2))
#define cpu_is_sama7d65_D1G()	(cpu_is_sama7d65() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7D65_D1G))
#define cpu_is_sama7d65_D2G()	(cpu_is_sama7d65() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7D65_D2G))
#define cpu_is_sama7d65_D4G()	(cpu_is_sama7d65() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7D65_D4G))
#define cpu_is_sama7d65_TA()	(cpu_is_sama7d65() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7D65_TA))

#ifndef __ASSEMBLY__
unsigned int get_chip_id(void);
unsigned int get_extension_chip_id(void);
char *get_cpu_name(void);
#endif

#endif /* #ifndef __SAMA7D65_H__ */
