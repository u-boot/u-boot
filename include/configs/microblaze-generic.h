/*
 * (C) Copyright 2007-2010 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/xilinx/microblaze-generic/xparameters.h"

/* MicroBlaze CPU */
#define	CONFIG_MICROBLAZE	1
#define	MICROBLAZE_V5		1

/* uart */
#ifdef XILINX_UARTLITE_BASEADDR
# define CONFIG_XILINX_UARTLITE
# define CONFIG_SERIAL_BASE	XILINX_UARTLITE_BASEADDR
# define CONFIG_BAUDRATE	XILINX_UARTLITE_BAUDRATE
# define CONFIG_SYS_BAUDRATE_TABLE	{ CONFIG_BAUDRATE }
# define CONSOLE_ARG	"console=console=ttyUL0,115200\0"
#elif XILINX_UART16550_BASEADDR
# define CONFIG_SYS_NS16550		1
# define CONFIG_SYS_NS16550_SERIAL
# if defined(__MICROBLAZEEL__)
#  define CONFIG_SYS_NS16550_REG_SIZE	-4
# else
#  define CONFIG_SYS_NS16550_REG_SIZE	4
# endif
# define CONFIG_CONS_INDEX		1
# define CONFIG_SYS_NS16550_COM1 \
		((XILINX_UART16550_BASEADDR & ~0xF) + 0x1000)
# define CONFIG_SYS_NS16550_CLK	XILINX_UART16550_CLOCK_HZ
# define CONFIG_BAUDRATE	115200

/* The following table includes the supported baudrates */
# define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}
# define CONSOLE_ARG	"console=console=ttyS0,115200\0"
#else
# error Undefined uart
#endif

/* setting reset address */
/*#define	CONFIG_SYS_RESET_ADDRESS	CONFIG_SYS_TEXT_BASE*/

/* ethernet */
#undef CONFIG_SYS_ENET
#if defined(XILINX_EMACLITE_BASEADDR)
# define CONFIG_XILINX_EMACLITE	1
# define CONFIG_SYS_ENET
#endif
#if defined(XILINX_LLTEMAC_BASEADDR)
# define CONFIG_XILINX_LL_TEMAC	1
# define CONFIG_SYS_ENET
#endif
#if defined(XILINX_AXIEMAC_BASEADDR)
# define CONFIG_XILINX_AXIEMAC	1
# define CONFIG_SYS_ENET
#endif

#undef ET_DEBUG

/* gpio */
#ifdef XILINX_GPIO_BASEADDR
# define CONFIG_SYS_GPIO_0		1
# define CONFIG_SYS_GPIO_0_ADDR		XILINX_GPIO_BASEADDR
#endif

/* interrupt controller */
#ifdef XILINX_INTC_BASEADDR
# define CONFIG_SYS_INTC_0		1
# define CONFIG_SYS_INTC_0_ADDR		XILINX_INTC_BASEADDR
# define CONFIG_SYS_INTC_0_NUM		XILINX_INTC_NUM_INTR_INPUTS
#endif

/* timer */
#ifdef XILINX_TIMER_BASEADDR
# if (XILINX_TIMER_IRQ != -1)
#  define CONFIG_SYS_TIMER_0		1
#  define CONFIG_SYS_TIMER_0_ADDR	XILINX_TIMER_BASEADDR
#  define CONFIG_SYS_TIMER_0_IRQ	XILINX_TIMER_IRQ
#  define FREQUENCE	XILINX_CLOCK_FREQ
#  define CONFIG_SYS_TIMER_0_PRELOAD	( FREQUENCE/1000 )
# endif
#elif XILINX_CLOCK_FREQ
# define CONFIG_XILINX_CLOCK_FREQ	XILINX_CLOCK_FREQ
#else
# error BAD CLOCK FREQ
#endif
/* FSL */
/* #define	CONFIG_SYS_FSL_2 */
/* #define	FSL_INTR_2	1 */

/*
 * memory layout - Example
 * CONFIG_SYS_TEXT_BASE = 0x1200_0000;
 * CONFIG_SYS_SRAM_BASE = 0x1000_0000;
 * CONFIG_SYS_SRAM_SIZE = 0x0400_0000;
 *
 * CONFIG_SYS_GBL_DATA_OFFSET = 0x1000_0000 + 0x0400_0000 - 0x1000 = 0x13FF_F000
 * CONFIG_SYS_MONITOR_BASE = 0x13FF_F000 - 0x40000 = 0x13FB_F000
 * CONFIG_SYS_MALLOC_BASE = 0x13FB_F000 - 0x40000 = 0x13F7_F000
 *
 * 0x1000_0000	CONFIG_SYS_SDRAM_BASE
 *					FREE
 * 0x1200_0000	CONFIG_SYS_TEXT_BASE
 *		U-BOOT code
 * 0x1202_0000
 *					FREE
 *
 *					STACK
 * 0x13F7_F000	CONFIG_SYS_MALLOC_BASE
 *					MALLOC_AREA	256kB	Alloc
 * 0x11FB_F000	CONFIG_SYS_MONITOR_BASE
 *					MONITOR_CODE	256kB	Env
 * 0x13FF_F000	CONFIG_SYS_GBL_DATA_OFFSET
 *					GLOBAL_DATA	4kB	bd, gd
 * 0x1400_0000	CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE
 */

/* ddr sdram - main memory */
#define	CONFIG_SYS_SDRAM_BASE		XILINX_RAM_START
#define	CONFIG_SYS_SDRAM_SIZE		XILINX_RAM_SIZE
#define	CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x1000)

/* global pointer */
/* start of global data */
#define	CONFIG_SYS_GBL_DATA_OFFSET \
		(CONFIG_SYS_SDRAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* monitor code */
#define	SIZE				0x40000
#define	CONFIG_SYS_MONITOR_LEN		SIZE
#define	CONFIG_SYS_MONITOR_BASE	\
		(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_GBL_DATA_OFFSET \
			- CONFIG_SYS_MONITOR_LEN - GENERATED_BD_INFO_SIZE)
#define	CONFIG_SYS_MONITOR_END \
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_SYS_MALLOC_LEN		SIZE
#define	CONFIG_SYS_MALLOC_BASE \
			(CONFIG_SYS_MONITOR_BASE - CONFIG_SYS_MALLOC_LEN)

/* stack */
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_MALLOC_BASE

/*#define	RAMENV */
#define	FLASH

#ifdef FLASH
# define CONFIG_SYS_FLASH_BASE		XILINX_FLASH_START
# define CONFIG_SYS_FLASH_SIZE		XILINX_FLASH_SIZE
# define CONFIG_SYS_FLASH_CFI		1
# define CONFIG_FLASH_CFI_DRIVER	1
/* ?empty sector */
# define CONFIG_SYS_FLASH_EMPTY_INFO	1
/* max number of memory banks */
# define CONFIG_SYS_MAX_FLASH_BANKS	1
/* max number of sectors on one chip */
# define CONFIG_SYS_MAX_FLASH_SECT	512
/* hardware flash protection */
# define CONFIG_SYS_FLASH_PROTECTION

# ifdef	RAMENV
#  define CONFIG_ENV_IS_NOWHERE	1
#  define CONFIG_ENV_SIZE	0x1000
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

# else	/* !RAMENV */
#  define CONFIG_ENV_IS_IN_FLASH	1
/* 128K(one sector) for env */
#  define CONFIG_ENV_SECT_SIZE	0x20000
#  define CONFIG_ENV_ADDR \
			(CONFIG_SYS_FLASH_BASE + (2 * CONFIG_ENV_SECT_SIZE))
#  define CONFIG_ENV_SIZE	0x20000
# endif /* !RAMBOOT */
#else /* !FLASH */
/* ENV in RAM */
# define CONFIG_SYS_NO_FLASH	1
# define CONFIG_ENV_IS_NOWHERE	1
# define CONFIG_ENV_SIZE	0x1000
# define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)
/* hardware flash protection */
# define CONFIG_SYS_FLASH_PROTECTION
#endif /* !FLASH */

/* system ace */
#ifdef XILINX_SYSACE_BASEADDR
# define CONFIG_SYSTEMACE
/* #define DEBUG_SYSTEMACE */
# define SYSTEMACE_CONFIG_FPGA
# define CONFIG_SYS_SYSTEMACE_BASE	XILINX_SYSACE_BASEADDR
# define CONFIG_SYS_SYSTEMACE_WIDTH	XILINX_SYSACE_MEM_WIDTH
# define CONFIG_DOS_PARTITION
#endif

#if defined(XILINX_USE_ICACHE)
# define CONFIG_ICACHE
#else
# undef CONFIG_ICACHE
#endif

#if defined(XILINX_USE_DCACHE)
# define CONFIG_DCACHE
#else
# undef CONFIG_DCACHE
#endif

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

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MFSL
#define CONFIG_CMD_ECHO

#if defined(CONFIG_DCACHE) || defined(CONFIG_ICACHE)
# define CONFIG_CMD_CACHE
#else
# undef CONFIG_CMD_CACHE
#endif

#ifndef CONFIG_SYS_ENET
# undef CONFIG_CMD_NET
# undef CONFIG_CMD_NFS
#else
# define CONFIG_CMD_PING
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_TFTPPUT
#endif

#if defined(CONFIG_SYSTEMACE)
# define CONFIG_CMD_EXT2
# define CONFIG_CMD_FAT
#endif

#if defined(FLASH)
# define CONFIG_CMD_ECHO
# define CONFIG_CMD_FLASH
# define CONFIG_CMD_IMLS
# define CONFIG_CMD_JFFS2

# if !defined(RAMENV)
#  define CONFIG_CMD_SAVEENV
#  define CONFIG_CMD_SAVES
# endif
#else
# undef CONFIG_CMD_IMLS
# undef CONFIG_CMD_FLASH
# undef CONFIG_CMD_JFFS2
#endif

#if defined(CONFIG_CMD_JFFS2)
/* JFFS2 partitions */
#define CONFIG_CMD_MTDPARTS	/* mtdparts command line support */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=flash-0"

/* default mtd partition table */
#define MTDPARTS_DEFAULT	"mtdparts=flash-0:256k(u-boot),"\
				"256k(env),3m(kernel),1m(romfs),"\
				"1m(cramfs),-(jffs2)"
#endif

/* Miscellaneous configurable options */
#define	CONFIG_SYS_PROMPT	"U-Boot-mONStR> "
/* size of console buffer */
#define	CONFIG_SYS_CBSIZE	512
 /* print buffer size */
#define	CONFIG_SYS_PBSIZE \
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
/* max number of command args */
#define	CONFIG_SYS_MAXARGS	15
#define	CONFIG_SYS_LONGHELP
/* default load address */
#define	CONFIG_SYS_LOAD_ADDR	XILINX_RAM_START

#define	CONFIG_BOOTDELAY	-1	/* -1 disables auto-boot */
#define	CONFIG_BOOTARGS		"root=romfs"
#define	CONFIG_HOSTNAME		XILINX_BOARD_NAME
#define	CONFIG_BOOTCOMMAND	"base 0;tftp 11000000 image.img;bootm"
#define	CONFIG_IPADDR		192.168.0.3
#define	CONFIG_SERVERIP		192.168.0.5
#define	CONFIG_GATEWAYIP	192.168.0.1
#define	CONFIG_ETHADDR		00:E0:0C:00:00:FD

/* architecture dependent code */
#define	CONFIG_SYS_USR_EXCEP	/* user exception */
#define CONFIG_SYS_HZ	1000

#define	CONFIG_PREBOOT	"echo U-BOOT for ${hostname};setenv preboot;echo"

#define	CONFIG_EXTRA_ENV_SETTINGS	"unlock=yes\0" \
					"nor0=flash-0\0"\
					"mtdparts=mtdparts=flash-0:"\
					"256k(u-boot),256k(env),3m(kernel),"\
					"1m(romfs),1m(cramfs),-(jffs2)\0"

#define CONFIG_CMDLINE_EDITING

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef CONFIG_SYS_HUSH_PARSER
# define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif

/* Enable flat device tree support */
#define CONFIG_LMB		1
#define CONFIG_FIT		1
#define CONFIG_OF_LIBFDT	1

#if defined(CONFIG_XILINX_LL_TEMAC) || defined(CONFIG_XILINX_AXIEMAC)
# define CONFIG_MII		1
# define CONFIG_CMD_MII		1
# define CONFIG_PHY_GIGE	1
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN	1
# define CONFIG_PHYLIB		1
# define CONFIG_PHY_ATHEROS	1
# define CONFIG_PHY_BROADCOM	1
# define CONFIG_PHY_DAVICOM	1
# define CONFIG_PHY_LXT		1
# define CONFIG_PHY_MARVELL	1
# define CONFIG_PHY_MICREL	1
# define CONFIG_PHY_NATSEMI	1
# define CONFIG_PHY_REALTEK	1
# define CONFIG_PHY_VITESSE	1
#else
# undef CONFIG_MII
# undef CONFIG_CMD_MII
# undef CONFIG_PHYLIB
#endif

#endif	/* __CONFIG_H */
