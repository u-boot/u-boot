/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC823		1	/* This is a MPC823 CPU		*/
#define CONFIG_V37		1	/* ...on a Marel V37 board 	*/

#define CONFIG_LCD
#define CONFIG_SHARP_LQ084V1DG21
#undef CONFIG_LCD_LOGO

/*-----------------------------------------------------------------------------
 * I2C Configuration
 *-----------------------------------------------------------------------------
 */
#define CONFIG_I2C              1
#define CFG_I2C_SLAVE           0x2

#define	CONFIG_8xx_CONS_SMC1	1
#undef	CONFIG_8xx_CONS_SMC2		/* Console is on SMC2		*/
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600	/* console baudrate = 115kbps	*/
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	2	/* autoboot after 2 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */
#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_BOOTCOMMAND							\
	"tftpboot; " 								\
	"setenv bootargs console=tty0 "                                   \
	"root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "		 	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " 	\
	"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_CAN_DRIVER	1	/* CAN Driver support enabled	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_DATE


/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor1"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor1=v37-1"
#define MTDPARTS_DEFAULT	"mtdparts=v37-1:-(jffs2)"
*/

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE0		0x40000000
#define CFG_FLASH_BASE1		0x60000000
#define CFG_FLASH_BASE		CFG_FLASH_BASE1

#if defined(DEBUG)
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#define CFG_MONITOR_BASE	CFG_FLASH_BASE0
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	35	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_NVRAM	1
#define	CFG_ENV_ADDR		0x80000000/* Address of Environment */
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

#define CFG_ENV_OFFSET		0

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
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	0xFFFFFF88
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_FRC | SIUMCR_GB5E)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
/*%%%#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE) */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR (PISCR_PS | PISCR_PITF)
/*
#define CFG_PISCR (PISCR_PS | PISCR_PITF)
*/

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 *
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
/* up to 50 MHz we use a 1:1 clock */
#define CFG_PLPRCR	( (1524 << PLPRCR_MF_SHIFT) | PLPRCR_SPLSS | PLPRCR_TMIST | PLPRCR_TEXPS )

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
/* up to 50 MHz we use a 1:1 clock */
#define CFG_SCCR	(SCCR_COM00 | SCCR_TBS)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CFG_PCMCIA_MEM_ADDR	(0xE0000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0xE4000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_IO_ADDR	(0xEC000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#undef	CONFIG_IDE_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_PCMCIA		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(2 * CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	0x0100

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER	0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0x60000000	/* FLASH bank #1	*/

#define CFG_PRELIM_OR_AM	0xFE000000	/* OR addr mask */

#define CFG_OR_TIMING_FLASH 	0xF56

#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V)

#define CFG_OR5_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR5_PRELIM	((FLASH_BASE1_PRELIM & BR_BA_MSK) | BR_PS_32 | BR_V)

/*
 * BR1 and OR1 (Battery backed SRAM)
 */
#define	CFG_BR1_PRELIM	0x80000401
#define CFG_OR1_PRELIM	0xFFC00736

/*
 * BR2 and OR2 (SDRAM)
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM base	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB */

#define CFG_OR_TIMING_SDRAM	0x00000A00

#define CFG_OR2_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_SDRAM )
#define CFG_BR2_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/* Marel V37 mem setting */

#define	CFG_BR3_CAN	0xC0000401
#define CFG_OR3_CAN	0xFFFF0724

/*
#define	CFG_BR3_PRELIM	0xFA400001
#define CFG_OR3_PRELIM	0xFFFF8910
#define	CFG_BR4_PRELIM	0xFA000401
#define CFG_OR4_PRELIM	0xFFFE0970
*/

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA	97		/* start with divider for 100 MHz	*/

/*
 * Refresh clock Prescalar
 */
#define CFG_MPTPR	MPTPR_PTP_DIV16

/*
 * MAMR settings for SDRAM
 */

/* 10 column SDRAM */
#define CFG_MAMR_10COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_2 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A12 |	\
			 MAMR_GPL_A4DIS | MAMR_RLFA_4X | MAMR_WLFA_3X | MAMR_TLFA_16X)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
