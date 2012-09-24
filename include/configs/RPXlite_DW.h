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
/* #define DEPLOYMENT	1 */

#undef	CONFIG_MPC860
#define CONFIG_MPC823		1	/* This is a MPC823e CPU. */
#define CONFIG_RPXLITE		1	/* RPXlite DW version board */

#define	CONFIG_SYS_TEXT_BASE	0xff000000

#ifdef	CONFIG_LCD			/* with LCD controller ?	*/
#define CONFIG_SPLASH_SCREEN		/* ... with splashscreen support*/
#endif

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600	/* console default baudrate = 9600bps	*/

#ifdef DEBUG
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	6	/* autoboot after 6 seconds	*/

#ifdef DEPLOYMENT
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		\
	"autoboot in %d seconds (stop with 'st')...\n", bootdelay
#define CONFIG_AUTOBOOT_STOP_STR	"st"
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_RESET_TO_RETRY		1
#define CONFIG_BOOT_RETRY_MIN		1
#endif	/* DEPLOYMENT */
#endif	/* DEBUG */

/* pre-boot commands */
#define CONFIG_PREBOOT		"setenv stdout serial;setenv stdin serial"

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
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/
#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#undef	CONFIG_STATUS_LED		/* disturbs display. Status LED disabled. */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#if 1	       /* Enable this stuff could make image enlarge about 25KB. Mask it if you
		  don't want the advanced function */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_DHCP

#ifdef	CONFIG_SPLASH_SCREEN
#define CONFIG_CMD_BMP
#endif


/* test-only */
#define CONFIG_SYS_JFFS2_FIRST_BANK	0	    /* use for JFFS2 */
#define CONFIG_SYS_JFFS2_NUM_BANKS	1	    /* ! second bank contains U-Boot */

#define CONFIG_NETCONSOLE

#endif	/* 1 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"u-boot>"	/* Monitor Command Prompt   */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif

#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0040000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00C0000	/* 4 ... 12 MB in DRAM	*/
#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

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
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00		/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFF000000

#if defined(DEBUG) || defined(CONFIG_CMD_IDE)
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define CONFIG_SYS_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor */
#endif

#define CONFIG_SYS_MONITOR_BASE	0xFF000000
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
#define CONFIG_SYS_MAX_FLASH_SECT	71	/* max number of sectors on one chip	*/
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#ifdef	CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_ADDR		0xFA000100
#define CONFIG_ENV_SIZE		0x1000
#else
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET		0x30000 /* Offset of Environment Sector		*/
#define CONFIG_ENV_SIZE		0x8000	/* Total Size of Environment Sector	*/
#endif	/* CONFIG_ENV_IS_IN_NVRAM */

#define CONFIG_SYS_RESET_ADDRESS	((ulong)((((immap_t *)CONFIG_SYS_IMMR)->im_clkrst.res)))

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control	32-bit			12-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | 0x00000600 | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif	/* We can get SYPCR: 0xFFFF0689. */

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration	32-bit			 12-30
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_MLRC10)	       /* SIUMCR:0x00000800 */

/*---------------------------------------------------------------------
 * TBSCR - Time Base Status and Control	 16-bit			 12-16
 *---------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF | TBSCR_TBE)
/* TBSCR: 0x00C3 [SAM] */

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register 16-bit	 12-18
 *-----------------------------------------------------------------------
 * [RTC enabled but not stopped on FRZ]
 */
#define CONFIG_SYS_RTCSC    (RTCSC_SEC | RTCSC_ALR | RTCSC_RTE) /* RTCSC:0x00C1	*/

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control 16-bit		 12-23
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 * [Periodic timer enabled,Periodic timer interrupt disable. ]
 */
#define CONFIG_SYS_PISCR (PISCR_PS | PISCR_PITF | PISCR_PTE)  /* PISCR:0x0083		*/

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	32-bit	 5-7
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 */
/* up to 64 MHz we use a 1:2 clock */
#if defined(RPXlite_64MHz)
#define CONFIG_SYS_PLPRCR	( (7 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS )   /*PLPRCR: 0x00700000. */
#else
#define CONFIG_SYS_PLPRCR	( (5 << PLPRCR_MF_SHIFT) | PLPRCR_TEXPS )
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
#define CONFIG_SYS_SCCR	( SCCR_TBS | SCCR_EBDF01 )  /* %%%SCCR:0x02020000 */
#else
#define CONFIG_SYS_SCCR	( SCCR_TBS | SCCR_EBDF00 )  /* %%%SCCR:0x02000000 */
#endif

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
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

#define		CONFIG_SYS_DER		0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */
#define FLASH_BASE_PRELIM	0xFC000000	/* FLASH base	*/
#define CONFIG_SYS_PRELIM_OR_AM	0xFC000000	/* OR addr mask */

/* FLASH timing: ACS = 0, TRLX = 0, CSNT = 0, SCY = 8, ETHR = 0, BIH = 1 */
#define CONFIG_SYS_OR_TIMING_FLASH (OR_SCY_8_CLK | OR_BI)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE_PRELIM & BR_BA_MSK) | BR_V)

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM base	*/
#define SDRAM_MAX_SIZE		0x08000000	/* max 128 MB in system */

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000E00
#define CONFIG_SYS_OR_AM_SDRAM		(-(SDRAM_MAX_SIZE & OR_AM_MSK))
#define CONFIG_SYS_OR1_PRELIM	( CONFIG_SYS_OR_AM_SDRAM | CONFIG_SYS_OR_TIMING_SDRAM )
#define CONFIG_SYS_BR1_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/* RPXlite mem setting */
#define CONFIG_SYS_BR3_PRELIM	0xFA400001		/* BCSR */
#define CONFIG_SYS_OR3_PRELIM	0xFF7F8900
#define CONFIG_SYS_BR4_PRELIM	0xFA000401		/* NVRAM&SRAM */
#define CONFIG_SYS_OR4_PRELIM	0xFFFE0040

/*
 * Memory Periodic Timer Prescaler
 */
/* periodic timer for refresh */
#if defined(RPXlite_64MHz)
#define CONFIG_SYS_MAMR_PTA	32
#else
#define CONFIG_SYS_MAMR_PTA	20
#endif

/*
 * Refresh clock Prescalar
 */
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV2

/*
 * MAMR settings for SDRAM
 */

/* 9 column SDRAM */
#define CONFIG_SYS_MAMR_9COL  ((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE | \
			MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10)
/* CONFIG_SYS_MAMR_9COL:0x20904000 @ 64MHz */

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
#define CONFIG_ROOTPATH "/workspace/myfilesystem/target/"
#define CONFIG_BOOTFILE "uImage.rpxusb"
#define CONFIG_HOSTNAME LITE_H1_DW

#endif	/* __CONFIG_H */
