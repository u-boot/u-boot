/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Sam Song, IEMC. SHU, samsongshu@yahoo.com.cn
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

/* Yoo. Jonghoon, IPone, yooth@ipone.co.kr
 * U-BOOT port on RPXlite board
 */

/*
 * Sam Song, IEMC. SHU, samsongshu@yahoo.com.cn
 * U-BOOT port on RPXlite DW version board--RPXlite_DW
 * June 8 ,2004
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

/* #define DEBUG	1 */
/* #ifdef DEPLOYMENT	1 */

#undef	CONFIG_MPC860
#define CONFIG_MPC823		1	/* This is a MPC823e CPU. */
#define CONFIG_RPXLITE		1	/* RPXlite DW version board */

#ifdef	CONFIG_LCD			/* with LCD controller ?	*/
#define CONFIG_SPLASH_SCREEN		/* ... with splashscreen support*/
#endif

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600	/* console default baudrate = 9600bps	*/

#ifdef DEBUG
#define CONFIG_BOOTDELAY        -1      /* autoboot disabled            */
#else
#define CONFIG_BOOTDELAY        6       /* autoboot after 6 seconds     */

#ifdef DEPLOYMENT
#define CONFIG_BOOT_RETRY_TIME          -1
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT          "autoboot in %d seconds (stop with 'st')...\n"
#define CONFIG_AUTOBOOT_STOP_STR        "st"
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_RESET_TO_RETRY           1
#define CONFIG_BOOT_RETRY_MIN           1
#endif	/* DEPLOYMENT */
#endif	/* DEBUG */

/* pre-boot commands */
#define CONFIG_PREBOOT          "setenv stdout serial;setenv stdin serial"

#undef	CONFIG_BOOTARGS
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs console=tty0 console=ttyS0,9600 "	\
		"root=/dev/nfs rw nfsroot=${serverip}:${rootpath}\0"	\
	"ramargs=setenv bootargs console=tty0 root=/dev/ram rw\0"	\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"gatewayip=172.16.115.254\0"					\
	"netmask=255.255.255.0\0"					\
	"kernel_addr=ff040000\0"					\
	"ramdisk_addr=ff200000\0"					\
	"ku=era ${kernel_addr} ff1fffff;cp.b 100000 ${kernel_addr} "	\
		"${filesize};md ${kernel_addr};"			\
		"echo kernel updating finished\0"			\
	"uu=protect off 1:0-4;era 1:0-4;cp.b 100000 ff000000 "		\
		"${filesize};md ff000000;"				\
		"echo u-boot updating finished\0"			\
	"eu=protect off 1:6;era 1:6;reset\0"				\
	"lcd=setenv stdout lcd;setenv stdin lcd\0"			\
	"ser=setenv stdout serial;setenv stdin serial\0"		\
	"verify=no"

#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/
#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#undef	CONFIG_STATUS_LED		/* disturbs display. Status LED disabled. */

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"u-boot>"	/* Monitor Command Prompt   */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0040000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00C0000	/* 4 ... 12 MB in DRAM	*/
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
#define CFG_INIT_RAM_END	0x2F00		/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	64		/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFF000000

#if defined(DEBUG) || (CONFIG_COMMANDS & CFG_CMD_IDE)
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define CFG_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor */
#endif

#define CFG_MONITOR_BASE	0xFF000000
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
#define CFG_MAX_FLASH_SECT	71	/* max number of sectors on one chip	*/
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#ifdef	CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_ADDR		0xFA000100
#define CFG_ENV_SIZE		0x1000
#else
#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_OFFSET		0x30000 /* Offset of Environment Sector		*/
#define CFG_ENV_SIZE		0x8000	/* Total Size of Environment Sector	*/
#endif	/* CFG_ENV_IS_IN_NVRAM */

#define CFG_RESET_ADDRESS	((ulong)((((immap_t *)CFG_IMMR)->im_clkrst.res)))

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control	32-bit			12-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | 0x00000600 | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif	/* We can get SYPCR: 0xFFFF0689. */

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration	32-bit			 12-30
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_MLRC10)	       /* SIUMCR:0x00000800 */

/*---------------------------------------------------------------------
 * TBSCR - Time Base Status and Control	 16-bit			 12-16
 *---------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF | TBSCR_TBE)
/* TBSCR: 0x00C3 [SAM] */

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register 16-bit	 12-18
 *-----------------------------------------------------------------------
 * [RTC enabled but not stopped on FRZ]
 */
#define CFG_RTCSC    (RTCSC_SEC | RTCSC_ALR | RTCSC_RTE) /* RTCSC:0x00C1	*/

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control 16-bit		 12-23
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 * [Periodic timer enabled,Periodic timer interrupt disable. ]
 */
#define CFG_PISCR (PISCR_PS | PISCR_PITF | PISCR_PTE)  /* PISCR:0x0083		*/

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	32-bit	 5-7
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 */
/* up to 64 MHz we use a 1:2 clock */
#if defined(RPXlite_64MHz)
#define CFG_PLPRCR	( (7 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS )   /*PLPRCR: 0x00700000. */
#else
#define CFG_PLPRCR	( (5 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS )
#endif

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		5-3
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
/* Up to 48MHz system clock, we use 1:1 SYSTEM/BUS ratio */
#if defined(RPXlite_64MHz)
#define CFG_SCCR	( SCCR_TBS | SCCR_EBDF01 )  /* %%%SCCR:0x02020000 */
#else
#define CFG_SCCR        ( SCCR_TBS | SCCR_EBDF00 )  /* %%%SCCR:0x02000000 */
#endif

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
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

#define		CFG_DER		0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */
#define FLASH_BASE_PRELIM	0xFC000000	/* FLASH base	*/
#define CFG_PRELIM_OR_AM	0xFC000000	/* OR addr mask */

/* FLASH timing: ACS = 0, TRLX = 0, CSNT = 0, SCY = 8, ETHR = 0, BIH = 1 */
#define CFG_OR_TIMING_FLASH (OR_SCY_8_CLK | OR_BI)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE_PRELIM & BR_BA_MSK) | BR_V)

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM base	*/
#define SDRAM_MAX_SIZE		0x08000000	/* max 128 MB in system */

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000E00
#define CFG_OR_AM_SDRAM		(-(SDRAM_MAX_SIZE & OR_AM_MSK))
#define CFG_OR1_PRELIM	( CFG_OR_AM_SDRAM | CFG_OR_TIMING_SDRAM )
#define CFG_BR1_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/* RPXlite mem setting */
#define CFG_BR3_PRELIM	0xFA400001		/* BCSR */
#define CFG_OR3_PRELIM	0xFF7F8900
#define CFG_BR4_PRELIM	0xFA000401		/* NVRAM&SRAM */
#define CFG_OR4_PRELIM	0xFFFE0040

/*
 * Memory Periodic Timer Prescaler
 */
/* periodic timer for refresh */
#if defined(RPXlite_64MHz)
#define CFG_MAMR_PTA	32
#else
#define CFG_MAMR_PTA	20
#endif

/*
 * Refresh clock Prescalar
 */
#define CFG_MPTPR	MPTPR_PTP_DIV2

/*
 * MAMR settings for SDRAM
 */

/* 9 column SDRAM */
#define CFG_MAMR_9COL  ((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE | \
			MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10)
/* CFG_MAMR_9COL:0x20904000 @ 64MHz */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* Configuration variable added by yooth. */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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

#define BCSR1_SMC1CTS	0x40	/* Added by SAM. */
#define BCSR1_SMC1TRS	0x80	/* Added by SAM. */

#define BCSR2_ENRTCIRQ	0x01	/* Added by SAM. */
#define BCSR2_ENBRG1	0x04	/* Added by SAM. */

#define BCSR2_ENPA5HDR	0x08	/* USB Control */
#define BCSR2_ENUSBCLK	0x10
#define BCSR2_USBPWREN	0x20
#define BCSR2_USBSPD	0x40
#define BCSR2_USBSUSP	0x80

#define BCSR3_BWKAPWR	0x01   /* Changed by SAM. Backup battery situation */
#define BCSR3_IRQRTC	0x02   /* Changed by SAM. NVRAM Battery */
#define BCSR3_RDY_BSY	0x04   /* Changed by SAM. Flash Operation */
#define BCSR3_MPLX_LIN	0x08   /* Changed by SAM. Linear or Multiplexed address Mode */

#define BCSR3_D27	0x10	  /* Dip Switch settings */
#define BCSR3_D26	0x20
#define BCSR3_D25	0x40
#define BCSR3_D24	0x80

/*
 * Environment setting
 */
#define CONFIG_ETHADDR	00:10:EC:00:37:5B
#define CONFIG_IPADDR	172.16.115.7
#define CONFIG_SERVERIP 172.16.115.6
#define CONFIG_ROOTPATH /workspace/myfilesystem/target/
#define CONFIG_BOOTFILE uImage.rpxusb

#endif	/* __CONFIG_H */
