/*
 * (C) Copyright 2011 Comelit Group SpA
 * Luca Ceresoli <luca.ceresoli@comelit.it>
 *
 * Based on omap3_beagle.h:
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the Comelit DIG297 board.
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

#include <asm/mach-types.h>
#ifdef MACH_TYPE_OMAP3_CPS
#error "MACH_TYPE_OMAP3_CPS has been defined properly, please remove this."
#else
#define MACH_TYPE_OMAP3_CPS 2751
#endif
#define CONFIG_MACH_TYPE MACH_TYPE_OMAP3_CPS

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP		/* in a TI OMAP core */
#define CONFIG_OMAP34XX		/* which is a 34XX */

#define CONFIG_SYS_TEXT_BASE	0x80008000

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#undef CONFIG_USE_IRQ				/* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10) /* UBI needs >= 512 kB */

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration: UART3 (ttyO2)
 */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_OMAP_HSMMC		1
#define CONFIG_DOS_PARTITION

/* library portions to compile in */
#define CONFIG_RBTREE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_LZO

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_UBI		/* UBI Support			*/
#define CONFIG_CMD_UBIFS	/* UBIFS Support		*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands    */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT	"mtdparts=omap2-nand.0:896k(uboot),"\
				"128k(uboot-env),3m(kernel),252m(ubi)"

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NAND		/* NAND support			*/

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#undef CONFIG_CMD_NFS		/* NFS support			*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_DRIVER_OMAP34XX_I2C	1

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER
#define CONFIG_TWL4030_LED

/*
 * Board NAND Info.
 */
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define GPMC_NAND_ECC_LP_x16_LAYOUT

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */

#if defined(CONFIG_CMD_NET)
/*
 * SMSC9220 Ethernet
 */

#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE     0x2C000000

#endif /* (CONFIG_CMD_NET) */

/* Environment information */
#define CONFIG_BOOTDELAY		1

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyO2,115200n8\0" \
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"partition=nand0,3\0"\
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"nandroot=ubi0:rootfs ro\0" \
	"nandrootfstype=ubifs\0" \
	"nfspath=/srv/nfs\0" \
	"tftpfilename=uImage\0" \
	"gatewayip=0.0.0.0\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
			"${netmask}:${hostname}::off\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"ubi.mtd=3 " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
			"${netmask}:${hostname}::off\0" \
	"netargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"root=/dev/nfs rw " \
		"nfsroot=${serverip}:${nfspath} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
			"${netmask}:${hostname}::off\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 100000 300000; " \
		"bootm ${loadaddr}\0" \
	"netboot=echo Booting from network ...; " \
		"run netargs; " \
		"tftp ${loadaddr} ${serverip}:${tftpfilename}; " \
		"bootm ${loadaddr}\0" \
	"resetenv=nand erase e0000 20000\0"\

#define CONFIG_BOOTCOMMAND \
	"run nandboot"

#define CONFIG_AUTO_COMPLETE
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"DIG297# "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)	/* memtest */
								/* works on */
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)	/* default */
							/* load address */

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4 << 10)	/* IRQ stack 4 KiB */
#define CONFIG_STACKSIZE_FIQ	(4 << 10)	/* FIQ stack 4 KiB */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20)	/* at least 32 MiB */
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_SYS_FLASH_BASE		boot_flash_base

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_ENV_IS_IN_NAND
#define SMNAND_ENV_OFFSET		0x0E0000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			SMNAND_ENV_OFFSET

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#endif /* __CONFIG_H */
