/*
 * (C) Copyright 2001, 2002
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

#include <mpc8xx_irq.h>


# ifdef DEBUG
#  warning DEBUG Defined
# endif /* DEBUG */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC860		1
#define CONFIG_IAD210		1	/* ...on a IAD210  module	*/
#define CONFIG_MPC860T		1
#define CONFIG_MPC862		1

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

#undef  CONFIG_8xx_CONS_SMC1
#undef  CONFIG_8xx_CONS_SMC2
#define CONFIG_8xx_CONS_SCC2   /* V24 on SCC2 */
#undef  CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600


# define MPC8XX_FACT            16
# define CONFIG_8xx_GCLK_FREQ   (64000000L)  /* define if can't use get_gclk_freq */
# define CONFIG_CLOCKS_IN_MHZ	1            /* clocks passsed to Linux in MHz */

#if 0
# define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
# define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

/* using this define saves us updating another source file */
#define CONFIG_BOARD_EARLY_INIT_F 1

#undef	CONFIG_BOOTARGS
/* #define CONFIG_BOOTCOMMAND							\
 	"bootp;" 								\
 	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " 	\
 	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" 	\
 	"bootm"
*/

#define CONFIG_BOOTCOMMAND	\
	"setenv bootargs root=/dev/nfs" 	\
	"ip=192.168.28.129:139.10.137.138:192.168.28.1:255.255.255.0:iadlinux002::off; " 	\

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

/* #define	CONFIG_STATUS_LED	1*/	/* Status LED enabled		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


# undef  CONFIG_SCC1_ENET		/* disable SCC1 ethernet */
# define CONFIG_FEC_ENET    1	/* use FEC ethernet  */
# define CONFIG_MII         1
# define CFG_DISCOVER_PHY   1
# define CONFIG_FEC_UTOPIA  1
# define CONFIG_ETHADDR     08:00:06:26:A2:6D
# define CONFIG_IPADDR      192.168.28.128
# define CONFIG_SERVERIP    139.10.137.138
# define CFG_DISCOVER_PHY   1

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/* enable I2C and select the hardware/software driver */
#undef  CONFIG_HARD_I2C			/* I2C with hardware support    */
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged               */
# define CFG_I2C_SPEED		50000
# define CFG_I2C_SLAVE		0xDD
# define CFG_I2C_EEPROM_ADDR	0x50
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL		0x00000020	/* PB 26 */
#define PB_SDA		0x00000010	/* PB 27 */

#define I2C_INIT	(immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE	(immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE	(immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ	((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DATE


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
#define CFG_IMMR		0xFFF00000
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
#define CFG_FLASH_BASE		0x08000000
#define CFG_FLASH_SIZE		((uint)(4 * 1024 * 1024))	/* max 16Mbyte */

#define CFG_RESET_ADDRESS	0xFFF00100

#if defined(DEBUG)
# define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
# define	CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif

# define CFG_MONITOR_BASE	CFG_FLASH_BASE
# define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

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
#define CFG_MAX_FLASH_SECT	67	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500		/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x8000
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

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
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CFG_PLPRCR	(((MPC8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
				PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11

#define CFG_SCCR	(SCCR_TBS	| SCCR_COM00	| SCCR_DFSYNC00	| \
			 SCCR_DFBRG00	| SCCR_DFNL000	| SCCR_DFNH000	| \
			 SCCR_DFLCD000	|SCCR_DFALCD00	)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration Register		19-4
 *-----------------------------------------------------------------------
 */
/* +0x09C4 => DRQP = 10 (IDMA requests have lowest priority) */
#define CFG_RCCR 0x0020

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 */
#define PCMCIA_MEM_ADDR		((uint)0xff020000)
#define PCMCIA_MEM_SIZE		((uint)(64 * 1024))

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER		0

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
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0xF8000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xF8000000	/* OR addr mask */

/* FLASH timing:
 TRLX = 0, CSNT = 1, SCY = 3, EHTR = 1	*/
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_BI | \
				 OR_SCY_3_CLK | OR_EHTR)

#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )

/*
 * BR2/3 and OR2/3 (SDRAM)
 *
 */
#define SDRAM_BASE_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/

#define CFG_OR2_PRELIM	(CFG_PRELIM_OR_AM | OR_CSNT_SAM  | OR_BI | OR_ACS_DIV4)
#define CFG_BR2_PRELIM	((SDRAM_BASE_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )
#define CFG_BR1_PRELIM	((SDRAM_BASE1_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V)

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA	124		/* start with divider for 64 MHz	*/

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CFG_MPTPR	        MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

#define CFG_MAMR	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_8X)


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#ifdef CONFIG_MPC860T

/* Interrupt level assignments.
*/
#define FEC_INTERRUPT	SIU_LEVEL1	/* FEC interrupt */

#endif /* CONFIG_MPC860T */


#endif	/* __CONFIG_H */
