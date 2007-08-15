/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
 *
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Jay Monkman <jtm@smoothsmoothie.com>
 *
 * Configuation settings for the WindRiver PPMC8260 board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*****************************************************************************
 *
 * These settings must match the way _your_ board is set up
 *
 *****************************************************************************/

/* What is the oscillator's (UX2) frequency in Hz? */
#define CONFIG_8260_CLKIN  (66 * 1000 * 1000)

/*-----------------------------------------------------------------------
 * MODCK_H & MODCLK[1-3] - Ref: Section 9.2 in MPC8206 User Manual
 *-----------------------------------------------------------------------
 * What should MODCK_H be? It is dependent on the oscillator
 * frequency, MODCK[1-3], and desired CPM and core frequencies.
 * Here are some example values (all frequencies are in MHz):
 *
 * MODCK_H   MODCK[1-3]	 Osc	CPM    Core  S2-6   S2-7   S2-8
 * -------   ----------	 ---	---    ----  -----  -----  -----
 * 0x2	     0x2	 33	133    133   Close  Open   Close
 * 0x2	     0x3	 33	133    166   Close  Open   Open
 * 0x2	     0x4	 33	133    200   Open   Close  Close
 * 0x2	     0x5	 33	133    233   Open   Close  Open
 * 0x2	     0x6	 33	133    266   Open   Open   Close
 *
 * 0x5	     0x5	 66	133    133   Open   Close  Open
 * 0x5	     0x6	 66	133    166   Open   Open   Close
 * 0x5	     0x7	 66	133    200   Open   Open   Open
 * 0x6	     0x0	 66	133    233   Close  Close  Close
 * 0x6	     0x1	 66	133    266   Close  Close  Open
 * 0x6	     0x2	 66	133    300   Close  Open   Close
 */
#define CFG_PPMC_MODCK_H 0x05

/* Define this if you want to boot from 0x00000100. If you don't define
 * this, you will need to program the bootloader to 0xfff00000, and
 * get the hardware reset config words at 0xfe000000. The simplest
 * way to do that is to program the bootloader at both addresses.
 * It is suggested that you just let U-Boot live at 0x00000000.
 */
#define CFG_PPMC_BOOT_LOW 1

/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain TEXT_BASE from board/ppmc8260/config.mk
 * The main FLASH is whichever is connected to *CS0. U-Boot expects
 * this to be the SIMM.
 */
#define CFG_FLASH0_BASE 0xFE000000
#define CFG_FLASH0_SIZE 16

/* What should be the base address of the first SDRAM DIMM and how big is
 * it (in Mbytes)?
*/
#define CFG_SDRAM0_BASE 0x00000000
#define CFG_SDRAM0_SIZE 128

/* What should be the base address of the second SDRAM DIMM and how big is
 * it (in Mbytes)?
*/
#define CFG_SDRAM1_BASE 0x08000000
#define CFG_SDRAM1_SIZE 128

/* What should be the base address of the on board SDRAM and how big is
 * it (in Mbytes)?
*/
#define CFG_SDRAM2_BASE 0x38000000
#define CFG_SDRAM2_SIZE 16

/* What should be the base address of the MAILBOX  and how big is it
 * (in Bytes)
 * The eeprom lives at CFG_MAILBOX_BASE + 0x80000000
 */
#define CFG_MAILBOX_BASE 0x32000000
#define CFG_MAILBOX_SIZE 8192

/* What is the base address of the I/O select lines and how big is it
 * (In Mbytes)?
 */

#define CFG_IOSELECT_BASE 0xE0000000
#define CFG_IOSELECT_SIZE 32


/* What should be the base address of the LEDs and switch S0?
 * If you don't want them enabled, don't define this.
 */
#define CFG_LED_BASE 0xF1000000

/*
 * PPMC8260 with 256 16 MB DIMM:
 *
 *     0x0000 0000     Exception Vector code, 8k
 *	     :
 *     0x0000 1FFF
 *     0x0000 2000     Free for Application Use
 *	     :
 *	     :
 *
 *	     :
 *	     :
 *     0x0FF5 FF30     Monitor Stack (Growing downward)
 *		       Monitor Stack Buffer (0x80)
 *     0x0FF5 FFB0     Board Info Data
 *     0x0FF6 0000     Malloc Arena
 *	     :		    CFG_ENV_SECT_SIZE, 256k
 *	     :		    CFG_MALLOC_LEN,    128k
 *     0x0FFC 0000     RAM Copy of Monitor Code
 *	     :		    CFG_MONITOR_LEN,   256k
 *     0x0FFF FFFF     [End of RAM], CFG_SDRAM_SIZE - 1
 */


/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere.
 * The console can be on SMC1 or SMC2
 */
#define CONFIG_CONS_ON_SMC	1	/* define if console on SMC */
#undef	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on neither */
#define CONFIG_CONS_INDEX	1	/* which SMC/SCC channel for console */

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */

#undef	CONFIG_ETHER_ON_SCC		/* define if ethernet on SCC	*/
#define CONFIG_ETHER_ON_FCC		/* define if ethernet on FCC	*/
#undef	CONFIG_ETHER_NONE		/* define if ethernet on neither */
#define CONFIG_ETHER_INDEX	2	/* which SCC/FCC channel for ethernet */
#define CONFIG_MII			/* MII PHY management	*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * Port pins used for bit-banged MII communictions (if applicable).
 */
#define MDIO_PORT	2	/* Port C */
#define MDIO_ACTIVE	(iop->pdir |=  0x00400000)
#define MDIO_TRISTATE	(iop->pdir &= ~0x00400000)
#define MDIO_READ	((iop->pdat &  0x00400000) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  0x00400000; \
			else	iop->pdat &= ~0x00400000

#define MDC(bit)	if(bit) iop->pdat |=  0x00200000; \
			else	iop->pdat &= ~0x00200000

#define MIIDELAY	udelay(1)


/* Define this to reserve an entire FLASH sector (256 KB) for
 * environment variables. Otherwise, the environment will be
 * put in the same sector as U-Boot, and changing variables
 * will erase U-Boot temporarily
 */
#define CFG_ENV_IN_OWN_SECT	1

/* Define to allow the user to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* What should the console's baud rate be? */
#define CONFIG_BAUDRATE		9600

/* Ethernet MAC address */

#define CONFIG_ETHADDR		00:a0:1e:90:2b:00

/* Define this to set the last octet of the ethernet address
 * from the DS0-DS7 switch and light the leds with the result
 * The DS0-DS7 switch and the leds are backwards with respect
 * to each other. DS7 is on the board edge side of both the
 * led strip and the DS0-DS7 switch.
 */
#define CONFIG_MISC_INIT_R

/* Set to a positive value to delay for running BOOTCOMMAND */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#if 0
/* Be selective on what keys can delay or stop the autoboot process
 *     To stop	use: " "
 */
# define CONFIG_AUTOBOOT_KEYED
# define CONFIG_AUTOBOOT_PROMPT "Autobooting in %d seconds, press \" \" to stop\n"
# define CONFIG_AUTOBOOT_STOP_STR	" "
# undef CONFIG_AUTOBOOT_DELAY_STR
# define DEBUG_BOOTKEYS		0
#endif

/* Define a command string that is automatically executed when no character
 * is read on the console interface withing "Boot Delay" after reset.
 */
#undef	CONFIG_BOOT_ROOT_INITRD 	/* Use ram disk for the root file system */
#define	CONFIG_BOOT_ROOT_NFS		/* Use a NFS mounted root file system */

#ifdef CONFIG_BOOT_ROOT_INITRD
#define CONFIG_BOOTCOMMAND \
	"version;" \
	"echo;" \
	"bootp;" \
	"setenv bootargs root=/dev/ram0 rw " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
	"bootm"
#endif /* CONFIG_BOOT_ROOT_INITRD */

#ifdef CONFIG_BOOT_ROOT_NFS
#define CONFIG_BOOTCOMMAND \
	"version;" \
	"echo;" \
	"bootp;" \
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
	"bootm"
#endif /* CONFIG_BOOT_ROOT_NFS */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_DNS


/* undef this to save memory */
#define CFG_LONGHELP

/* Monitor Command Prompt */
#define CFG_PROMPT		"=> "


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_CMD_MII
#define CONFIG_CMD_IMMAP

#undef CONFIG_CMD_KGDB


/* Where do the internal registers live? */
#define CFG_IMMR		0xf0000000

/*****************************************************************************
 *
 * You should not have to modify any of the following settings
 *
 *****************************************************************************/

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU   */
#define CONFIG_PPMC8260		1	/* on an Wind River PPMC8260 Board  */
#define CONFIG_CPM2		1	/* Has a CPM2 */

/*
 * Miscellaneous configurable options
 */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CBSIZE		1024	/* Console I/O Buffer Size	     */
#else
#  define CFG_CBSIZE		256	/* Console I/O Buffer Size	     */
#endif

/* Print Buffer Size */
#define CFG_PBSIZE	  (CFG_CBSIZE + sizeof(CFG_PROMPT)+16)

#define CFG_MAXARGS		32	/* max number of command args	*/

#define CFG_BARGSIZE		CFG_CBSIZE /* Boot Argument Buffer Size	   */

#define CFG_LOAD_ADDR		0x140000   /* default load address */
#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_MEMTEST_START	0x2000	/* memtest works from the end of */
					/* the exception vector table */
					/* to the end of the DRAM  */
					/* less monitor and malloc area */
#define CFG_STACK_USAGE		0x10000 /* Reserve 64k for the stack usage */
#define CFG_MEM_END_USAGE	( CFG_MONITOR_LEN \
				+ CFG_MALLOC_LEN \
				+ CFG_ENV_SECT_SIZE \
				+ CFG_STACK_USAGE )

#define CFG_MEMTEST_END		( CFG_SDRAM_SIZE * 1024 * 1024 \
				- CFG_MEM_END_USAGE )

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#if defined(CONFIG_ETHER_ON_SCC) && (CONFIG_ETHER_INDEX == 1)
/*
 *  Attention: This is board specific
 *  - RX clk is CLK11
 *  - TX clk is CLK12
 */
#define CFG_CMXSCR_VALUE       (CMXSCR_RS1CS_CLK11  |\
				CMXSCR_TS1CS_CLK12)

#elif defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)
/*
 * Attention: this is board-specific
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - Select bus for bd/buffers (see 28-13)
 * - Enable Full Duplex in FSMR
 */
#define CFG_CMXFCR_MASK		(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
#define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
#define CFG_CPMFCR_RAMTYPE	0
#define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)
#endif	/* CONFIG_ETHER_INDEX */

#define CFG_FLASH_BASE	CFG_FLASH0_BASE
#define CFG_FLASH_SIZE	CFG_FLASH0_SIZE
#define CFG_SDRAM_BASE	CFG_SDRAM0_BASE
#define CFG_SDRAM_SIZE	(CFG_SDRAM0_SIZE + CFG_SDRAM1_SIZE)

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */
#if defined(CFG_PPMC_BOOT_LOW)
#  define  CFG_PPMC_HRCW_BOOT_FLAGS  (HRCW_CIP | HRCW_BMS)
#else
#  define  CFG_PPMC_HRCW_BOOT_FLAGS  (0)
#endif /* defined(CFG_PPMC_BOOT_LOW) */

/* get the HRCW ISB field from CFG_IMMR */
#define CFG_PPMC_HRCW_IMMR	( ((CFG_IMMR & 0x10000000) >> 10) | \
				  ((CFG_IMMR & 0x01000000) >>  7) | \
				  ((CFG_IMMR & 0x00100000) >>  4) )

#define CFG_HRCW_MASTER		( HRCW_EBM				| \
				  HRCW_BPS11				| \
				  HRCW_L2CPC10				| \
				  HRCW_DPPC00				| \
				  CFG_PPMC_HRCW_IMMR			| \
				  HRCW_MMR00				| \
				  HRCW_LBPC00				| \
				  HRCW_APPC10				| \
				  HRCW_CS10PC00				| \
				  (CFG_PPMC_MODCK_H & HRCW_MODCK_H1111) | \
				  CFG_PPMC_HRCW_BOOT_FLAGS )

/* no slaves */
#define CFG_HRCW_SLAVE1		0
#define CFG_HRCW_SLAVE2		0
#define CFG_HRCW_SLAVE3		0
#define CFG_HRCW_SLAVE4		0
#define CFG_HRCW_SLAVE5		0
#define CFG_HRCW_SLAVE6		0
#define CFG_HRCW_SLAVE7		0

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 * Note also that the logic that sets CFG_RAMBOOT is platform dependent.
 */
#define CFG_MONITOR_BASE	CFG_FLASH0_BASE

#ifndef CFG_MONITOR_BASE
#define CFG_MONITOR_BASE	0x0ff80000
#endif

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#  define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 374 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CFG_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_FLASH_INCREMENT	0	/* there is only one bank		*/
#define CFG_FLASH_PROTECTION	1	/* use hardware protection		*/
#define CFG_FLASH_USE_BUFFER_WRITE 1    /* use buffered writes (20x faster)     */


#ifndef CFG_RAMBOOT

#  define CFG_ENV_IS_IN_FLASH	1
#  ifdef CFG_ENV_IN_OWN_SECT
#    define CFG_ENV_ADDR	(CFG_MONITOR_BASE + 0x40000)
#    define CFG_ENV_SECT_SIZE	0x40000
#  else
#    define CFG_ENV_ADDR (CFG_FLASH_BASE + CFG_MONITOR_LEN - CFG_ENV_SECT_SIZE)
#    define CFG_ENV_SIZE	0x1000	/* Total Size of Environment Sector	*/
#    define CFG_ENV_SECT_SIZE	0x40000 /* see README - env sect real size	*/
#  endif /* CFG_ENV_IN_OWN_SECT */

#else
#  define CFG_ENV_IS_IN_FLASH	1
#  define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x40000)
#define CFG_ENV_SIZE		0x1000
#  define CFG_ENV_SECT_SIZE	0x40000
#endif /* CFG_RAMBOOT */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU */

#if defined(CONFIG_CMD_KGDB)
# define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers			 2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT	(HID0_ICE  |\
			 HID0_DCE  |\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)

#define CFG_HID0_FINAL	(HID0_ICE  |\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#define CFG_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register
 *-----------------------------------------------------------------------
 */
#define CFG_RMR		0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR		(BCR_EBM      |\
			 0x30000000)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 * Ref Section 4.3.2.6	page 4-31
 *-----------------------------------------------------------------------
 */

#define CFG_SIUMCR	(SIUMCR_ESE	 |\
			 SIUMCR_DPPC00	 |\
			 SIUMCR_L2CPC10	 |\
			 SIUMCR_LBPC00	 |\
			 SIUMCR_APPC10	 |\
			 SIUMCR_CS10PC00 |\
			 SIUMCR_BCTLC00	 |\
			 SIUMCR_MMR00)


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#define CFG_SYPCR	(SYPCR_SWTC |\
			 SYPCR_BMT  |\
			 SYPCR_PBME |\
			 SYPCR_LBME |\
			 SYPCR_SWRI |\
			 SYPCR_SWP)

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC	(TMCNTSC_SEC |\
			 TMCNTSC_ALR |\
			 TMCNTSC_TCF |\
			 TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR	(PISCR_PS  |\
			 PISCR_PTF |\
			 PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 */
#define CFG_SCCR	0

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR	0

/*
 * Initialize Memory Controller:
 *
 * Bank Bus	Machine PortSz	Device
 * ---- ---	------- ------	------
 *  0	60x	GPCM	32 bit	FLASH (SIMM - 32MB) *
 *  1	unused
 *  2	60x	SDRAM	64 bit	SDRAM (DIMM - 128MB)
 *  3	60x	SDRAM	64 bit	SDRAM (DIMM - 128MB)
 *  4	Local	SDRAM	32 bit	SDRAM (on board - 16MB)
 *  5	60x	GPCM	 8 bit	Mailbox/EEPROM (8KB)
 *  6	60x	GPCM	 8 bit	FLASH  (on board - 2MB) *
 *  7	60x	GPCM	 8 bit	LEDs, switches
 *
 *  (*) This configuration requires the PPMC8260 be configured
 *	so that *CS0 goes to the FLASH SIMM, and *CS6 goes to
 *	the on board FLASH. In other words, JP24 should have
 *	pins 1 and 2 jumpered and pins 3 and 4 jumpered.
 *
 */

/*-----------------------------------------------------------------------
 * BR0,BR1 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR0,OR1 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 0,1 - FLASH SIMM
 *
 * This expects the FLASH SIMM to be connected to *CS0
 * It consists of 4 AM29F080B parts.
 *
 * Note: For the 4 MB SIMM, *CS1 is unused.
 */

/* BR0 is configured as follows:
 *
 *     - Base address of 0xFE000000
 *     - 32 bit port size
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - GPCM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR0_PRELIM	((CFG_FLASH0_BASE & BRx_BA_MSK) |\
			 BRx_PS_32			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

/* OR0 is configured as follows:
 *
 *     - 32 MB
 *     - *BCTL0 is asserted upon access to the current memory bank
 *     - *CW / *WE are negated a quarter of a clock earlier
 *     - *CS is output at the same time as the address lines
 *     - Uses a clock cycle length of 5
 *     - *PSDVAL is generated internally by the memory controller
 *	 unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *	 initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *	 current bank and the next access.
 */
#define CFG_OR0_PRELIM	(MEG_TO_AM(CFG_FLASH0_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV1			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_TRLX			|\
			 ORxG_EHTR)

/*-----------------------------------------------------------------------
 * BR2,BR3 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR2,OR3 - Option Register
 *     Ref: Section 10.3.2 on page 10-16
 *-----------------------------------------------------------------------
 */

/*
 * Bank 2,3 - 128 MB SDRAM DIMM
 */

/* With a 128 MB DIMM, the BR2 is configured as follows:
 *
 *     - Base address of 0x00000000/0x08000000
 *     - 64 bit port size (60x bus only)
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - SDRAM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR2_PRELIM	((CFG_SDRAM0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

#define CFG_BR3_PRELIM	((CFG_SDRAM1_BASE & BRx_BA_MSK) |\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

/* With a 128 MB DIMM, the OR2 is configured as follows:
 *
 *     - 128 MB
 *     - 4 internal banks per device
 *     - Row start address bit is A8 with PSDMR[PBI] = 0
 *     - 13 row address lines
 *     - Back-to-back page mode
 *     - Internal bank interleaving within save device enabled
 */

#define CFG_OR2_PRELIM	(MEG_TO_AM(CFG_SDRAM0_SIZE)	|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A7		|\
			 ORxS_NUMR_13)

#define CFG_OR3_PRELIM	(MEG_TO_AM(CFG_SDRAM1_SIZE)	|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A7		|\
			 ORxS_NUMR_13)


/*-----------------------------------------------------------------------
 * PSDMR - 60x Bus SDRAM Mode Register
 *     Ref: Section 10.3.3 on page 10-21
 *-----------------------------------------------------------------------
 */

/* With a 128 MB DIMM, the PSDMR is configured as follows:
 *
 *     - Page Based Interleaving,
 *     - Refresh Enable,
 *     - Normal Operation
 *     - Address Multiplexing where A5 is output on A14 pin
 *	 (A6 on A15, and so on),
 *     - use address pins A13-A15 as bank select,
 *     - A9 is output on SDA10 during an ACTIVATE command,
 *     - earliest timing for ACTIVATE command after REFRESH command is 7 clocks,
 *     - earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 *	 is 3 clocks,
 *     - earliest timing for READ/WRITE command after ACTIVATE command is
 *	 2 clocks,
 *     - earliest timing for PRECHARGE after last data was read is 1 clock,
 *     - earliest timing for PRECHARGE after last data was written is 1 clock,
 *     - External Address Multiplexing enabled
 *     - CAS Latency is 2.
 */
#define CFG_PSDMR	(PSDMR_RFEN	      |\
			 PSDMR_SDAM_A14_IS_A5 |\
			 PSDMR_BSMA_A13_A15   |\
			 PSDMR_SDA10_PBI0_A9  |\
			 PSDMR_RFRC_7_CLK     |\
			 PSDMR_PRETOACT_3W    |\
			 PSDMR_ACTTORW_2W     |\
			 PSDMR_LDOTOPRE_1C    |\
			 PSDMR_WRC_1C	      |\
			 PSDMR_EAMUX	      |\
			 PSDMR_CL_2)


#define CFG_PSRT	0x0e
#define CFG_MPTPR	MPTPR_PTP_DIV32


/*-----------------------------------------------------------------------
 * BR4 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR4 - Option Register
 *     Ref: Section 10.3.2 on page 10-16
 *-----------------------------------------------------------------------
 */

/*
 * Bank 4 - On board SDRAM
 *
 */
/* With 16 MB of onboard SDRAM	BR4 is configured as follows
 *
 *     - Base address 0x38000000
 *     - 32 bit port size
 *     - Data error checking disabled
 *     - Read/Write access
 *     - SDRAM local bus
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 *
 */

#define CFG_BR4_PRELIM	((CFG_SDRAM2_BASE & BRx_BA_MSK) |\
			 BRx_PS_32			|\
			 BRx_DECC_NONE			|\
			 BRx_MS_SDRAM_L			|\
			 BRx_V)

/*
 * With 16MB SDRAM, OR4 is configured as follows
 *     - 4 internal banks per device
 *     - Row start address bit is A10 with LSDMR[PBI] = 0
 *     - 12 row address lines
 *     - Back-to-back page mode
 *     - Internal bank interleaving within save device enabled
 */

#define CFG_OR4_PRELIM	(MEG_TO_AM(CFG_SDRAM2_SIZE)	|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A10		|\
			 ORxS_NUMR_12)


/*-----------------------------------------------------------------------
 * LSDMR - Local Bus SDRAM Mode Register
 *     Ref: Section 10.3.4 on page 10-24
 *-----------------------------------------------------------------------
 */

/* With a 16 MB onboard SDRAM, the LSDMR is configured as follows:
 *
 *     - Page Based Interleaving,
 *     - Refresh Enable,
 *     - Normal Operation
 *     - Address Multiplexing where A5 is output on A13 pin
 *	 (A6 on A15, and so on),
 *     - use address pins A15-A17 as bank select,
 *     - A11 is output on SDA10 during an ACTIVATE command,
 *     - earliest timing for ACTIVATE command after REFRESH command is 7 clocks,
 *     - earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 *	 is 2 clocks,
 *     - earliest timing for READ/WRITE command after ACTIVATE command is
 *	 2 clocks,
 *     - SDRAM burst length is 8
 *     - earliest timing for PRECHARGE after last data was read is 1 clock,
 *     - earliest timing for PRECHARGE after last data was written is 1 clock,
 *     - External Address Multiplexing disabled
 *     - CAS Latency is 2.
 */
#define CFG_LSDMR	(PSDMR_RFEN	      |\
			 PSDMR_SDAM_A13_IS_A5 |\
			 PSDMR_BSMA_A15_A17   |\
			 PSDMR_SDA10_PBI0_A11 |\
			 PSDMR_RFRC_7_CLK     |\
			 PSDMR_PRETOACT_2W    |\
			 PSDMR_ACTTORW_2W     |\
			 PSDMR_BL	      |\
			 PSDMR_LDOTOPRE_1C    |\
			 PSDMR_WRC_1C	      |\
			 PSDMR_CL_2)

#define CFG_LSRT	0x0e

/*-----------------------------------------------------------------------
 * BR5 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR5 - Option Register
 *     Ref: Section 10.3.2 on page 10-16
 *-----------------------------------------------------------------------
 */

/*
 * Bank 5 EEProm and Mailbox
 *
 * The EEPROM and mailbox live on the same chip select.
 * the eeprom is selected if the MSb of the address is set and the mailbox is
 * selected if the MSb of the address is clear.
 *
 */

/* BR5 is configured as follows:
 *
 *     - Base address of 0x32000000/0xF2000000
 *     - 8 bit
 *     - Data error checking disabled
 *     - Read/Write access
 *     - GPCM 60x Bus
 *     - SDRAM local bus
 *     - No data pipelining is done
 *     - Valid
 */

#define CFG_BR5_PRELIM	((CFG_MAILBOX_BASE & BRx_BA_MSK) |\
			 BRx_PS_8			 |\
			 BRx_DECC_NONE			 |\
			 BRx_MS_GPCM_P			 |\
			 BRx_V)
/* OR5 is configured as follows
 *     - buffer control enabled
 *     - chip select negated normally
 *     - CS output 1/2 clock after address
 *     - 15 wait states
 *     - *PSDVAL is generated internally by the memory controller
 *	 unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *	 initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *	 current bank and the next access.
 */

#define CFG_OR5_PRELIM ((P2SZ_TO_AM(CFG_MAILBOX_SIZE) & ~0x80000000) |\
			 ORxG_ACS_DIV2				     |\
			 ORxG_SCY_15_CLK			     |\
			 ORxG_TRLX				     |\
			 ORxG_EHTR)

/*-----------------------------------------------------------------------
 * BR6 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR6 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 6 - I/O select
 *
 */

/* BR6 is configured as follows:
 *
 *     - Base address of 0xE0000000
 *     - 16 bit port size
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - GPCM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR6_PRELIM	((CFG_IOSELECT_BASE & BRx_BA_MSK) |\
			   BRx_PS_16			  |\
			   BRx_MS_GPCM_P		  |\
			   BRx_V)

/* OR6 is configured as follows
 *     - buffer control enabled
 *     - chip select negated normally
 *     - CS output 1/2 clock after address
 *     - 15 wait states
 *     - *PSDVAL is generated internally by the memory controller
 *	 unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *	 initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *	 current bank and the next access.
 */

#define CFG_OR6_PRELIM (MEG_TO_AM(CFG_IOSELECT_SIZE) |\
			 ORxG_ACS_DIV2		     |\
			 ORxG_SCY_15_CLK	     |\
			 ORxG_TRLX		     |\
			 ORxG_EHTR)


/*-----------------------------------------------------------------------
 * BR7 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR7 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 7 - LEDs and switches
 *
 *  LEDs     are at 0x00001 (write only)
 *  switches are at 0x00001 (read only)
 */
#ifdef CFG_LED_BASE

/* BR7 is configured as follows:
 *
 *     - Base address of 0xA0000000
 *     - 8 bit port size
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - GPCM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR7_PRELIM	((CFG_LED_BASE & BRx_BA_MSK)	 |\
			   BRx_PS_8			 |\
			   BRx_DECC_NONE		 |\
			   BRx_MS_GPCM_P		 |\
			   BRx_V)

/* OR7 is configured as follows:
 *
 *     - 1 byte
 *     - *BCTL0 is asserted upon access to the current memory bank
 *     - *CW / *WE are negated a quarter of a clock earlier
 *     - *CS is output at the same time as the address lines
 *     - Uses a clock cycle length of 15
 *     - *PSDVAL is generated internally by the memory controller
 *	 unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *	 initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *	 current bank and the next access.
 */
#define CFG_OR7_PRELIM	(ORxG_AM_MSK		       |\
			 ORxG_CSNT		       |\
			 ORxG_ACS_DIV1		       |\
			 ORxG_SCY_15_CLK	       |\
			 ORxG_TRLX		       |\
			 ORxG_EHTR)
#endif /* CFG_LED_BASE */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM	0x02	/* Software reboot		     */

#endif	/* __CONFIG_H */
