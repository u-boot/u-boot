/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG ORIGEN (EXYNOS4210) board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ORIGEN_H
#define __CONFIG_ORIGEN_H

#include <configs/exynos4-common.h>

#define CONFIG_SYS_PROMPT		"ORIGEN # "


/* High Level Configuration Options */
#define CONFIG_EXYNOS4210		1	/* which is a EXYNOS4210 SoC */
#define CONFIG_ORIGEN			1	/* working with ORIGEN*/

#define CONFIG_SYS_DCACHE_OFF		1

/* ORIGEN has 4 bank of DRAM */
#define CONFIG_NR_DRAM_BANKS		4
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM_1			CONFIG_SYS_SDRAM_BASE
#define SDRAM_BANK_SIZE			(256 << 20)	/* 256 MB */

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x6000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000)

#define CONFIG_SYS_TEXT_BASE		0x43E00000

#define CONFIG_MACH_TYPE		MACH_TYPE_ORIGEN

/* select serial console configuration */
#define CONFIG_SERIAL2
#define CONFIG_BAUDRATE			115200

/* Console configuration */
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#define CONFIG_SYS_MEM_TOP_HIDE	(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/* Power Down Modes */
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

#undef CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DHCP
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

/* MMC SPL */
#define COPY_BL2_FNPTR_ADDR	0x02020030
#define CONFIG_SPL_TEXT_BASE	0x02021410

#define CONFIG_BOOTCOMMAND	"fatload mmc 0 40007000 uImage; bootm 40007000"

#define CONFIG_IDENT_STRING		" for ORIGEN"

#define CONFIG_CLK_1000_400_200

/* MIU (Memory Interleaving Unit) */
#define CONFIG_MIU_2BIT_21_7_INTERLEAVED

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KB */
#define RESERVE_BLOCK_SIZE		(512)
#define BL1_SIZE			(16 << 10) /*16 K reserved for BL1*/
#define CONFIG_ENV_OFFSET		(RESERVE_BLOCK_SIZE + BL1_SIZE)

#define CONFIG_SPL_LDSCRIPT	"board/samsung/common/exynos-uboot-spl.lds"
#define CONFIG_SPL_MAX_FOOTPRINT	(14 * 1024)

#define CONFIG_SYS_INIT_SP_ADDR		0x02040000

/* U-boot copy size from boot Media to DRAM.*/
#define COPY_BL2_SIZE		0x80000
#define BL2_START_OFFSET	((CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)/512)
#define BL2_SIZE_BLOC_COUNT	(COPY_BL2_SIZE/512)

#endif	/* __CONFIG_H */
