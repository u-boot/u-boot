/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Pantelis Antoniou, Intracom S.A., panto@intracom.gr
 * U-Boot port on NetVia board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC850		1	/* This is a MPC850 CPU		*/
#define CONFIG_NETVIA		1	/* ...on a NetVia board		*/

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#if !defined(CONFIG_NETVIA_VERSION) || CONFIG_NETVIA_VERSION == 1
#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#else
#define CONFIG_8xx_CONS_NONE
#define CONFIG_MAX3100_SERIAL
#endif

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/

#define CONFIG_XIN		10000000
#define CONFIG_8xx_GCLK_FREQ	80000000

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#undef	CONFIG_CLOCKS_IN_MHZ	/* clocks NOT passsed to Linux in MHz */

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"tftpboot; "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_STATUS_LED	1	/* Status LED enabled		*/

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2
#define CONFIG_BOARD_SPECIFIC_LED	/* version has board specific leds */
#endif

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_NISDOMAIN


#undef CONFIG_MAC_PARTITION
#undef CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2
/* #define CONFIG_CMD_NAND */ /* disabled */
#endif


#define CONFIG_BOARD_EARLY_INIT_F 1
#define CONFIG_MISC_INIT_R

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0300000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0700000	/* 3 ... 7 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x3000	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x40000000
#if defined(DEBUG)
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

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
#define CONFIG_ENV_SECT_SIZE	0x10000

#define	CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x60000)
#define	CONFIG_ENV_SIZE		0x4000

#define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + 0x70000)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

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
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#ifndef	CONFIG_CAN_DRIVER
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01 | SIUMCR_FRC)
#else	/* we must activate GPL5 in the SIUMCR for CAN */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01 | SIUMCR_FRC)
#endif	/* CONFIG_CAN_DRIVER */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 *
 *
 *-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */

#define SCCR_MASK	SCCR_EBDF11

#if CONFIG_8xx_GCLK_FREQ == 50000000

#define CONFIG_SYS_PLPRCR	( ((5 - 1) << PLPRCR_MF_SHIFT) | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
#define CONFIG_SYS_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

#elif CONFIG_8xx_GCLK_FREQ == 80000000

#define CONFIG_SYS_PLPRCR	( ((8 - 1) << PLPRCR_MF_SHIFT) | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
#define CONFIG_SYS_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00 | SCCR_EBDF01)

#endif

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CONFIG_SYS_DER	0x2002000F*/
#define CONFIG_SYS_DER	0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/* FLASH timing: ACS = 11, TRLX = 0, CSNT = 1, SCY = 5, EHTR = 1	*/
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_BI | OR_SCY_5_CLK | OR_TRLX)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_8 | BR_V )

/*
 * BR3 and OR3 (SDRAM)
 *
 */
#define SDRAM_BASE3_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	(OR_CSNT_SAM | OR_G5LS)

#define CONFIG_SYS_OR3_PRELIM	((0xFFFFFFFFLU & ~(SDRAM_MAX_SIZE - 1)) | CONFIG_SYS_OR_TIMING_SDRAM)
#define CONFIG_SYS_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_PS_32 | BR_V)

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA	208

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CONFIG_SYS_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 9 column SDRAM */
#define CONFIG_SYS_MAMR_9COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)

/* Ethernet at SCC2 */
#define CONFIG_SCC2_ENET

/****************************************************************/

#define DSP_SIZE	0x00010000	/* 64K */
#define FPGA_SIZE	0x00010000	/* 64K */

#define DSP0_BASE	0xF1000000
#define DSP1_BASE	(DSP0_BASE + DSP_SIZE)
#define FPGA_BASE	(DSP1_BASE + DSP_SIZE)

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2

#define ER_SIZE		0x00010000	/* 64K */
#define ER_BASE		(FPGA_BASE + FPGA_SIZE)

#define NAND_SIZE	0x00010000	/* 64K */
#define NAND_BASE	(ER_BASE + ER_SIZE)

#endif

/****************************************************************/

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2

#define STATUS_LED_BIT		0x00000001		/* bit 31 */
#define STATUS_LED_PERIOD	(CONFIG_SYS_HZ / 2)
#define STATUS_LED_STATE	STATUS_LED_BLINKING

#define STATUS_LED_BIT1		0x00000002		/* bit 30 */
#define STATUS_LED_PERIOD1	(CONFIG_SYS_HZ / 2)
#define STATUS_LED_STATE1	STATUS_LED_OFF

#define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/
#define STATUS_LED_BOOT		0		/* LED 0 used for boot status */

#endif


/*****************************************************************************/

#ifndef __ASSEMBLY__

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2

/* LEDs */

/* last value written to the external register; we cannot read back */
extern unsigned int last_er_val;

/* led_id_t is unsigned long mask */
typedef unsigned int led_id_t;

static inline void __led_init(led_id_t mask, int state)
{
	unsigned int new_er_val;

	if (state)
		new_er_val = last_er_val & ~mask;
	else
		new_er_val = last_er_val |  mask;

	*(volatile unsigned int *)ER_BASE = new_er_val;
	last_er_val = new_er_val;
}

static inline void __led_toggle(led_id_t mask)
{
	unsigned int new_er_val;

	new_er_val = last_er_val ^ mask;
	*(volatile unsigned int *)ER_BASE = new_er_val;
	last_er_val = new_er_val;
}

static inline void __led_set(led_id_t mask, int state)
{
	unsigned int new_er_val;

	if (state)
		new_er_val = last_er_val & ~mask;
	else
		new_er_val = last_er_val |  mask;

	*(volatile unsigned int *)ER_BASE = new_er_val;
	last_er_val = new_er_val;
}

/* MAX3100 console */
#define MAX3100_SPI_RXD_PORT	(((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbdat)
#define MAX3100_SPI_RXD_BIT	0x00000008

#define MAX3100_SPI_TXD_PORT	(((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbdat)
#define MAX3100_SPI_TXD_BIT	0x00000004

#define MAX3100_SPI_CLK_PORT	(((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbdat)
#define MAX3100_SPI_CLK_BIT	0x00000002

#define MAX3100_CS_PORT		(((volatile immap_t *)CONFIG_SYS_IMMR)->im_ioport.iop_pddat)
#define MAX3100_CS_BIT		0x0010

#endif

#endif

/*************************************************************************************************/

#endif	/* __CONFIG_H */
