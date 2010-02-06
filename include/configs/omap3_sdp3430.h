/*
 * (C) Copyright 2006-2009
 * Texas Instruments Incorporated.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 * Nishanth Menon <nm@ti.com>
 *
 * Configuration settings for the 3430 TI SDP3430 board.
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

/* TODO: REMOVE THE FOLLOWING
 * Retained the following till size.h is removed in u-boot
 */
#include <asm/sizes.h>
/*
 * High Level Configuration Options
 */
#define CONFIG_ARMCORTEXA8	1	/* This is an ARM V7 CPU core */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP3430		1	/* which is in a 3430 */
#define CONFIG_OMAP3_3430SDP	1	/* working with SDP Rev2 */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * NOTE:  these #defines presume standard SDP jumper settings.
 * In particular:
 *  - 26 MHz clock (not 19.2 or 38.4 MHz)
 *  - Boot from 128MB NOR, not NAND or OneNAND
 *
 * At this writing, OMAP3 U-Boot support doesn't permit concurrent
 * support for all the flash types the board supports.
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#undef CONFIG_USE_IRQ			/* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

/*
 * Size of malloc() pool
 * Total Size Environment - 256k
 * Malloc - add 256k
 */
#define CONFIG_ENV_SIZE			(256 << 10)
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (256 << 10))
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* bytes reserved for */
						/* initial data */

/*--------------------------------------------------------------------------*/

/*
 * Hardware drivers
 */

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER		1

/*
 * serial port - NS16550 compatible
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/* Original SDP u-boot used UART1 and thus J8 (innermost); that can be
 * swapped with UART2 via jumpering.  Downsides of using J8:  it doesn't
 * support UART boot (that's only for UART3); it prevents sharing a Linux
 * kernel (LL_DEBUG_UART3) or filesystem (getty ttyS2) with most boards.
 *
 * UART boot uses UART3 on J9, and the SDP user's guide says to use
 * that for console.  Downsides of using J9:  you can't use IRDA too;
 * since UART3 isn't in the CORE power domain, it may be a bit less
 * usable in certain PM-sensitive debug scenarios.
 */
#undef CONSOLE_J9			/* else J8/UART1 (innermost) */

#ifdef CONSOLE_J9
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3	/* UART3 */
#else
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_SERIAL1			1	/* UART1 */
#endif

#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/*
 * I2C for power management setup
 */
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_DRIVER_OMAP34XX_I2C	1

/* DDR - I use Infineon DDR */
#define CONFIG_OMAP3_INFINEON_DDR	1

/* OMITTED:  single 1 Gbit MT29F1G NAND flash */

/*
 * NOR boot support - single 1 Gbit PF48F6000M0 Strataflash
 */
#define CONFIG_SYS_FLASH_BASE		0x10000000
#define CONFIG_FLASH_CFI_DRIVER		1	/* Use drivers/cfi_flash.c */
#define CONFIG_SYS_FLASH_CFI		1	/* use CFI geometry data */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* ~10x faster writes */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* hardware sector protection */
#define CONFIG_SYS_FLASH_EMPTY_INFO	1	/* flinfo 'E' for empty */
#define CONFIG_SYS_FLASH_BANKS_LIST     {CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_BANKS      1	/* max number of flash banks */

#define CONFIG_SYS_FLASH_CFI_WIDTH	2
#define PHYS_FLASH_SIZE			(128 << 20)
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max sectors on one chip */

/* timeout values are in milliseconds */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100 * CONFIG_SYS_HZ)

/* OMITTED:  single 2 Gbit KFM2G16 OneNAND flash */

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_SYS_ENV_SECT_SIZE	(256 << 10)
#define CONFIG_ENV_OFFSET		CONFIG_SYS_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_ENV_SECT_SIZE)
/*--------------------------------------------------------------------------*/

/* commands to include */
#include <config_cmd_default.h>

/* Enabled commands */
#define CONFIG_CMD_DHCP		/* DHCP Support			*/
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NET

/* Disabled commands */
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

/*--------------------------------------------------------------------------*/
/*
 * MMC boot support
 */

#if defined(CONFIG_CMD_MMC)
#define CONFIG_MMC			1
#define CONFIG_OMAP3_MMC		1
#define CONFIG_DOS_PARTITION		1
#endif

/*----------------------------------------------------------------------------
 * SMSC9115 Ethernet from SMSC9118 family
 *----------------------------------------------------------------------------
 */
#if defined(CONFIG_CMD_NET)

#define CONFIG_NET_MULTI
#define CONFIG_LAN91C96
#define CONFIG_LAN91C96_BASE	DEBUG_BASE
#define CONFIG_LAN91C96_EXT_PHY

#define CONFIG_BOOTP_SEND_HOSTNAME
/*
 * BOOTP fields
 */
#define CONFIG_BOOTP_SUBNETMASK		0x00000001
#define CONFIG_BOOTP_GATEWAY		0x00000002
#define CONFIG_BOOTP_HOSTNAME		0x00000004
#define CONFIG_BOOTP_BOOTPATH		0x00000010
#endif /* (CONFIG_CMD_NET) */

/*
 * Environment setup
 *
 * Default boot order:  mmc bootscript, MMC uImage, NOR image.
 * Network booting environment must be configured at site.
 */

/* allow overwriting serial config and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyS0,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext3 rootwait\0" \
	"norargs=setenv bootargs console=${console} " \
		"root=/dev/mtdblock3 rw " \
		"rootfstype=jffs2\0" \
	"loadbootscript=fatload mmc 0 ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from MMC/SD ...; " \
		"autoscr ${loadaddr}\0" \
	"loaduimage=fatload mmc 0 ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from MMC/SD ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"norboot=echo Booting from NOR ...; " \
		"run norargs; " \
		"bootm 0x80000\0" \

#define CONFIG_BOOTCOMMAND \
	"if mmcinit; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run norboot; " \
			"fi; " \
		"fi; " \
	"else run norboot; fi"

#define CONFIG_AUTO_COMPLETE		1

/*--------------------------------------------------------------------------*/

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"OMAP34XX SDP # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

/* SDRAM Test range - start at 16 meg boundary -ends at 32Meg -
 * a basic sanity check ONLY
 * IF you would like to increase coverage, increase the end address
 * or run the test with custom options
 */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0 + 0x01000000)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + (32 << 20))

/* Default load address */
#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)

/*--------------------------------------------------------------------------*/

/*
 * 3430 has 12 GP timers, they can be driven by the SysClk (12/13/19.2) or by
 * 32KHz clk, or from external sig. This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10) /* Regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4 << 10) /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4 << 10) /* FIQ stack */
#endif

/*
 * SDRAM Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20) /* at least 32 meg */
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/* SDRAM Bank Allocation method */
#define SDRC_R_B_C		1

/*--------------------------------------------------------------------------*/

/*
 * NOR FLASH usage ... default nCS0:
 *  - one 256KB sector for U-Boot
 *  - one 256KB sector for its parameters (not all used)
 *  - eight sectors (2 MB) for kernel
 *  - rest for JFFS2
 */

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)

#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

/*
 * NAND FLASH usage ... default nCS1:
 *  - four 128KB sectors for X-Loader
 *  - four 128KB sectors for U-Boot
 *  - two 128KB sector for its parameters
 *  - 32 sectors (4 MB) for kernel
 *  - rest for filesystem
 */

/*
 * OneNAND FLASH usage ... default nCS2:
 *  - four 128KB sectors for X-Loader
 *  - two 128KB sectors for U-Boot
 *  - one 128KB sector for its parameters
 *  - sixteen sectors (2 MB) for kernel
 *  - rest for filesystem
 */

/*--------------------------------------------------------------------------*/

#ifndef __ASSEMBLY__
extern unsigned int boot_flash_base;
extern volatile unsigned int boot_flash_env_addr;
extern unsigned int boot_flash_off;
extern unsigned int boot_flash_sec;
extern unsigned int boot_flash_type;
#endif

#endif				/* __CONFIG_H */
