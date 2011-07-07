/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Configuation settings for the SAMSUNG Universal (s5pc100) board.
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

/* High Level Configuration Options */
#define CONFIG_SAMSUNG		1	/* in a SAMSUNG core */
#define CONFIG_S5P		1	/* which is in a S5P Family */
#define CONFIG_S5PC110		1	/* which is in a S5PC110 */
#define CONFIG_MACH_GONI	1	/* working with Goni */

#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* input clock of PLL: has 24MHz input clock at S5PC110 */
#define CONFIG_SYS_CLK_FREQ_C110	24000000

/* DRAM Base */
#define CONFIG_SYS_SDRAM_BASE		0x30000000

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_EDITING

/*
 * Size of malloc() pool
 * 1MB = 0x100000, 0x100000 = 1024 * 1024
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (1 << 20))
/*
 * select serial console configuration
 */
#define CONFIG_SERIAL2			1	/* use SERIAL2 */
#define CONFIG_SERIAL_MULTI		1
#define CONFIG_BAUDRATE			115200

/* MMC */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_S5P_MMC			1

/* PWM */
#define CONFIG_PWM			1

/* It should define before config_cmd_default.h */
#define CONFIG_SYS_NO_FLASH		1

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_XIMG
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_ONENAND
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_MMC

#define CONFIG_BOOTDELAY		1
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

/* Actual modem binary size is 16MiB. Add 2MiB for bad block handling */
#define MTDIDS_DEFAULT		"onenand0=samsung-onenand"
#define MTDPARTS_DEFAULT	"mtdparts=samsung-onenand:1m(bootloader)"\
				",256k(params)"\
				",2816k(config)"\
				",8m(csa)"\
				",7m(kernel)"\
				",1m(log)"\
				",12m(modem)"\
				",60m(qboot)"\
				",-(UBI)\0"

#define NORMAL_MTDPARTS_DEFAULT MTDPARTS_DEFAULT

#define CONFIG_BOOTCOMMAND	"run ubifsboot"

#define CONFIG_DEFAULT_CONSOLE	"console=ttySAC2,115200n8\0"

#define CONFIG_RAMDISK_BOOT	"root=/dev/ram0 rw rootfstype=ext2" \
		" ${console} ${meminfo}"

#define CONFIG_COMMON_BOOT	"${console} ${meminfo} ${mtdparts}"

#define CONFIG_BOOTARGS	"root=/dev/mtdblock8 ubi.mtd=8 ubi.mtd=3 ubi.mtd=6" \
		" rootfstype=cramfs " CONFIG_COMMON_BOOT

#define CONFIG_UPDATEB	"updateb=onenand erase 0x0 0x100000;" \
			" onenand write 0x32008000 0x0 0x100000\0"

#define CONFIG_UBI_MTD	" ubi.mtd=${ubiblock} ubi.mtd=3 ubi.mtd=6"

#define CONFIG_UBIFS_OPTION	"rootflags=bulk_read,no_chk_data_crc"

#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_UPDATEB \
	"updatek=" \
		"onenand erase 0xc00000 0x600000;" \
		"onenand write 0x31008000 0xc00000 0x600000\0" \
	"updateu=" \
		"onenand erase 0x01560000 0x1eaa0000;" \
		"onenand write 0x32000000 0x1260000 0x8C0000\0" \
	"bootk=" \
		"onenand read 0x30007FC0 0xc00000 0x600000;" \
		"bootm 0x30007FC0\0" \
	"flashboot=" \
		"set bootargs root=/dev/mtdblock${bootblock} " \
		"rootfstype=${rootfstype}" CONFIG_UBI_MTD " ${opts} " \
		"${lcdinfo} " CONFIG_COMMON_BOOT "; run bootk\0" \
	"ubifsboot=" \
		"set bootargs root=ubi0!rootfs rootfstype=ubifs " \
		CONFIG_UBIFS_OPTION CONFIG_UBI_MTD " ${opts} ${lcdinfo} " \
		CONFIG_COMMON_BOOT "; run bootk\0" \
	"tftpboot=" \
		"set bootargs root=ubi0!rootfs rootfstype=ubifs " \
		CONFIG_UBIFS_OPTION CONFIG_UBI_MTD " ${opts} ${lcdinfo} " \
		CONFIG_COMMON_BOOT "; tftp 0x30007FC0 uImage; " \
		"bootm 0x30007FC0\0" \
	"ramboot=" \
		"set bootargs " CONFIG_RAMDISK_BOOT \
		" initrd=0x33000000,8M ramdisk=8192\0" \
	"mmcboot=" \
		"set bootargs root=${mmcblk} rootfstype=${rootfstype}" \
		CONFIG_UBI_MTD " ${opts} ${lcdinfo} " \
		CONFIG_COMMON_BOOT "; run bootk\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"bootchart=set opts init=/sbin/bootchartd; run bootcmd\0" \
	"verify=n\0" \
	"rootfstype=cramfs\0" \
	"console=" CONFIG_DEFAULT_CONSOLE \
	"mtdparts=" MTDPARTS_DEFAULT \
	"meminfo=mem=80M mem=256M@0x40000000 mem=128M@0x50000000\0" \
	"mmcblk=/dev/mmcblk1p1\0" \
	"bootblock=9\0" \
	"ubiblock=8\0" \
	"ubi=enabled\0" \
	"opts=always_resume=1"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser	*/
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT	"Goni # "
#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4000000)

#define CONFIG_SYS_HZ			1000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Stack sizes */
#define CONFIG_STACKSIZE	(256 << 10)	/* 256 KiB */

/* Goni has 3 banks of DRAM, but swap the bank */
#define CONFIG_NR_DRAM_BANKS	3
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE	/* OneDRAM Bank #0 */
#define PHYS_SDRAM_1_SIZE	(80 << 20)		/* 80 MB in Bank #0 */
#define PHYS_SDRAM_2		0x40000000		/* mDDR DMC1 Bank #1 */
#define PHYS_SDRAM_2_SIZE	(256 << 20)		/* 256 MB in Bank #1 */
#define PHYS_SDRAM_3		0x50000000		/* mDDR DMC2 Bank #2 */
#define PHYS_SDRAM_3_SIZE	(128 << 20)		/* 128 MB in Bank #2 */

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* 256 KiB */

/* FLASH and environment organization */
#define CONFIG_ENV_IS_IN_ONENAND	1
#define CONFIG_ENV_SIZE			(256 << 10)	/* 256 KiB, 0x40000 */
#define CONFIG_ENV_ADDR			(1 << 20)	/* 1 MB, 0x100000 */

#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SAMSUNG_ONENAND		1
#define CONFIG_SYS_ONENAND_BASE		0xB0000000

#define CONFIG_DOS_PARTITION		1

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - 0x1000000)

#endif	/* __CONFIG_H */
