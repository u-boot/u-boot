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
#define CONFIG_EP1S40		1		/* EP1S40 board		*/
#define CONFIG_SYS_CLK_FREQ	50000000	/* 50 MHz core clk	*/

#define CONFIG_SYS_RESET_ADDR		0x00000000	/* Hard-reset address	*/
#define CONFIG_SYS_EXCEPTION_ADDR	0x01000020	/* Exception entry point*/
#define CONFIG_SYS_NIOS_SYSID_BASE	0x021208b8	/* System id address	*/

/*------------------------------------------------------------------------
 * CACHE -- the following will support II/s and II/f. The II/s does not
 * have dcache, so the cache instructions will behave as NOPs.
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_ICACHE_SIZE		4096		/* 4 KByte total	*/
#define CONFIG_SYS_ICACHELINE_SIZE	32		/* 32 bytes/line	*/
#define CONFIG_SYS_DCACHE_SIZE		2048		/* 2 KByte (II/f)	*/
#define CONFIG_SYS_DCACHELINE_SIZE	4		/* 4 bytes/line (II/f)	*/

/*------------------------------------------------------------------------
 * MEMORY BASE ADDRESSES
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0x00000000	/* FLASH base addr	*/
#define CONFIG_SYS_FLASH_SIZE		0x00800000	/* 8 MByte		*/
#define CONFIG_SYS_SDRAM_BASE		0x01000000	/* SDRAM base addr	*/
#define CONFIG_SYS_SDRAM_SIZE		0x01000000	/* 16 MByte		*/
#define CONFIG_SYS_SRAM_BASE		0x02000000	/* SRAM base addr	*/
#define CONFIG_SYS_SRAM_SIZE		0x00100000	/* 1 MB			*/

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION
 *	-Monitor at top.
 *	-The heap is placed below the monitor.
 *	-Global data is placed below the heap.
 *	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256k		*/
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 256*1024) /* 256k heap */

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MONITOR_BASE - CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_MALLOC_BASE - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP		CONFIG_SYS_GBL_DATA_OFFSET

/*------------------------------------------------------------------------
 * FLASH (AM29LV065D)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* Max # sects per bank */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* Max # of flash banks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	8000		/* Erase timeout (msec) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	100		/* Write timeout (msec) */

/*------------------------------------------------------------------------
 * ENVIRONMENT -- Put environment in sector CONFIG_SYS_MONITOR_LEN above
 * CONFIG_SYS_FLASH_BASE, since we assume that u-boot is stored at the bottom
 * of flash memory. This will keep the environment in user region
 * of flash. NOTE: the monitor length must be multiple of sector size
 * (which is common practice).
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_FLASH	1		/* Environment in flash */
#define CONFIG_ENV_SIZE		(64 * 1024)	/* 64 KByte (1 sector)	*/
#define CONFIG_ENV_OVERWRITE			/* Serial change Ok	*/
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#if defined(CONFIG_CONSOLE_JTAG)
#define CONFIG_SYS_NIOS_CONSOLE	0x021208b0	/* JTAG UART base addr	*/
#else
#define CONFIG_SYS_NIOS_CONSOLE	0x02120840	/* UART base addr	*/
#endif

#define CONFIG_SYS_NIOS_FIXEDBAUD	1		/* Baudrate is fixed	*/
#define CONFIG_BAUDRATE		115200		/* Initial baudrate	*/
#define CONFIG_SYS_BAUDRATE_TABLE	{115200}	/* It's fixed ;-)	*/

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1		/* Suppress console info*/

/*------------------------------------------------------------------------
 * EPCS Device -- None for stratix.
 *----------------------------------------------------------------------*/
#undef CONFIG_SYS_NIOS_EPCSBASE

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
#define CONFIG_SYS_NIOS_TMRBASE	0x02120820	/* Tick timer base addr */
#define CONFIG_SYS_NIOS_TMRIRQ		3		/* Timer IRQ num	*/
#define CONFIG_SYS_NIOS_TMRMS		10		/* 10 msec per tick	*/
#define CONFIG_SYS_NIOS_TMRCNT (CONFIG_SYS_NIOS_TMRMS * (CONFIG_SYS_CLK_FREQ/1000))
#define CONFIG_SYS_HZ		(CONFIG_SYS_CLK_FREQ/(CONFIG_SYS_NIOS_TMRCNT + 1))

/*------------------------------------------------------------------------
 * STATUS LED -- Provides a simple blinking led. For Nios2 each board
 * must implement its own led routines -- since leds are board-specific.
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_LEDPIO_ADDR		0x02120870	/* LED PIO base addr	*/
#define CONFIG_STATUS_LED			/* Enable status driver */

#define STATUS_LED_BIT		1		/* Bit-0 on PIO		*/
#define STATUS_LED_STATE	1		/* Blinking		*/
#define STATUS_LED_PERIOD	(500/CONFIG_SYS_NIOS_TMRMS) /* Every 500 msec	*/

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
#define CONFIG_CMD_BDI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVES


/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_LONGHELP				/* Provide extended help*/
#define CONFIG_SYS_PROMPT		"==> "		/* Command prompt	*/
#define CONFIG_SYS_CBSIZE		256		/* Console I/O buf size */
#define CONFIG_SYS_MAXARGS		16		/* Max command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot arg buf size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print buf size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE	/* Default load address */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE	/* Start addr for test	*/
#define CONFIG_SYS_MEMTEST_END		CONFIG_SYS_INIT_SP - 0x00020000

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#endif	/* __CONFIG_H */
