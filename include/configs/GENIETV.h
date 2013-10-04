 /*
  * A collection of structures, addresses, and values associated with
  * the Motorola 860T FADS board.  Copied from the MBX stuff.
  * Magnus Damm added defines for 8xxrom and extended bd_info.
  * Helmut Buchsbaum added bitvalues for BCSRx
  *
  * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
  */

/*
 * The GENIETV is using the following physical memorymap (copied from
 * the FADS configuration):
 *
 * ff020000 -> ff02ffff : pcmcia
 * ff010000 -> ff01ffff : BCSR       connected to CS1, setup by 8xxROM
 * ff000000 -> ff00ffff : IMAP       internal in the cpu
 * 30000000 -> 300fffff : flash      connected to CS0
 * 00000000 -> nnnnnnnn : sdram      setup by U-Boot
 *
 * CS pins are connected as follows:
 *
 * CS0 -512Kb boot flash
 * CS1 - SDRAM #1
 * CS2 - SDRAM #2
 * CS3 - Flash #1
 * CS4 - Flash #2
 * CS5 - Lon (if present)
 * CS6 - PCMCIA #1
 * CS7 - PCMCIA #2
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define	CONFIG_SYS_TEXT_BASE	0x00000000

#define	CONFIG_ETHADDR		08:00:22:50:70:63	/* Ethernet address */
#define CONFIG_ENV_OVERWRITE	1	/* Overwrite the environment */

#define CONFIG_SYS_ALLOC_DPRAM			/* Use dynamic DPRAM allocation */

#define CONFIG_SYS_AUTOLOAD		"n"	/* No autoload */

/*#define CONFIG_VIDEO		1	/  To enable the video initialization */
/*#define CONFIG_VIDEO_ADDR	0x00200000 */
/*#define CONFIG_HARD_I2C	1	/  I2C with hardware support */
/*#define CONFIG_PCMCIA		1	/  To enable the PCMCIA initialization */

/*#define CONFIG_SYS_PCMCIA_IO_ADDR	0xff020000 */
/*#define CONFIG_SYS_PCMCIA_IO_SIZE	0x10000 */
/*#define CONFIG_SYS_PCMCIA_MEM_ADDR	0xe0000000 */
/*#define CONFIG_SYS_PCMCIA_MEM_SIZE	0x10000 */

/* Video related */

/*#define CONFIG_VIDEO_LOGO			1	/  Show the logo */
/*#define CONFIG_VIDEO_ENCODER_AD7177		1	/  Enable this encoder */
/*#define CONFIG_VIDEO_ENCODER_AD7177_ADDR	0xF4	/  ALSB to ground */

/* Wireless 56Khz 4PPM keyboard on SMCx */

/*#define CONFIG_KEYBOARD		0 */
/*#define CONFIG_WL_4PPM_KEYBOARD_SMC	0	/  SMC to use (0 indexed) */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#include <mpc8xx_irq.h>

#define CONFIG_GENIETV		1
#define CONFIG_MPC823		1

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600

#define MPC8XX_FACT	12			/* Multiply by 12	*/
#define MPC8XX_XIN	5000000			/* 4 MHz clock		*/

#define MPC8XX_HZ	((MPC8XX_XIN) * (MPC8XX_FACT))
#define CONFIG_SYS_PLPRCR_MF	((MPC8XX_FACT-1) << 20)
#define CONFIG_8xx_GCLK_FREQ	MPC8XX_HZ	/* Force it - dont measure it */

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#if 1
#define CONFIG_BOOTDELAY	1	/* autoboot after 2 seconds	*/
#define CONFIG_LOADS_ECHO	0	/* Dont echoes received characters */
#define CONFIG_BOOTARGS		""
#define CONFIG_BOOTCOMMAND							\
"bootp; tftp; "									\
"setenv bootargs console=tty0 console=ttyS0 "					\
"root=/dev/nfs nfsroot=${serverip}:${rootpath} "				\
"ip=${ipaddr}:${serverip}:${gatewayip}:${subnetmask}:${hostname}:eth0:off ;"	\
"bootm "
#else
#define CONFIG_BOOTDELAY	0	/* autoboot disabled		*/
#endif

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/


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


/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		":>"		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		8		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00004000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00800000	/* 0 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00100000	/* default load address */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFF000000
#define CONFIG_SYS_IMMR_SIZE		((uint)(64 * 1024))

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 * Also NOTE that it doesn't mean SDRAM - it means MEMORY.
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x02800000
#define CONFIG_SYS_FLASH_SIZE		((uint)(8 * 1024 * 1024))	/* max 8Mbyte */
#if 0
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 128 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* Reserve 512 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 128 kB for malloc()	*/

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
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x10000	/* Offset of Environment Sector		*/
#define	CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector (64k)*/

/* values according to the manual */

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
 * SIUMCR - SIU Module Configuration						11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 *
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC10)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer  *
 * interrupt status bit - leave PLL multiplication factor unchanged !
 *
 * #define CONFIG_SYS_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
 */
#define CONFIG_SYS_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST | CONFIG_SYS_PLPRCR_MF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR       (SCCR_TBS     | \
				SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
				SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
				SCCR_DFALCD00)

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
 * BR0 and OR0(FLASH)
 */

#define FLASH_BASE0_PRELIM	0x02800000	/* FLASH bank #0		*/

#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask		*/
#define CONFIG_SYS_PRELIM_OR_AM	0xFF800000	/* OR addr mask (512Kb) */

/* FLASH timing */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV2 | OR_BI | \
				OR_SCY_15_CLK | OR_TRLX )

/*#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH) */
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)		/* 0xfff80ff4 */
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V | BR_PS_8)	/* 0x02800401 */

/*
 * BR1/2 and OR1/2 (SDRAM)
*/

#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000A00

#define SDRAM_MAX_SIZE		0x04000000	/* 64Mb bank */
#define SDRAM_BASE1_PRELIM	0x00000000	/* First bank */
#define SDRAM_BASE2_PRELIM	0x10000000	/* Second bank */

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MBMR_PTB		0x5d		/* start with divider for 100 MHz	*/

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CONFIG_SYS_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_4K        MPTPR_PTP_DIV32
/*
 * MBMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CONFIG_SYS_MBMR_8COL	((CONFIG_SYS_MBMR_PTB << MAMR_PTA_SHIFT)  | MAMR_PTAE | \
			MAMR_G0CLA_A11 | MAMR_RLFA_1X | MAMR_WLFA_1X \
			| MAMR_TLFA_4X)	/* 0x5d802114 */

/* values according to the manual */

#define CONFIG_DRAM_50MHZ		1
#define CONFIG_SDRAM_50MHZ

/* We don't use the 8259.
*/
#define NR_8259_INTS	0

/*
 * MPC8xx CPM Options
 */
#define CONFIG_SCC_ENET 1

#define CONFIG_DISK_SPINUP_TIME 1000000

/* PCMCIA configuration */

#define PCMCIA_MAX_SLOTS    1
#define PCMCIA_SLOT_B 1

#endif	/* __CONFIG_H */
