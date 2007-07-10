/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
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

/*------------------------------------------------------------------------
 * BOARD/CPU
 *----------------------------------------------------------------------*/
#define	CONFIG_PCI5441		1		/* PCI-5441 board	*/
#define CONFIG_SYS_CLK_FREQ	50000000	/* 50 MHz core clk	*/

#define CFG_RESET_ADDR		0x00000000	/* Hard-reset address	*/
#define CFG_EXCEPTION_ADDR	0x01000020	/* Exception entry point*/
#define CFG_NIOS_SYSID_BASE	0x00920828	/* System id address	*/
#define	CONFIG_BOARD_EARLY_INIT_F 1	/* enable early board-spec. init*/

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

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION
 * 	-Monitor at top.
 * 	-The heap is placed below the monitor.
 * 	-Global data is placed below the heap.
 * 	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(128 * 1024)	/* Reserve 128k		*/
#define CFG_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)

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
#define CFG_FLASH_WORD_SIZE	unsigned char	/* flash word size	*/

/*------------------------------------------------------------------------
 * ENVIRONMENT -- Put environment in sector CFG_MONITOR_LEN above
 * CFG_RESET_ADDR, since we assume the monitor is stored at the
 * reset address, no? This will keep the environment in user region
 * of flash. NOTE: the monitor length must be multiple of sector size
 * (which is common practice).
 *----------------------------------------------------------------------*/
#define	CFG_ENV_IS_IN_FLASH	1		/* Environment in flash */
#define CFG_ENV_SIZE		(64 * 1024)	/* 64 KByte (1 sector)	*/
#define CONFIG_ENV_OVERWRITE			/* Serial change Ok	*/
#define CFG_ENV_ADDR 	(CFG_RESET_ADDR + CFG_MONITOR_LEN)

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#if defined(CONFIG_CONSOLE_JTAG)
#define CFG_NIOS_CONSOLE	0x00920820	/* JTAG UART base addr	*/
#else
#define CFG_NIOS_CONSOLE	0x009208a0	/* UART base addr	*/
#endif

#define CFG_NIOS_FIXEDBAUD	1		/* Baudrate is fixed	*/
#define CONFIG_BAUDRATE		115200		/* Initial baudrate	*/
#define CFG_BAUDRATE_TABLE	{115200}	/* It's fixed ;-)	*/

#define CFG_CONSOLE_INFO_QUIET	1		/* Suppress console info*/

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
#define CFG_NIOS_TMRBASE	0x00920860	/* Tick timer base addr	*/
#define CFG_NIOS_TMRIRQ		3		/* Timer IRQ num	*/
#define CFG_NIOS_TMRMS		10		/* 10 msec per tick	*/
#define CFG_NIOS_TMRCNT	(CFG_NIOS_TMRMS * (CONFIG_SYS_CLK_FREQ/1000))
#define	CFG_HZ		(CONFIG_SYS_CLK_FREQ/(CFG_NIOS_TMRCNT + 1))


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
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVES


/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define	CFG_LONGHELP				/* Provide extended help*/
#define	CFG_PROMPT		"==> "		/* Command prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O buf size	*/
#define	CFG_MAXARGS		16	    	/* Max command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE 	/* Boot arg buf size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print buf size	*/
#define	CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* Default load address	*/
#define CFG_MEMTEST_START	CFG_SDRAM_BASE	/* Start addr for test	*/
#define CFG_MEMTEST_END		CFG_INIT_SP - 0x00020000

#endif	/* __CONFIG_H */
