/*
 * (C) Copyright 2005
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
#define CONFIG_MPC852T		1
#define CONFIG_NC650		1

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

/*
 * 10 MHz - PLL input clock
 */
#define CONFIG_8xx_OSCLK		10000000

/*
 * 50 MHz - default CPU clock
 */
#define CONFIG_8xx_CPUCLK_DEFAULT	50000000

/*
 * 15 MHz - CPU minimum clock
 */
#define CFG_8xx_CPUCLK_MIN		15000000

/*
 * 133 MHz - CPU maximum clock
 */
#define CFG_8xx_CPUCLK_MAX		133000000

#define CFG_MEASURE_CPUCLK
#define CFG_8XX_XIN			CONFIG_8xx_OSCLK

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_PREBOOT	"echo;echo Type \"run flash_nfs\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp;" 								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " 	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" 	\
	"bootm"

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#undef	CONFIG_STATUS_LED		/* Status LED disabled		*/

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define	CONFIG_FEC_ENET		1	/* use FEC ethernet  */
#define FEC_ENET
#define CONFIG_MII
#define CFG_DISCOVER_PHY	1


/* enable I2C and select the hardware/software driver */
#undef  CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
#define CFG_I2C_SPEED		100000	/* 100 kHz			*/
#define CFG_I2C_SLAVE		0x7f

/*
 * Software (bit-bang) I2C driver configuration
 */
#define SCL		0x1000		/* PA 3 */
#define SDA		0x2000		/* PA 2 */

#define __I2C_DIR	immr->im_ioport.iop_padir
#define __I2C_DAT	immr->im_ioport.iop_padat
#define __I2C_PAR	immr->im_ioport.iop_papar
#define	I2C_INIT	{ __I2C_PAR &= ~(SDA|SCL);	\
			  __I2C_DIR |= (SDA|SCL);	}
#define	I2C_READ	((__I2C_DAT & SDA) ? 1 : 0)
#define	I2C_SDA(x)	{ if (x) __I2C_DAT |= SDA; else __I2C_DAT &= ~SDA; }
#define	I2C_SCL(x)	{ if (x) __I2C_DAT |= SCL; else __I2C_DAT &= ~SCL; }
#define	I2C_DELAY	{ udelay(5); }
#define	I2C_ACTIVE	{ __I2C_DIR |= SDA; }
#define	I2C_TRISTATE	{ __I2C_DIR &= ~SDA; }

#define CONFIG_RTC_PCF8563
#define CFG_I2C_RTC_ADDR		0x51

#define CONFIG_COMMANDS	      ( CONFIG_CMD_DFL	| \
				CFG_CMD_ASKENV	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_I2C	| \
				CFG_CMD_NAND	| \
				CFG_CMD_JFFS2	| \
				CFG_CMD_NFS	| \
				CFG_CMD_SNTP	)

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

#define CFG_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0400000	/* 1 ... 4 MB in DRAM	*/

#define CFG_LOAD_ADDR	 	0x00100000

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
#define CFG_IMMR_SIZE		((uint)(64 * 1024))

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
#define CFG_FLASH_BASE		0x40000000

#define CFG_RESET_ADDRESS	0xFFF00100

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc()	*/

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
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/


#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x00740000

#define CFG_ENV_SECT_SIZE	0x20000	/* Total Size of Environment sector	*/
#define	CFG_ENV_SIZE		0x4000	/* Used Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*
 * NAND flash support
 */
#define CFG_MAX_NAND_DEVICE	1
#define NAND_ChipID_UNKNOWN	0x00
#define SECTORSIZE		512
#define NAND_MAX_FLOORS		1
#define NAND_MAX_CHIPS		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3
#define ADDR_COLUMN		1
#define NAND_NO_RB

#define NAND_WAIT_READY(nand)		udelay(12)
#define WRITE_NAND_COMMAND(d, adr)	WRITE_NAND(d, adr + 2)
#define WRITE_NAND_ADDRESS(d, adr)	WRITE_NAND(d, adr + 1)
#define WRITE_NAND(d, adr)		(*(volatile uint8_t *)(adr) = (uint8_t)(d))
#define READ_NAND(adr)			(*(volatile uint8_t *)(adr))
#define NAND_DISABLE_CE(nand)		/* nop */
#define NAND_ENABLE_CE(nand)		/* nop */
#define NAND_CTL_CLRALE(nandptr)	/* nop */
#define NAND_CTL_SETALE(nandptr)	/* nop */
#define NAND_CTL_CLRCLE(nandptr)	/* nop */
#define NAND_CTL_SETCLE(nandptr)	/* nop */

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control					11-9
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
 * SIUMCR - SIU Module Configuration					11-6
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

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
#define SCCR_MASK	SCCR_EBDF11
#define CFG_SCCR	(SCCR_COM00	| SCCR_DFSYNC00	| \
			 SCCR_DFBRG00	| SCCR_DFNL000	| SCCR_DFNH000	| \
			 SCCR_DFLCD000	| SCCR_DFALCD00)

 /*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER		0

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/

#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/* FLASH timing: Default value of OR0 after reset */
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_MSK | OR_BI | \
				 OR_SCY_15_CLK | OR_TRLX)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_8 | BR_V)

/*
 * BR2 and OR2 (NAND Flash) - now addressed through UPMB
 */
#define CFG_NAND_BASE		0x50000000
#define CFG_NAND_SIZE		0x04000000

#define CFG_OR_TIMING_NAND	(OR_CSNT_SAM | OR_ACS_DIV1 | OR_BI | \
				 OR_SCY_15_CLK | OR_EHTR | OR_TRLX)

#define CFG_BR2_PRELIM  ((CFG_NAND_BASE & BR_BA_MSK) | BR_PS_8 | BR_MS_UPMB | BR_V  )
#define CFG_OR2_PRELIM  (((-CFG_NAND_SIZE) & OR_AM_MSK) | OR_BI )

/*
 * BR3 and OR3 (SDRAM)
 */
#define SDRAM_BASE3_PRELIM	0x00000000	/* SDRAM bank		*/
#define SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

 /*
  * SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)
  */
#define CFG_OR_TIMING_SDRAM	0x00000A00

#define CFG_OR3_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_SDRAM)
#define CFG_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V)

/*
 * BR5 and OR5 (SRAM)
 */
#define CFG_SRAM_BASE		0x60000000
#define CFG_SRAM_SIZE		0x00080000

#define CFG_OR_TIMING_SRAM	(OR_CSNT_SAM | OR_ACS_DIV1 | OR_BI | \
				 OR_SCY_15_CLK | OR_EHTR | OR_TRLX)

#define CFG_BR5_PRELIM  ((CFG_SRAM_BASE & BR_BA_MSK) | BR_PS_8 | BR_V )
#define CFG_OR5_PRELIM  (((-CFG_SRAM_SIZE) & OR_AM_MSK) | CFG_OR_TIMING_SRAM)


/*
 * 4096 Rows from SDRAM example configuration
 * 1000 factor s -> ms
 * 64   PTP (pre-divider from MPTPR) from SDRAM example configuration
 * 4    Number of refresh cycles per period
 * 64   Refresh cycle in ms per number of rows
 */
#define CFG_PTA_PER_CLK		((4096 * 64 * 1000) / (4 * 64))

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA		39

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CFG_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CFG_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

#define CFG_MAMR_8COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE | \
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 | \
			 MAMR_RLFA_1X    | MAMR_WLFA_1X    | MAMR_TLFA_4X)
#define CFG_MAMR_9COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE | \
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 | \
			 MAMR_RLFA_1X    | MAMR_WLFA_1X    | MAMR_TLFA_4X)

/*
 * MBMR settings for NAND flash
 */

#define CFG_MBMR_NAND ( MBMR_WLFB_5X )

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#define CONFIG_JFFS2_NAND 1			/* jffs2 on nand support */
#define NAND_CACHE_PAGES 16			/* size of nand cache in 512 bytes pages */

/*
 * JFFS2 partitions
 */

/* No command line, one static partition */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_SIZE		0x00400000
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=nc650-0,nand0=nc650-nand"

#define MTDPARTS_DEFAULT	"mtdparts=nc650-0:1m(kernel1),1m(kernel2)," \
					"2560k(cramfs1),2560k(cramfs2)," \
					"256k(u-boot),256k(env);" \
				"nc650-nand:4m(nand1),28m(nand2)"
*/

#endif	/* __CONFIG_H */
