/*
 * (C) Copyright 2000
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

#define CONFIG_PREBOOT	"echo;echo Type \"run flash_nfs\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"tftpboot; " 								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " 	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " 	\
	"bootm"

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_STATUS_LED	1	/* Status LED enabled		*/

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2
#define CONFIG_BOARD_SPECIFIC_LED	/* version has board specific leds */
#endif

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

#define CONFIG_BOOTP_MASK		(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE | CONFIG_BOOTP_NISDOMAIN)

#undef CONFIG_MAC_PARTITION
#undef CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define CONFIG_COMMANDS_BASE  ( CONFIG_CMD_DFL	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_PING )

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2
#define CONFIG_COMMANDS		(CONFIG_COMMANDS_BASE | CFG_CMD_NAND)
#else
#define CONFIG_COMMANDS		CONFIG_COMMANDS_BASE
#endif

#define CONFIG_BOARD_EARLY_INIT_F 1
#define CONFIG_MISC_INIT_R

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0300000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0700000	/* 3 ... 7 MB in DRAM	*/

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
#define CFG_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x3000	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x40000000
#if defined(DEBUG)
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
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
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	8	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SECT_SIZE	0x10000

#define	CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x60000)
#define CFG_ENV_OFFSET		0
#define	CFG_ENV_SIZE		0x4000

#define CFG_ENV_ADDR_REDUND	(CFG_FLASH_BASE + 0x70000)
#define CFG_ENV_OFFSET_REDUND	0
#define CFG_ENV_SIZE_REDUND	CFG_ENV_SIZE

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#ifndef	CONFIG_CAN_DRIVER
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01 | SIUMCR_FRC)
#else	/* we must activate GPL5 in the SIUMCR for CAN */
#define CFG_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01 | SIUMCR_FRC)
#endif	/* CONFIG_CAN_DRIVER */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

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

#define CFG_PLPRCR	( ((5 - 1) << PLPRCR_MF_SHIFT) | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
#define CFG_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

#elif CONFIG_8xx_GCLK_FREQ == 80000000

#define CFG_PLPRCR	( ((8 - 1) << PLPRCR_MF_SHIFT) | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
#define CFG_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00 | SCCR_EBDF01)

#endif

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CFG_DER	0x2002000F*/
#define CFG_DER	0

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
#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/* FLASH timing: ACS = 11, TRLX = 0, CSNT = 1, SCY = 5, EHTR = 1	*/
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_BI | OR_SCY_5_CLK | OR_TRLX)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_8 | BR_V )

/*
 * BR3 and OR3 (SDRAM)
 *
 */
#define SDRAM_BASE3_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	(OR_CSNT_SAM | OR_G5LS)

#define CFG_OR3_PRELIM	((0xFFFFFFFFLU & ~(SDRAM_MAX_SIZE - 1)) | CFG_OR_TIMING_SDRAM)
#define CFG_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_PS_32 | BR_V)

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA	208

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CFG_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 9 column SDRAM */
#define CFG_MAMR_9COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

/* Ethernet at SCC2 */
#define CONFIG_SCC2_ENET

#define CONFIG_ARTOS			/* include ARTOS support */

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
#define STATUS_LED_PERIOD	(CFG_HZ / 2)
#define STATUS_LED_STATE	STATUS_LED_BLINKING

#define STATUS_LED_BIT1		0x00000002		/* bit 30 */
#define STATUS_LED_PERIOD1	(CFG_HZ / 2)
#define STATUS_LED_STATE1	STATUS_LED_OFF

#define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/
#define STATUS_LED_BOOT		0		/* LED 0 used for boot status */

#endif

/*****************************************************************************/

#if defined(CONFIG_NETVIA_VERSION) && CONFIG_NETVIA_VERSION >= 2

/* NAND */
#define CFG_NAND_BASE			NAND_BASE
#define CONFIG_MTD_NAND_ECC_JFFS2

#define CFG_MAX_NAND_DEVICE		1

#define SECTORSIZE		512
#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3
#define NAND_ChipID_UNKNOWN 	0x00
#define NAND_MAX_FLOORS		1
#define NAND_MAX_CHIPS		1

#define NAND_DISABLE_CE(nand) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) |=  0x0040; \
	} while(0)

#define NAND_ENABLE_CE(nand) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) &= ~0x0040; \
	} while(0)

#define NAND_CTL_CLRALE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) &= ~0x0100; \
	} while(0)

#define NAND_CTL_SETALE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) |=  0x0100; \
	} while(0)

#define NAND_CTL_CLRCLE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) &= ~0x0080; \
	} while(0)

#define NAND_CTL_SETCLE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) |=  0x0080; \
	} while(0)

#define NAND_WAIT_READY(nand) \
	do { \
		while ((((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat & 0x100) == 0) \
			; \
	} while (0)

#define WRITE_NAND_COMMAND(d, adr) \
	do { \
		*(volatile unsigned char *)((unsigned long)(adr)) = (unsigned char)(d); \
	} while(0)

#define WRITE_NAND_ADDRESS(d, adr) \
	do { \
		*(volatile unsigned char *)((unsigned long)(adr)) = (unsigned char)(d); \
	} while(0)

#define WRITE_NAND(d, adr) \
	do { \
		*(volatile unsigned char *)((unsigned long)(adr)) = (unsigned char)(d); \
	} while(0)

#define READ_NAND(adr) \
	((unsigned char)(*(volatile unsigned char *)(unsigned long)(adr)))

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
#define MAX3100_SPI_RXD_PORT	(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pbdat)
#define MAX3100_SPI_RXD_BIT	0x00000008

#define MAX3100_SPI_TXD_PORT	(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pbdat)
#define MAX3100_SPI_TXD_BIT	0x00000004

#define MAX3100_SPI_CLK_PORT	(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pbdat)
#define MAX3100_SPI_CLK_BIT	0x00000002

#define MAX3100_CS_PORT		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat)
#define MAX3100_CS_BIT		0x0010

#endif

#endif

/*************************************************************************************************/

#endif	/* __CONFIG_H */
