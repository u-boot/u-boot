/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Compact Integrator board.
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

/* Integrator-specific configuration */
#define CONFIG_INTEGRATOR
#define CONFIG_ARCH_CINTEGRATOR
#define CONFIG_CM_INIT
#define CONFIG_CM_REMAP
#define CONFIG_CM_SPD_DETECT

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_TEXT_BASE		0x01000000
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer 1 is clocked at 1Mhz */
#define CONFIG_SYS_TIMERBASE		0x13000100

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R		1	/* call misc_init_r during start up */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */
#define CONFIG_SMC91111
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC91111_BASE    0xC8000000
#undef CONFIG_SMC91111_EXT_PHY

/*
 * NS16550 Configuration
 */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK	14745600
#define CONFIG_PL01x_PORTS	{ (void *)CONFIG_SYS_SERIAL0, (void *)CONFIG_SYS_SERIAL1 }
#define CONFIG_CONS_INDEX	0
#define CONFIG_BAUDRATE		38400
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_SERIAL0		0x16000000
#define CONFIG_SYS_SERIAL1		0x17000000


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS	"root=/dev/mtdblock0 console=ttyAMA0 console=tty ip=dhcp netdev=27,0,0xfc800000,0xfc800010,eth0 video=clcdfb:0"
#define CONFIG_BOOTCOMMAND "tftpboot ; bootm"
#define CONFIG_SERVERIP 192.168.1.100
#define CONFIG_IPADDR 192.168.1.104
#define CONFIG_BOOTFILE "uImage"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory */
#define CONFIG_SYS_PROMPT	"Integrator-CP # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256			/* Console I/O Buffer Size*/
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16			/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE		/* Boot Argument Buffer Size*/

#define CONFIG_SYS_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* 128 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE PHYS_SDRAM_1_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET (CONFIG_SYS_SDRAM_BASE + \
				    CONFIG_SYS_INIT_RAM_SIZE - \
				    GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * FLASH and environment organization

 * Top varies according to amount fitted
 * Reserve top 4 blocks of flash
 * - ARM Boot Monitor
 * - Unused
 * - SIB block
 * - U-Boot environment
 *
 * Base is always 0x24000000

 */
#define CONFIG_SYS_FLASH_BASE		0x24000000
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define CONFIG_SYS_MAX_FLASH_SECT	64
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define PHYS_FLASH_SIZE			0x01000000	/* 16MB */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Write */

#define CONFIG_SYS_MONITOR_LEN		0x00100000
#define CONFIG_ENV_IS_IN_FLASH	1

/*
 * Move up the U-Boot & monitor area if more flash is fitted.
 * If this U-Boot is to be run on Integrators with varying flash sizes,
 * drivers/mtd/cfi_flash.c::flash_init() can read the Integrator CP_FLASHPROG
 * register and dynamically assign CONFIG_ENV_ADDR & CONFIG_SYS_MONITOR_BASE
 * - CONFIG_SYS_MONITOR_BASE is set to indicate that the environment is not
 * embedded in the boot monitor(s) area
 */
#if ( PHYS_FLASH_SIZE == 0x04000000 )

#define CONFIG_ENV_ADDR		0x27F00000
#define CONFIG_SYS_MONITOR_BASE	0x27F40000

#elif (PHYS_FLASH_SIZE == 0x02000000 )

#define CONFIG_ENV_ADDR		0x25F00000
#define CONFIG_SYS_MONITOR_BASE	0x25F40000

#else

#define CONFIG_ENV_ADDR		0x24F00000
#define CONFIG_SYS_MONITOR_BASE	0x27F40000

#endif

#define CONFIG_ENV_SECT_SIZE	0x40000		/* 256KB */
#define CONFIG_ENV_SIZE		8192		/* 8KB */

/*
 * The ARM boot monitor initializes the board.
 * However, the default U-Boot code also performs the initialization.
 * If desired, this can be prevented by defining SKIP_LOWLEVEL_INIT
 * - see documentation supplied with board for details of how to choose the
 * image to run at reset/power up
 * e.g. whether the ARM Boot Monitor runs before U-Boot

#define CONFIG_SKIP_LOWLEVEL_INIT

 */

/*
 * The ARM boot monitor does not relocate U-Boot.
 * However, the default U-Boot code performs the relocation check,
 * and may relocate the code if the memory map is changed.
 * If necessary this can be prevented by defining SKIP_RELOCATE_UBOOT

#define SKIP_CONFIG_RELOCATE_UBOOT

 */
/*-----------------------------------------------------------------------
 * There are various dependencies on the core module (CM) fitted
 * Users should refer to their CM user guide
 * - when porting adjust u-boot/Makefile accordingly
 * to define the necessary CONFIG_ s for the CM involved
 * see e.g. cp_926ejs_config
 */

#include "armcoremodule.h"

/*
 * If CONFIG_SKIP_LOWLEVEL_INIT is not defined &
 * the core module has a CM_INIT register
 * then the U-Boot initialisation code will
 * e.g. ARM Boot Monitor or pre-loader is repeated once
 * (to re-initialise any existing CM_INIT settings to safe values).
 *
 * This is usually not the desired behaviour since the platform
 * will either reboot into the ARM monitor (or pre-loader)
 * or continuously cycle thru it without U-Boot running,
 * depending upon the setting of Integrator/CP switch S2-4.
 *
 * However it may be needed if Integrator/CP switch S2-1
 * is set OFF to boot direct into U-Boot.
 * In that case comment out the line below.
#undef	CONFIG_CM_INIT
 */

#endif /* __CONFIG_H */
