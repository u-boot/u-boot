/*
 * (C) Copyright 2003-2010
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

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

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
#define CONFIG_ROOTPATH "/opt/eldk/pcc_8xx"

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
#define CONFIG_SYS_ALT_MEMTEST

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/


/* M48T02 Paralled access timekeeper with same interface as the M48T35A*/
#define CONFIG_RTC_M48T35A 1

#if 0
#define CONFIG_WATCHDOG 1		/* watchdog enabled		*/
#else
#undef CONFIG_WATCHDOG
#endif

/*  NVRAM and RTC */
#define CONFIG_SYS_NVRAM_BASE_ADDR 0xFA000000
#define CONFIG_SYS_NVRAM_SIZE 2048


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


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define CONFIG_AUTOBOOT_KEYED	/* Enable password protection */
#define CONFIG_AUTOBOOT_PROMPT		\
	"\nEnter password - autoboot in %d sec...\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"system"
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00040000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x01f00000	/* 256K ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFA200000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE	0xFF000000

#if 1
    #define CONFIG_FLASH_CFI_DRIVER
#else
    #undef CONFIG_FLASH_CFI_DRIVER
#endif


#ifdef CONFIG_FLASH_CFI_DRIVER
    #define CONFIG_SYS_FLASH_CFI 1
    #undef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
    #define CONFIG_SYS_FLASH_BANKS_LIST {CONFIG_SYS_FLASH_BASE}
#endif

/*%%% #define CONFIG_SYS_FLASH_BASE		0xFFF00000 */
#if defined(DEBUG) || defined(CONFIG_CMD_IDE)
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define CONFIG_SYS_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	0xFFF00000
/*%%% #define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET	    0x00040000	/*   Offset   of Environment Sector	absolute address 0xfff40000*/
#define CONFIG_ENV_SECT_SIZE	0x40000	/* Total Size of Environment Sector	*/
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET+CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/* FPGA */
#define CONFIG_MISC_INIT_R
#define CONFIG_SYS_FPGA_SPARTAN2
#define CONFIG_SYS_FPGA_PROG_FEEDBACK


/*-----------------------------------------------------------------------
 * Reset address
 */
#define CONFIG_SYS_RESET_ADDRESS	((ulong)((((immap_t *)CONFIG_SYS_IMMR)->im_clkrst.res)))

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | 0x00000600 | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_MLRC10)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
/*%%%#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE) */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR (PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 *
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
/* up to 50 MHz we use a 1:1 clock */
#define CONFIG_SYS_PLPRCR	( (5 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS )

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
/* up to 50 MHz we use a 1:1 clock */
#define CONFIG_SYS_SCCR	(SCCR_COM00 | SCCR_TBS)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(2 * CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0100

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CONFIG_SYS_DER 0x2002000F*/
#define CONFIG_SYS_DER 0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE_PRELIM	0xFE000000	/* FLASH base */
#define CONFIG_SYS_PRELIM_OR_AM	0xFE000000	/* OR addr mask */

/* FLASH timing: ACS = 0, TRLX = 0, CSNT = 0, SCY = 4, ETHR = 0, BIH = 1 */
#define CONFIG_SYS_OR_TIMING_FLASH (OR_SCY_4_CLK | OR_BI)

#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE_PRELIM & BR_BA_MSK) | BR_V)

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM base	*/
#define SDRAM_MAX_SIZE		0x08000000	/* max 128 MB */

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000E00

#define CONFIG_SYS_OR1_PRELIM	(0xF0000000 | CONFIG_SYS_OR_TIMING_SDRAM ) /* map 256 MB */
#define CONFIG_SYS_BR1_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/* RPXLITE mem setting */
#define CONFIG_SYS_BR3_PRELIM	0xFA400001		/* FPGA */
#define CONFIG_SYS_OR3_PRELIM	0xFFFF8910

#define CONFIG_SYS_BR4_PRELIM	0xFA000401		/* NVRAM&SRAM */
#define CONFIG_SYS_OR4_PRELIM	0xFFFE0970

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA	20

/*
 * Refresh clock Prescalar
 */
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV2

/*
 * MAMR settings for SDRAM
 */

/* 9 column SDRAM */
#define CONFIG_SYS_MAMR_9COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_16X | MAMR_WLFA_16X | MAMR_TLFA_16X)

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
