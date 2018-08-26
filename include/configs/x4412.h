/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the 9tripod X4412 (EXYNOS4412) board.
 */

#ifndef __CONFIG_X4412_H
#define __CONFIG_X4412_H

#include <configs/exynos4-common.h>

/* X4412 has 2 bank of DRAM */
#define CONFIG_NR_DRAM_BANKS		2
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM_1			CONFIG_SYS_SDRAM_BASE
#define SDRAM_BANK_SIZE			(512 << 20)	/* 512 MiB */

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x6000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000)
#define CONFIG_SYS_INIT_SP_ADDR	    (CONFIG_SYS_LOAD_ADDR - 0x1000000)

#define CONFIG_MACH_TYPE		MACH_TYPE_X4412

/* select serial console configuration */
#define CONFIG_SERIAL3
#define CONFIG_BAUDRATE 115200

/* Console configuration */
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC3,115200n8\0"

#define CONFIG_SYS_MEM_TOP_HIDE	(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/* Power Down Modes */
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

/* SPL configuration */
#define CONFIG_SPL_TEXT_BASE	0x02025000
#define CONFIG_SPL_MAX_FOOTPRINT	(14 * 1024)
#define CONFIG_SPL_STACK    0x02060000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x40007000\0" \
	"rdaddr=0x48000000\0" \
	"kerneladdr=0x40007000\0" \
	"ramdiskaddr=0x48000000\0" \
	"console=ttySAC3,115200n8\0" \
	"mmcdev=0\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
        "loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
        "bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
                "source ${loadaddr}\0"

#define CONFIG_BOOTCOMMAND \
	"if mmc rescan; then " \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"fi; " \
	"fi;" \
	"load mmc ${mmcdev} ${loadaddr} uImage; bootm ${loadaddr} "

#define CONFIG_CLK_1000_400_200

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KB */
#define RESERVE_BLOCK_SIZE		(512)
#define BL1_SIZE			(15 << 10) /* 15 KiB reserved for BL1*/
#define BL2_SZIE            (16 << 10)
#define COPY_UBOOT_SIZE     (0x80000)   /* 512 KiB */
#define COPY_TZSW_SIZE      (0x27000)   /* 156 KiB */
#define UBOOT_START_OFFSET  ((RESERVE_BLOCK_SIZE + BL1_SIZE + BL2_SZIE)/512)
#define UBOOT_SIZE_BLOC_COUNT   (COPY_UBOOT_SIZE/512)
#define TZSW_START_OFFSET   (UBOOT_START_OFFSET + UBOOT_SIZE_BLOC_COUNT)
#define TZSW_SIZE_BLOC_COUNT    (COPY_TZSW_SIZE/512)
#define CONFIG_ENV_OFFSET		(RESERVE_BLOCK_SIZE + BL1_SIZE + BL2_SZIE \
                                    + COPY_UBOOT_SIZE + COPY_TZSW_SIZE)

#endif	/* __CONFIG_H */
