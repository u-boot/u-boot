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

#include <asm/arch/mx31-regs.h>

 /* High Level Configuration Options */
#define CONFIG_ARM1136		1	/* This is an arm1136 CPU core */
#define CONFIG_MX31		1	/* in a mx31 */
#define CONFIG_QONG		1
#define CONFIG_MX31_HCLK_FREQ	26000000	/* 26MHz */
#define CONFIG_MX31_CLK32	32768

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART	1
#define CONFIG_SYS_MX31_UART1	1

#define CONFIG_MX31_GPIO

#define CONFIG_MXC_SPI
#define CONFIG_DEFAULT_SPI_BUS	1
#define CONFIG_DEFAULT_SPI_MODE	(SPI_MODE_2 | SPI_CS_HIGH)
#define CONFIG_RTC_MC13783

#define CONFIG_FSL_PMIC
#define CONFIG_FSL_PMIC_BUS	1
#define CONFIG_FSL_PMIC_CS	0
#define CONFIG_FSL_PMIC_CLK	100000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_2 | SPI_CS_HIGH)

/* FPGA */
#define CONFIG_QONG_FPGA	1
#define CONFIG_FPGA_BASE	(CS1_BASE)

#ifdef CONFIG_QONG_FPGA
/* Ethernet */
#define CONFIG_DNET		1
#define CONFIG_DNET_BASE	(CS1_BASE + QONG_FPGA_PERIPH_SIZE)
#define CONFIG_NET_MULTI	1

/* Framebuffer and LCD */
#define CONFIG_LCD
#define CONFIG_VIDEO_MX3
#define	CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define LCD_BPP		LCD_COLOR16
#define	CONFIG_SPLASH_SCREEN
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_DISPLAY_COM57H5M10XRC

/*
 * Reducing the ARP timeout from default 5 seconds to 200ms we speed up the
 * initial TFTP transfer, should the user wish one, significantly.
 */
#define CONFIG_ARP_TIMEOUT	200UL

#endif /* CONFIG_QONG_FPGA */

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_SPI
#define CONFIG_CMD_DATE
#define BOARD_LATE_INIT

/*
 * You can compile in a MAC address and your custom net settings by using
 * the following syntax.
 *
 * #define CONFIG_ETHADDR		xx:xx:xx:xx:xx:xx
 * #define CONFIG_SERVERIP		<server ip>
 * #define CONFIG_IPADDR		<board ip>
 * #define CONFIG_GATEWAYIP		<gateway ip>
 * #define CONFIG_NETMASK		<your netmask>
 */

#define CONFIG_BOOTDELAY	5

#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

#define xstr(s)	str(s)
#define str(s)	#s

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
	"kernel_addr=A00A0000\0"					\
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
	"update=protect off " xstr(CONFIG_SYS_MONITOR_BASE)		\
		" +${filesize};era " xstr(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};cp.b ${fileaddr} "			\
		xstr(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"		\
	"upd=run load update\0"						\

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

#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser		*/
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_MISC_INIT_R	1
/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */

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

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x60000)

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
/* Flash memory is CFI compliant */
#define CONFIG_SYS_FLASH_CFI			1
/* Use drivers/cfi_flash.c */
#define CONFIG_FLASH_CFI_DRIVER			1
/* Use buffered writes (~10x faster) */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
/* Use hardware sector protection */
#define CONFIG_SYS_FLASH_PROTECTION		1

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
#define MTDIDS_DEFAULT		"nor0=physmap-flash.0"
#define MTDPARTS_DEFAULT	\
	"mtdparts=physmap-flash.0:384k(U-Boot),128k(env1),"	\
	"128k(env2),2432k(kernel),13m(ramdisk),-(user)"

#endif /* __CONFIG_H */
