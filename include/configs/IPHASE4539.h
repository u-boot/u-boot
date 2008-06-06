/*
 * (C) Copyright 2002 Wolfgang Grandegger <wg@denx.de>
 *
 * This file is based on similar values for other boards found in
 * other U-Boot config files, mainly tqm8260.h and mpc8260ads.h.
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
 * Config header file for a Interphase 4539 PMC, 64 MB SDRAM, 4MB Flash.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU   */
#define CONFIG_IPHASE4539	1	/* ...on a Interphase 4539 PMC */

#define CONFIG_CPM2		1	/* Has a CPM2 */

/*-----------------------------------------------------------------------
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere (for example, on the cogent platform, there are serial
 * ports on the motherboard which are used for the serial console - see
 * cogent/cma101/serial.[ch]).
 */
#define	CONFIG_CONS_ON_SMC		/* define if console on SMC */
#undef	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on something else */
#define CONFIG_CONS_INDEX	1	/* which serial channel for console */

/*-----------------------------------------------------------------------
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC   */
#define CONFIG_ETHER_ON_FCC		/* define if ether on FCC   */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define CONFIG_ETHER_INDEX	3	/* which channel for ether  */

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 3)

/*-----------------------------------------------------------------------
 * - Rx-CLK is CLK14
 * - Tx-CLK is CLK16
 * - Select bus for bd/buffers (see 28-13)
 * - Half duplex
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC3 | CMXFCR_RF3CS_MSK | CMXFCR_TF3CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK14 | CMXFCR_TF3CS_CLK16)
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

#endif	/* CONFIG_ETHER_ON_FCC, CONFIG_ETHER_INDEX */

/* other options */

#define CONFIG_8260_CLKIN	66666666	/* in Hz */
#define CONFIG_BAUDRATE		19200

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#undef  CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

/*
 * Software (bit-bang) I2C driver configuration
 */
#ifdef CONFIG_SOFT_I2C
#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00010000; \
			else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00020000; \
			else    iop->pdat &= ~0x00020000
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */
#endif /* CONFIG_SOFT_I2C */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>


#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_BOOTCOMMAND	"bootm 100000"	/* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/ram rw"

#if defined(CONFIG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which serial channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
#endif

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00F00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_CLOCKS_IN_MHZ	1	/* clocks passed to Linux in MHz */
					/* for versions < 2.4.5-pre5	 */

#define CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_RESET_ADDRESS	0x04400000

#define CONFIG_MISC_INIT_R	1	/* We need misc_init_r()	*/

/*-----------------------------------------------------------------------
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration (Setup by the
 * startup code). Please note that CFG_SDRAM_BASE _must_ start at 0.
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFF800000

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(256 << 10)     /* Reserve 256 kB for Monitor  */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc() */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	64	/* max num of sects on one chip */
#define CFG_MAX_FLASH_SIZE	(CFG_MAX_FLASH_SECT * 0x10000)	/* 4 MB */

#define CFG_FLASH_ERASE_TOUT	2400000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

/* Environment in FLASH, there is little space left in Serial EEPROM */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SECT_SIZE	0x10000 /* We use one complete sector	*/
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x10000) /* 2. sector */


/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CFG_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CFG_SIUMCR.
 */
#define CFG_HRCW_MASTER ( ( HRCW_BPS01 | HRCW_EBM )		|\
			  ( HRCW_L2CPC10 | HRCW_ISB110 )	|\
			  ( HRCW_MMR11 | HRCW_APPC10 )		|\
			  ( HRCW_CS10PC01 | HRCW_MODCK_H0101 )	 \
			) /* 0x14863245 */

/* no slaves */
#define CFG_HRCW_SLAVE1 0
#define CFG_HRCW_SLAVE2 0
#define CFG_HRCW_SLAVE3 0
#define CFG_HRCW_SLAVE4 0
#define CFG_HRCW_SLAVE5 0
#define CFG_HRCW_SLAVE6 0
#define CFG_HRCW_SLAVE7 0

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFF000000 /* We keep original value */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	  /* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02	  /* Software reboot			*/


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32     /* For MPC8260 CPU		*/
#if defined(CONFIG_CMD_KGDB)
# define CFG_CACHELINE_SHIFT	5      /* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers		2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT	(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
			 HID0_IFEM|HID0_ABE)
#define CFG_HID0_FINAL	(HID0_IFEM|HID0_ABE)
#define CFG_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register					 5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CFG_RMR		RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR		0xA01C0000

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR	0X4205C000

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				 4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined (CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 *-----------------------------------------------------------------------
 */
#define CFG_TMCNTSC	(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR	(PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CFG_SCCR	0

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR	0

/*-----------------------------------------------------------------------
 * Init Memory Controller:
 *
 * Bank Bus	Machine PortSz	Device
 * ---- ---	------- ------	------
 *  0	60x	GPCM	64 bit	FLASH
 *  1	60x	SDRAM	64 bit	SDRAM
 */

#define CFG_BR0_PRELIM	((CFG_FLASH_BASE & BRx_BA_MSK) | 0x0801)
#define CFG_OR0_PRELIM	0xFF800882
#define CFG_BR1_PRELIM	((CFG_SDRAM_BASE & BRx_BA_MSK) | 0x0041)
#define CFG_OR1_PRELIM	0xF8002CD0

#define CFG_PSDMR	0x404A241A
#define CFG_MPTPR	0x00007400
#define CFG_PSRT	0x00000007

#endif /* __CONFIG_H */
