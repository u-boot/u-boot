/*
 * Copyright (C) 2011 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * Configuation settings for the SAMSUNG TRATS (EXYNOS4210) board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SAMSUNG		/* in a SAMSUNG core */
#define CONFIG_S5P		/* which is in a S5P Family */
#define CONFIG_EXYNOS4210	/* which is in a EXYNOS4210 */
#define CONFIG_TRATS		/* working with TRATS */
#define CONFIG_TIZEN		/* TIZEN lib */

#include <asm/arch/cpu.h>	/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_L2CACHE_OFF

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x63300000

/* input clock of PLL: TRATS has 24MHz input clock at EXYNOS4210 */
#define CONFIG_SYS_CLK_FREQ_C210	24000000
#define CONFIG_SYS_CLK_FREQ		CONFIG_SYS_CLK_FREQ_C210

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/* MACH_TYPE_TRATS macro will be removed once added to mach-types */
#define MACH_TYPE_TRATS			3928
#define CONFIG_MACH_TYPE		MACH_TYPE_TRATS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (1 << 20))

/* select serial console configuration */
#define CONFIG_SERIAL_MULTI
#define CONFIG_SERIAL2			/* use SERIAL 2 */
#define CONFIG_BAUDRATE			115200

/* MMC */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_S5P_SDHCI
#define CONFIG_SDHCI

/* PWM */
#define CONFIG_PWM

/* It should define before config_cmd_default.h */
#define CONFIG_SYS_NO_FLASH

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_XIMG
#undef CONFIG_CMD_CACHE
#undef CONFIG_CMD_ONENAND
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_MMC

#define CONFIG_BOOTDELAY		1
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTARGS			"Please use defined boot"
#define CONFIG_BOOTCOMMAND		"run mmcboot"

#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"
#define CONFIG_BOOTBLOCK		"10"
#define CONFIG_ENV_COMMON_BOOT		"${console} ${meminfo}"

#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootk=" \
		"run loaduimage; bootm 0x40007FC0\0" \
	"updatemmc=" \
		"mmc boot 0 1 1 1; mmc write 0 0x42008000 0 0x200;" \
		"mmc boot 0 1 1 0\0" \
	"updatebackup=" \
		"mmc boot 0 1 1 2; mmc write 0 0x42100000 0 0x200;" \
		"mmc boot 0 1 1 0\0" \
	"updatebootb=" \
		"mmc read 0 0x42100000 0x80 0x200; run updatebackup\0" \
	"lpj=lpj=3981312\0" \
	"nfsboot=" \
		"set bootargs root=/dev/nfs rw " \
		"nfsroot=${nfsroot},nolock,tcp " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
		"${netmask}:generic:usb0:off " CONFIG_ENV_COMMON_BOOT \
		"; run bootk\0" \
	"ramfsboot=" \
		"set bootargs root=/dev/ram0 rw rootfstype=ext2 " \
		"${console} ${meminfo} " \
		"initrd=0x43000000,8M ramdisk=8192\0" \
	"mmcboot=" \
		"set bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart} " \
		"${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo}; " \
		"run loaduimage; bootm 0x40007FC0\0" \
	"bootchart=set opts init=/sbin/bootchartd; run bootcmd\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"mmcoops=mmc read 0 0x40000000 0x40 8; md 0x40000000 0x400\0" \
	"verify=n\0" \
	"rootfstype=ext4\0" \
	"console=" CONFIG_DEFAULT_CONSOLE \
	"meminfo=crashkernel=32M@0x50000000\0" \
	"nfsroot=/nfsroot/arm\0" \
	"bootblock=" CONFIG_BOOTBLOCK "\0" \
	"mmcdev=0\0" \
	"mmcbootpart=2\0" \
	"mmcrootpart=3\0" \
	"opts=always_resume=1"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"TRATS # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4800000)

#define CONFIG_SYS_HZ			1000

/* Stack sizes */
#define CONFIG_STACKSIZE		(256 << 10) /* regular stack 256KB */

/* TRATS has 2 banks of DRAM */
#define CONFIG_NR_DRAM_BANKS	2
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE	/* LDDDR2 DMC 0 */
#define PHYS_SDRAM_1_SIZE	(512 << 20)		/* 512 MB in CS 0 */
#define PHYS_SDRAM_2		0x50000000		/* LPDDR2 DMC 1 */
#define PHYS_SDRAM_2_SIZE	(512 << 20)		/* 512 MB in CS 0 */

#define CONFIG_SYS_MEM_TOP_HIDE		(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		((32 - 4) << 10) /* 32KiB - 4KiB */

#define CONFIG_DOS_PARTITION

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_CACHELINE_SIZE       32

#include <asm/arch/gpio.h>
/*
 * I2C Settings
 */
#define CONFIG_SOFT_I2C_GPIO_SCL exynos4_gpio_part1_get_nr(b, 7)
#define CONFIG_SOFT_I2C_GPIO_SDA exynos4_gpio_part1_get_nr(b, 6)

#define CONFIG_SOFT_I2C
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define CONFIG_SYS_I2C_SPEED	50000
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_MAX_I2C_BUS	7

#define CONFIG_PMIC
#define CONFIG_PMIC_I2C
#define CONFIG_PMIC_MAX8997

#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED

/* LCD */
#define CONFIG_EXYNOS_FB
#define CONFIG_LCD
#define CONFIG_CMD_BMP
#define CONFIG_BMP_32BPP
#define CONFIG_FB_ADDR		0x52504000
#define CONFIG_S6E8AX0
#define CONFIG_EXYNOS_MIPI_DSIM
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE ((500 * 120 * 4) + (1 << 12))

#endif	/* __CONFIG_H */
