/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Total Compute platform. Parts were derived from other ARM
 * configurations.
 * (C) Copyright 2020 Arm Limited
 * Usama Arif <usama.arif@arm.com>
 */

#ifndef __TOTAL_COMPUTE_H
#define __TOTAL_COMPUTE_H

#define CONFIG_REMAKE_ELF

/* Link Definitions */
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

#define CONFIG_SYS_BOOTM_LEN	(64 << 20)

#define UART0_BASE		0x7ff80000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + (8 << 20))

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK	7372800

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	0x90000000

/* Physical Memory Map */
#define PHYS_SDRAM_1		0x80000000
/* Top 48MB reserved for secure world use */
#define DRAM_SEC_SIZE		0x03000000
#define PHYS_SDRAM_1_SIZE	0x80000000 - DRAM_SEC_SIZE
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CONFIG_ARM_PL180_MMCI_BASE		0x001c050000
#define CONFIG_SYS_MMC_MAX_BLK_COUNT		127
#define CONFIG_ARM_PL180_MMCI_CLOCK_FREQ	12000000

#define CONFIG_EXTRA_ENV_SETTINGS	\
				"bootm_size=0x20000000\0"	\
				"load_addr=0xa0000000\0"	\
				"kernel_addr_r=0x80080000\0"	\
				"initrd_addr_r=0x88000000\0"	\
				"fdt_addr_r=0x83000000\0"
/*
 * If vbmeta partition is present, boot Android with verification using AVB.
 * Else if system partition is present (no vbmeta partition), boot Android
 * without verification (for development purposes).
 * Else boot FIT image.
 */
#define CONFIG_BOOTCOMMAND	\
				"if part number mmc 0 vbmeta is_avb; then" \
				"  echo MMC with vbmeta partition detected.;" \
				"  echo starting Android Verified boot.;" \
				"  avb init 0; " \
				"  if avb verify; then " \
				"    set bootargs $bootargs $avb_bootargs; " \
				"    part start mmc 0 boot boot_start; " \
				"    part size mmc 0 boot boot_size; " \
				"    mmc read ${load_addr} ${boot_start} ${boot_size}; " \
				"    bootm ${load_addr} ${load_addr} ${fdt_addr_r}; " \
				"  else; " \
				"    echo AVB verification failed.; " \
				"    exit; " \
				"  fi; " \
				"elif part number mmc 0 system is_non_avb_android; then " \
				"  booti ${kernel_addr_r} ${initrd_addr_r} ${fdt_addr_r};" \
				"else;" \
				"  echo Booting FIT image.;" \
				"  bootm ${load_addr} ${load_addr} ${fdt_addr_r}; " \
				"fi;"

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_FLASH_BASE		0x0C000000
/* 256 x 256KiB sectors */
#define CONFIG_SYS_MAX_FLASH_SECT	256

#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_32BIT
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */
#define FLASH_MAX_SECTOR_SIZE		0x00040000

#endif /* __TOTAL_COMPUTE_H */
