/*
 * (C) Copyright 2003-2005
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
 * changes for 16M board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#undef CONFIG_MPC860
#define CONFIG_MPC850		1	/* This is a MPC850 CPU		*/
#define CONFIG_RPXLITE		1	/* QUANTUM is the RPXlite clone */
#define CONFIG_RMU		1   /* The QUNATUM is based on our RMU */

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600	/* console baudrate = 9600bps	*/
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

/* default developmenmt environment */

#define CONFIG_ETHADDR 00:0B:17:00:00:00

#define CONFIG_IPADDR  10.10.69.10
#define CONFIG_SERVERIP 10.10.69.49
#define CONFIG_NETMASK	255.255.255.0
#define CONFIG_HOSTNAME QUANTUM
#define CONFIG_ROOTPATH /opt/eldk/pcc_8xx

#define CONFIG_BOOTARGS	 "root=/dev/ram rw"

#define CONFIG_BOOTCOMMAND "bootm ff000000"

#define CONFIG_EXTRA_ENV_SETTINGS \
    "serial#=12345\0"		\
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath}\0"	\
	"ramargs=setenv bootargs root=/dev/ram rw\0" \
    "addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off\0"

/*
 * Select the more full-featured memory test (Barr embedded systems)
 */
#define CFG_ALT_MEMTEST

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/


/* M48T02 Paralled access timekeeper with same interface as the M48T35A*/
#define CONFIG_RTC_M48T35A 1

#if 0
#define CONFIG_WATCHDOG 1		/* watchdog enabled		*/
#else
#undef CONFIG_WATCHDOG
#endif

/*  NVRAM and RTC */
#define CFG_NVRAM_BASE_ADDR 0xFA000000
#define CFG_NVRAM_SIZE 2048


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SNTP


#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define CONFIG_AUTOBOOT_KEYED	/* Enable password protection */
#define CONFIG_AUTOBOOT_PROMPT		"\nEnter password - autoboot in %d sec...\n"
#define CONFIG_AUTOBOOT_DELAY_STR	"system"
/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00040000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x01f00000	/* 256K ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFA200000

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
#define CFG_FLASH_BASE	0xFF000000

#if 1
    #define CFG_FLASH_CFI_DRIVER
#else
    #undef CFG_FLASH_CFI_DRIVER
#endif


#ifdef CFG_FLASH_CFI_DRIVER
    #define CFG_FLASH_CFI 1
    #undef CFG_FLASH_USE_BUFFER_WRITE
    #define CFG_FLASH_BANKS_LIST {CFG_FLASH_BASE}
#endif

/*%%% #define CFG_FLASH_BASE		0xFFF00000 */
#if defined(DEBUG) || defined(CONFIG_CMD_IDE)
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define CFG_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#endif
#define CFG_MONITOR_BASE	0xFFF00000
/*%%% #define CFG_MONITOR_BASE	CFG_FLASH_BASE */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

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
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET	    0x00F40000	/*   Offset   of Environment Sector	absolute address 0xfff40000*/
#define CFG_ENV_SECT_SIZE	0x40000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_ENV_OFFSET)

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/* FPGA */
#define CONFIG_MISC_INIT_R
#define CFG_FPGA_SPARTAN2
#define CFG_FPGA_PROG_FEEDBACK


/*-----------------------------------------------------------------------
 * Reset address
 */
#define CFG_RESET_ADDRESS	((ulong)((((immap_t *)CFG_IMMR)->im_clkrst.res)))

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
#define CFG_SYPCR	(SYPCR_SWTC | 0x00000600 | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_MLRC10)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF | TBSCR_TBE)

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

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 *
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
/* up to 50 MHz we use a 1:1 clock */
#define CFG_PLPRCR	( (5 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS )

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
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

#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/
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
/*#define	CFG_DER 0x2002000F*/
#define CFG_DER 0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE_PRELIM	0xFE000000	/* FLASH base */
#define CFG_PRELIM_OR_AM	0xFE000000	/* OR addr mask */

/* FLASH timing: ACS = 0, TRLX = 0, CSNT = 0, SCY = 4, ETHR = 0, BIH = 1 */
#define CFG_OR_TIMING_FLASH (OR_SCY_4_CLK | OR_BI)

#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE_PRELIM & BR_BA_MSK) | BR_V)

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM base	*/
#define SDRAM_MAX_SIZE		0x08000000	/* max 128 MB */

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000E00

#define CFG_OR1_PRELIM	(0xF0000000 | CFG_OR_TIMING_SDRAM ) /* map 256 MB */
#define CFG_BR1_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/* RPXLITE mem setting */
#define CFG_BR3_PRELIM	0xFA400001		/* FPGA */
#define CFG_OR3_PRELIM	0xFFFF8910

#define CFG_BR4_PRELIM	0xFA000401		/* NVRAM&SRAM */
#define CFG_OR4_PRELIM	0xFFFE0970

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA	20

/*
 * Refresh clock Prescalar
 */
#define CFG_MPTPR	MPTPR_PTP_DIV2

/*
 * MAMR settings for SDRAM
 */

/* 9 column SDRAM */
#define CFG_MAMR_9COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_16X | MAMR_WLFA_16X | MAMR_TLFA_16X)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02	/* Software reboot			*/

/*
 * BCSRx
 *
 * Board Status and Control Registers
 *
 */

#define BCSR0 0xFA400000
#define BCSR1 0xFA400001
#define BCSR2 0xFA400002
#define BCSR3 0xFA400003

#define BCSR0_ENMONXCVR 0x01	/* Monitor XVCR Control */
#define BCSR0_ENNVRAM	0x02	/* CS4# Control */
#define BCSR0_LED5	0x04	/* LED5 control 0='on' 1='off' */
#define BCSR0_LED4	0x08	/* LED4 control 0='on' 1='off' */
#define BCSR0_FULLDPLX	0x10	/* Ethernet XCVR Control */
#define BCSR0_COLTEST	0x20
#define BCSR0_ETHLPBK	0x40
#define BCSR0_ETHEN	0x80

#define BCSR1_PCVCTL7	0x01	/* PC Slot B Control */
#define BCSR1_PCVCTL6	0x02
#define BCSR1_PCVCTL5	0x04
#define BCSR1_PCVCTL4	0x08
#define BCSR1_IPB5SEL	0x10

#define BCSR2_ENPA5HDR	0x08	/* USB Control */
#define BCSR2_ENUSBCLK	0x10
#define BCSR2_USBPWREN	0x20
#define BCSR2_USBSPD	0x40
#define BCSR2_USBSUSP	0x80

#define BCSR3_BWRTC	0x01	/* Real Time Clock Battery */
#define BCSR3_BWNVR	0x02	/* NVRAM Battery */
#define BCSR3_RDY_BSY	0x04	/* Flash Operation */
#define BCSR3_RPXL	0x08	/* Reserved (reads back '1') */
#define BCSR3_D27	0x10	/* Dip Switch settings */
#define BCSR3_D26	0x20
#define BCSR3_D25	0x40
#define BCSR3_D24	0x80

#endif	/* __CONFIG_H */
