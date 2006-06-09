/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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

/*------------------------------------------------------------------------
 * BOARD/CPU
 *----------------------------------------------------------------------*/
#define CONFIG_EP1S10		1		/* EP1S10 board		*/
#define CONFIG_SYS_CLK_FREQ	50000000	/* 50 MHz core clk	*/

#define CFG_RESET_ADDR		0x00000000	/* Hard-reset address	*/
#define CFG_EXCEPTION_ADDR	0x01000020	/* Exception entry point*/
#define CFG_NIOS_SYSID_BASE	0x021208b8	/* System id address	*/

/*------------------------------------------------------------------------
 * CACHE -- the following will support II/s and II/f. The II/s does not
 * have dcache, so the cache instructions will behave as NOPs.
 *----------------------------------------------------------------------*/
#define CFG_ICACHE_SIZE		4096		/* 4 KByte total	*/
#define CFG_ICACHELINE_SIZE	32		/* 32 bytes/line	*/
#define CFG_DCACHE_SIZE		2048		/* 2 KByte (II/f)	*/
#define CFG_DCACHELINE_SIZE	4		/* 4 bytes/line (II/f)	*/

/*------------------------------------------------------------------------
 * MEMORY BASE ADDRESSES
 *----------------------------------------------------------------------*/
#define CFG_FLASH_BASE		0x00000000	/* FLASH base addr	*/
#define CFG_FLASH_SIZE		0x00800000	/* 8 MByte		*/
#define CFG_SDRAM_BASE		0x01000000	/* SDRAM base addr	*/
#define CFG_SDRAM_SIZE		0x01000000	/* 16 MByte		*/
#define CFG_SRAM_BASE		0x02000000	/* SRAM base addr	*/
#define CFG_SRAM_SIZE		0x00100000	/* 1 MB			*/

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION
 *	-Monitor at top.
 *	-The heap is placed below the monitor.
 *	-Global data is placed below the heap.
 *	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256k		*/
#define CFG_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 256*1024) /* 256k heap */

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_OFFSET	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP		CFG_GBL_DATA_OFFSET

/*------------------------------------------------------------------------
 * FLASH (AM29LV065D)
 *----------------------------------------------------------------------*/
#define CFG_MAX_FLASH_SECT	128		/* Max # sects per bank */
#define CFG_MAX_FLASH_BANKS	1		/* Max # of flash banks */
#define CFG_FLASH_ERASE_TOUT	8000		/* Erase timeout (msec) */
#define CFG_FLASH_WRITE_TOUT	100		/* Write timeout (msec) */

/*------------------------------------------------------------------------
 * ENVIRONMENT -- Put environment in sector CFG_MONITOR_LEN above
 * CFG_FLASH_BASE, since we assume that u-boot is stored at the bottom
 * of flash memory. This will keep the environment in user region
 * of flash. NOTE: the monitor length must be multiple of sector size
 * (which is common practice).
 *----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH	1		/* Environment in flash */
#define CFG_ENV_SIZE		(64 * 1024)	/* 64 KByte (1 sector)	*/
#define CONFIG_ENV_OVERWRITE			/* Serial change Ok	*/
#define CFG_ENV_ADDR	(CFG_FLASH_BASE + CFG_MONITOR_LEN)

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#if defined(CONFIG_CONSOLE_JTAG)
#define CFG_NIOS_CONSOLE	0x021208b0	/* JTAG UART base addr	*/
#else
#define CFG_NIOS_CONSOLE	0x02120840	/* UART base addr	*/
#endif

#define CFG_NIOS_FIXEDBAUD	1		/* Baudrate is fixed	*/
#define CONFIG_BAUDRATE		115200		/* Initial baudrate	*/
#define CFG_BAUDRATE_TABLE	{115200}	/* It's fixed ;-)	*/

#define CFG_CONSOLE_INFO_QUIET	1		/* Suppress console info*/

/*------------------------------------------------------------------------
 * EPCS Device -- None for stratix.
 *----------------------------------------------------------------------*/
#undef CFG_NIOS_EPCSBASE

/*------------------------------------------------------------------------
 * DEBUG
 *----------------------------------------------------------------------*/
#undef CONFIG_ROM_STUBS				/* Stubs not in ROM	*/

/*------------------------------------------------------------------------
 * TIMEBASE --
 *
 * The high res timer defaults to 1 msec. Since it includes the period
 * registers, we can slow it down to 10 msec using TMRCNT. If the default
 * period is acceptable, TMRCNT can be left undefined.
 *----------------------------------------------------------------------*/
#define CFG_NIOS_TMRBASE	0x02120820	/* Tick timer base addr */
#define CFG_NIOS_TMRIRQ		3		/* Timer IRQ num	*/
#define CFG_NIOS_TMRMS		10		/* 10 msec per tick	*/
#define CFG_NIOS_TMRCNT (CFG_NIOS_TMRMS * (CONFIG_SYS_CLK_FREQ/1000))
#define CFG_HZ		(CONFIG_SYS_CLK_FREQ/(CFG_NIOS_TMRCNT + 1))

/*------------------------------------------------------------------------
 * STATUS LED -- Provides a simple blinking led. For Nios2 each board
 * must implement its own led routines -- since leds are board-specific.
 *----------------------------------------------------------------------*/
#define CFG_LEDPIO_ADDR		0x02120870	/* LED PIO base addr	*/
#define CONFIG_STATUS_LED			/* Enable status driver */

#define STATUS_LED_BIT		1		/* Bit-0 on PIO		*/
#define STATUS_LED_STATE	1		/* Blinking		*/
#define STATUS_LED_PERIOD	(500/CFG_NIOS_TMRMS) /* Every 500 msec	*/

/*------------------------------------------------------------------------
 * ETHERNET -- The header file for the SMC91111 driver hurts my eyes ...
 * and really doesn't need any additional clutter. So I choose the lazy
 * way out to avoid changes there -- define the base address to ensure
 * cache bypass so there's no need to monkey with inx/outx macros.
 *----------------------------------------------------------------------*/
#define CONFIG_SMC91111_BASE	0x82110300	/* Base addr (bypass)	*/
#define CONFIG_DRIVER_SMC91111			/* Using SMC91c111	*/
#undef	CONFIG_SMC91111_EXT_PHY			/* Internal PHY		*/
#define CONFIG_SMC_USE_32_BIT			/* 32-bit interface	*/

#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.2.21
#define CONFIG_SERVERIP		192.168.2.16

/*------------------------------------------------------------------------
 * COMMANDS
 *----------------------------------------------------------------------*/
#define CONFIG_COMMANDS		(CFG_CMD_BDI	| \
				 CFG_CMD_DHCP	| \
				 CFG_CMD_ECHO	| \
				 CFG_CMD_ENV	| \
				 CFG_CMD_FLASH	| \
				 CFG_CMD_IMI	| \
				 CFG_CMD_IRQ	| \
				 CFG_CMD_LOADS	| \
				 CFG_CMD_LOADB	| \
				 CFG_CMD_MEMORY | \
				 CFG_CMD_MISC	| \
				 CFG_CMD_NET	| \
				 CFG_CMD_PING	| \
				 CFG_CMD_RUN	| \
				 CFG_CMD_SAVES	)
#include <cmd_confdefs.h>

/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP				/* Provide extended help*/
#define CFG_PROMPT		"==> "		/* Command prompt	*/
#define CFG_CBSIZE		256		/* Console I/O buf size */
#define CFG_MAXARGS		16		/* Max command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot arg buf size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print buf size */
#define CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* Default load address */
#define CFG_MEMTEST_START	CFG_SDRAM_BASE	/* Start addr for test	*/
#define CFG_MEMTEST_END		CFG_INIT_SP - 0x00020000

#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "

#endif	/* __CONFIG_H */
