/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC866		1	/* This is a MPC866 CPU		*/
#define CONFIG_MGSUVD		1	/* ...on a mgsuvd board	*/

/* Do boardspecific init */
#define CONFIG_BOARD_EARLY_INIT_R       1

#define CONFIG_8xx_GCLK_FREQ		66000000

#define CFG_SMC_UCODE_PATCH	1	/* Relocate SMC1 */
#define CFG_SMC_DPMEM_OFFSET	0x1fc0
#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/

#define CONFIG_BOOTCOUNT_LIMIT

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"addcons=setenv bootargs ${bootargs} console=ttyCPM0,${baudrate}\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp ${kernel_addr} ${bootfile}; "			\
		"tftp ${fdt_addr} ${fdt_file}; run nfsargs addip addcons;"\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"rootpath=/opt/eldk/ppc_8xx\0"					\
	"bootfile=/tftpboot/mgsuvd/uImage\0"				\
	"fdt_addr=400000\0"						\
	"kernel_addr=200000\0"						\
	"fdt_file=/tftpboot/mgsuvd/mgsuvd.dtb\0"			\
	"load=tftp 200000 ${u-boot}\0"					\
	"update=protect off f0000000 +${filesize};"			\
		"erase f0000000 +${filesize};"				\
		"cp.b 200000 f0000000 ${filesize};"			\
		"protect on f0000000 +${filesize}\0"			\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE

#undef CONFIG_RTC_MPC8xx		/* MPC866 does not support RTC	*/

#define	CONFIG_TIMESTAMP		/* but print image timestmps	*/

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CFG_HUSH_PARSER		1	/* Use the HUSH parser		*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xf0000000
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_FLASH_SIZE		32
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_MAX_FLASH_SECT	256	/* max num of sects on one chip */


#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x40000 /*   Offset   of Environment Sector	*/
#define CFG_ENV_SIZE		0x08000 /* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x40000 /* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET+CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

#define	CFG_USE_PPCENV			/* Environment embedded in sect .ppcenv */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#define CFG_SYPCR	0xffffff89

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR	0x00610480

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	0x01800000
#define CFG_SCCR	0x01800000

#define CFG_DER 0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xf0000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing: Default value of OR0 after reset
 */
#define CFG_OR0_PRELIM	0xfe000954
#define CFG_BR0_PRELIM	0xf0000401

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE1_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define SDRAM_MAX_SIZE		(64 << 20)	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000A00

#define CFG_OR1_PRELIM	0xfc000800
#define CFG_BR1_PRELIM	(0x000000C0 | 0x01)

#define CFG_MPTPR	0x0200
/* PTB=16, AMB=001, FIXME 1 RAS precharge cycles, 1 READ loop cycle (not used),
   1 Write loop Cycle (not used), 1 Timer Loop Cycle */
#define CFG_MBMR	0x10964111
#define CFG_MAR		0x00000088

/*
 * 4096	Rows from SDRAM example configuration
 * 1000	factor s -> ms
 * 64	PTP (pre-divider from MPTPR) from SDRAM example configuration
 * 4	Number of refresh cycles per period
 * 64	Refresh cycle in ms per number of rows
 */
#define CFG_PTA_PER_CLK	((4096 * 64 * 1000) / (4 * 64))

/* GPIO/PIGGY on CS3 initialization values
*/
#define CFG_PIGGY_BASE	(0x30000000)
#define CFG_OR3_PRELIM	(0xfe000d24)
#define CFG_BR3_PRELIM	(0x30000401)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#define CONFIG_SCC3_ENET
#define CONFIG_ETHPRIME		"SCC ETHERNET"
#define CONFIG_HAS_ETH0

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,866@0"
#define OF_SOC			"soc@fff00000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc/cpm/serial@a80"

#endif	/* __CONFIG_H */
