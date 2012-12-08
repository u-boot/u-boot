/*
 * Copyright (C) 2009, Ilya Yanok, Emcraft Systems, <yanok@emcraft.com>
 *
 * Configuration settings for the Dave/DENX QongEVB-LITE board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* High Level Configuration Options */
#define CONFIG_ARM1136			/* This is an arm1136 CPU core */
#define CONFIG_MX31			/* in a mx31 */
#define CONFIG_QONG

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_TEXT_BASE 0xa0000000

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1536 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

#define CONFIG_MXC_GPIO
#define CONFIG_HW_WATCHDOG

#define CONFIG_MXC_SPI
#define CONFIG_DEFAULT_SPI_BUS	1
#define CONFIG_DEFAULT_SPI_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_RTC_MC13XXX

#define CONFIG_POWER
#define CONFIG_POWER_SPI
#define CONFIG_POWER_FSL
#define CONFIG_FSL_PMIC_BUS	1
#define CONFIG_FSL_PMIC_CS	0
#define CONFIG_FSL_PMIC_CLK	100000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_FSL_PMIC_BITLEN	32

/* FPGA */
#define CONFIG_FPGA
#define CONFIG_QONG_FPGA
#define CONFIG_FPGA_BASE	(CS1_BASE)
#define CONFIG_FPGA_LATTICE
#define CONFIG_FPGA_COUNT	1

#ifdef CONFIG_QONG_FPGA
/* Ethernet */
#define CONFIG_DNET
#define CONFIG_DNET_BASE	(CS1_BASE + QONG_FPGA_PERIPH_SIZE)

/* Framebuffer and LCD */
#define CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_MX3
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SPLASH_SCREEN
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(512 << 10)

/* USB */
#define CONFIG_CMD_USB
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI			/* Enable EHCI USB support */
#define CONFIG_USB_EHCI_MXC
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORT	2
#define CONFIG_MXC_USB_PORTSC	(MXC_EHCI_MODE_ULPI | MXC_EHCI_UTMI_8BIT)
#define CONFIG_MXC_USB_FLAGS	MXC_EHCI_POWER_PINS_ENABLED
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#endif /* CONFIG_CMD_USB */

/*
 * Reducing the ARP timeout from default 5 seconds to 200ms we speed up the
 * initial TFTP transfer, should the user wish one, significantly.
 */
#define CONFIG_ARP_TIMEOUT	200UL

#endif /* CONFIG_QONG_FPGA */

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_SPI
#define CONFIG_CMD_UNZIP

#define CONFIG_BOARD_LATE_INIT

#define CONFIG_BOOTDELAY	5

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=ttymxc0,${baudrate}\0"			\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc=setenv bootargs ${bootargs}\0"				\
	"uboot_addr=A0000000\0"						\
	"kernel_addr=A00C0000\0"					\
	"ramdisk_addr=A0300000\0"					\
	"u-boot=qong/u-boot.bin\0"					\
	"kernel_addr_r=80800000\0"					\
	"hostname=qong\0"						\
	"bootfile=qong/uImage\0"					\
	"rootpath=/opt/eldk-4.2-arm/armVFP\0"				\
	"flash_self=run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm\0"						\
	"bootcmd=run flash_self\0"					\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};era " __stringify(CONFIG_SYS_MONITOR_BASE)\
		" +${filesize};cp.b ${fileaddr} "			\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"	\
	"upd=run load update\0"						\
	"videomode=video=ctfb:x:640,y:480,depth:16,mode:0,pclk:40000,"	\
		"le:120,ri:40,up:35,lo:10,hs:30,vs:3,sync:100663296,"	\
		"vmode:0\0"						\

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
		sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		32	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* memtest works on first 255MB of RAM */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + 0xff000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ			1000

#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_HUSH_PARSER			/* Use the HUSH parser */

#define CONFIG_MISC_INIT_R

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE
#define PHYS_SDRAM_1_SIZE	0x10000000	/* 256 MB */

/*
 * NAND driver
 */

#ifndef __ASSEMBLY__
extern void qong_nand_plat_init(void *chip);
extern int qong_nand_rdy(void *chip);
#endif
#define CONFIG_NAND_PLAT
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE	CS3_BASE
#define NAND_PLAT_INIT() qong_nand_plat_init(nand)

#define QONG_NAND_CLE(chip) ((unsigned long)(chip)->IO_ADDR_W | (1 << 24))
#define QONG_NAND_ALE(chip) ((unsigned long)(chip)->IO_ADDR_W | (1 << 23))
#define QONG_NAND_WRITE(addr, cmd) \
	do { \
		__REG8(addr) = cmd; \
	} while (0)

#define NAND_PLAT_WRITE_CMD(chip, cmd) QONG_NAND_WRITE(QONG_NAND_CLE(chip), cmd)
#define NAND_PLAT_WRITE_ADR(chip, cmd) QONG_NAND_WRITE(QONG_NAND_ALE(chip), cmd)
#define NAND_PLAT_DEV_READY(chip)      (qong_nand_rdy(chip))

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	1024
/* Monitor at beginning of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		0x40000		/* Reserve 256KiB */

#define	CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x80000)

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
/* Flash memory is CFI compliant */
#define CONFIG_SYS_FLASH_CFI
/* Use drivers/cfi_flash.c */
#define CONFIG_FLASH_CFI_DRIVER
/* Use buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
/* Use hardware sector protection */
#define CONFIG_SYS_FLASH_PROTECTION

/*
 * Filesystem
 */
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,"		\
				"nand0=gen_nand"
#define MTDPARTS_DEFAULT	\
	"mtdparts=physmap-flash.0:"				\
			"512k(U-Boot),128k(env1),128k(env2),"	\
			"2304k(kernel),13m(ramdisk),-(user);"	\
		"gen_nand:"					\
			"128m(nand)"

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE		IRAM_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)

#define CONFIG_BOARD_EARLY_INIT_F

#endif /* __CONFIG_H */
