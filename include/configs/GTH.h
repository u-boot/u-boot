/*
  * Parameters for GTH board
  * Based on FADS860T
  * by thomas.lange@corelatus.com

  * A collection of structures, addresses, and values associated with
  * the Motorola 860T FADS board.  Copied from the MBX stuff.
  * Magnus Damm added defines for 8xxrom and extended bd_info.
  * Helmut Buchsbaum added bitvalues for BCSRx
  *
  * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
  */

/*
 * ff000000 -> ff00ffff : IMAP       internal in the cpu
 * e0000000 -> ennnnnnn : pcmcia
 * 98000000 -> 983nnnnn : FPGA 4MB
 * 90000000 -> 903nnnnn : FPGA 4MB
 * 80000000 -> 80nnnnnn : flash      connected to CS0, final ( real ) location
 * 00000000 -> nnnnnnnn : sdram
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#include <mpc8xx_irq.h>

#define CONFIG_MPC860		1
#define CONFIG_MPC860T		1
#define CONFIG_GTH		1

#define CONFIG_MISC_INIT_R      1

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

#define MPC8XX_FACT		3		/* Multiply by 3 */
#define MPC8XX_XIN		16384000	/* 16.384 MHz */
#define MPC8XX_HZ ((MPC8XX_XIN) * (MPC8XX_FACT))

#define CONFIG_BOOTDELAY	1	/* autoboot after 0 seconds	*/

#define CONFIG_ENV_OVERWRITE 1 /* Allow change of ethernet address */

#define CONFIG_BOOT_RETRY_TIME 5 /* Retry boot in 5 secs */

#define CONFIG_RESET_TO_RETRY 1 /* If timeout waiting for command, perform a reset */

/* Only interrupt boot if space is pressed */
/* If a long serial cable is connected but */
/* other end is dead, garbage will be read */
#define CONFIG_AUTOBOOT_KEYED	1
#define CONFIG_AUTOBOOT_PROMPT	\
	"Press space to abort autoboot in %d second\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR "d"
#define CONFIG_AUTOBOOT_STOP_STR " "

#if 0
/* Net boot */
/* Loads a tftp image and starts it */
#define CONFIG_BOOTCOMMAND	"bootp;bootm 100000"	/* autoboot command */
#define CONFIG_BOOTARGS		"panic=1"
#else
/* Compact flash boot */
#define CONFIG_BOOTARGS "panic=1 root=/dev/hda7"
#define CONFIG_BOOTCOMMAND "disk 100000 0:5;bootm 100000"
#endif

/* Enable watchdog */
#define CONFIG_WATCHDOG 1

/* choose SCC1 ethernet (10BASET on motherboard)
 * or FEC ethernet (10/100 on daughterboard)
 */
#if 1
#define	CONFIG_SCC1_ENET	1	/* use SCC1 ethernet */
#undef	CONFIG_FEC_ENET			/* disable FEC ethernet  */
#define CONFIG_SYS_DISCOVER_PHY
#else
#undef	CONFIG_SCC1_ENET		/* disable SCC1 ethernet */
#define	CONFIG_FEC_ENET		1	/* use FEC ethernet  */
#define CONFIG_SYS_DISCOVER_PHY
#endif
#if defined(CONFIG_SCC1_ENET) && defined(CONFIG_FEC_ENET)
#error Both CONFIG_SCC1_ENET and CONFIG_FEC_ENET configured
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

#define CONFIG_CMD_IDE


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_PROMPT		"=>"	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0400000	/* 1 ... 4 MB in DRAM	*/

/* Default location to load data from net */
#define CONFIG_SYS_LOAD_ADDR		0x100000

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR			0xFF000000
#define CONFIG_SYS_IMMR_SIZE		((uint)(64 * 1024))

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define	CONFIG_SYS_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000

#define CONFIG_SYS_FLASH_BASE		0x80000000

#define CONFIG_SYS_FLASH_SIZE		((uint)(8 * 1024 * 1024))	/* max 8Mbyte */

#define	CONFIG_SYS_MONITOR_LEN		(384 << 10)	/* Reserve 384 kB for Monitor	*/

#define CONFIG_SYS_MONITOR_BASE TEXT_BASE

#define	CONFIG_SYS_MALLOC_LEN		(384 << 10)	/* Reserve 384 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	8	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH 1
#undef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0x000E0000
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

#define CONFIG_ENV_SECT_SIZE	0x50000	/* see README - env sector total size	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control					11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration					11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */

/*FIXME dont use for now */
/*#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE) */
/*#define CONFIG_SYS_RTCSC	(RTCSC_RTF) */

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)
/* PITE */
/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CONFIG_SYS_PLPRCR	(((MPC8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
				PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */

/* FIXME check values */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(SCCR_TBS|SCCR_RTSEL|SCCR_COM00|SCCR_DFSYNC00|SCCR_DFBRG00|SCCR_DFNL000|SCCR_DFNH000|SCCR_DFLCD000|SCCR_DFALCD00)

 /*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_DER		0

/* Because of the way the 860 starts up and assigns CS0 the
* entire address space, we have to set the memory controller
* differently.  Normally, you write the option register
* first, and then enable the chip select by writing the
* base register.  For CS0, you must write the base register
* first, followed by the option register.
*/

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */
/* the other CS:s are determined by looking at parameters in BCSRx */

#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE	/* FLASH bank #0	*/

#define CONFIG_SYS_REMAP_OR_AM		0xFF800000	/* 4 MB OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xFF800000	/* OR addr mask */

#define FPGA_2_BASE 0x90000000
#define FPGA_3_BASE 0x98000000

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0	*/
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK | OR_TRLX)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)


#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)   /* 1 Mbyte until detected and only 1 Mbyte is needed*/
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V | BR_PS_16 )

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#define CONFIG_ETHADDR          DE:AD:BE:EF:00:01    /* Ethernet address */

#ifdef CONFIG_MPC860T

/* Interrupt level assignments.
*/
#define FEC_INTERRUPT	SIU_LEVEL1	/* FEC interrupt */

#endif /* CONFIG_MPC860T */

/* We don't use the 8259.
*/
#define NR_8259_INTS	0

/* Machine type
*/
#define _MACH_8xx (_MACH_gth)

#ifdef CONFIG_MPC860
#define PCMCIA_SLOT_A 1
#define CONFIG_PCMCIA_SLOT_A 1
#endif

#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

#define CONFIG_IDE_8xx_PCCARD       1       /* Use IDE with PC Card Adapter */
#undef  CONFIG_IDE_8xx_DIRECT               /* Direct IDE    not supported  */
#undef  CONFIG_IDE_LED                  /* LED   for ide not supported  */
#undef  CONFIG_IDE_RESET                /* reset for ide not supported  */

#define CONFIG_SYS_IDE_MAXBUS          1       /* max. 1 IDE bus               */
#define CONFIG_SYS_IDE_MAXDEVICE       1       /* max. 1 drive per IDE bus     */

#define CONFIG_SYS_ATA_IDE0_OFFSET     0x0000
#define CONFIG_SYS_ATA_BASE_ADDR       CONFIG_SYS_PCMCIA_MEM_ADDR
/* Offset for data I/O                  */
#define CONFIG_SYS_ATA_DATA_OFFSET     (CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)
/* Offset for normal register accesses  */
#define CONFIG_SYS_ATA_REG_OFFSET      (2 * CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)
/* Offset for alternate registers       */
#define CONFIG_SYS_ATA_ALT_OFFSET      0x0100

#define CONFIG_8xx_GCLK_FREQ   MPC8XX_HZ       /* Force it - dont measure it */

#define PA_FRONT_LED ((u16)0x4) /* PA 13 */
#define PA_FL_CONFIG ((u16)0x20) /* PA 10 */
#define PA_FL_CE     ((u16)0x1000) /* PA 3 */

#define PB_ID_GND    ((u32)1) /* PB 31 */
#define PB_REV_1     ((u32)2) /* PB 30 */
#define PB_REV_0     ((u32)4) /* PB 29 */
#define PB_BLUE_LED  ((u32)0x400) /* PB 21 */
#define PB_EEPROM    ((u32)0x800) /* PB 20 */
#define PB_ID_3      ((u32)0x2000) /* PB 18 */
#define PB_ID_2      ((u32)0x4000) /* PB 17 */
#define PB_ID_1      ((u32)0x8000) /* PB 16 */
#define PB_ID_0      ((u32)0x10000) /* PB 15 */

/* NOTE. This is reset for 100Mbit port only */
#define PC_ENET100_RESET	((ushort)0x0080) /* PC 8 */

#endif	/* __CONFIG_H */
