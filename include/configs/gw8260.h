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
 * Jay Monkman <jmonkman@adventnetworks.com>
 *
 * (C) Copyright 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Oliver Brown <obrown@adventnetworks.com>
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

/*********************************************************************/
/* DESCRIPTION:
 *   This file contains the board configuartion for the GW8260 board.
 *
 * MODULE DEPENDENCY:
 *   None
 *
 * RESTRICTIONS/LIMITATIONS:
 *   None
 *
 * Copyright (c) 2001, Advent Networks, Inc.
 */
/*********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/* Enable debug prints */
#undef DEBUG                  /* General debug */
#undef DEBUG_BOOTP_EXT        /* Debug received vendor fields */

/* What is the oscillator's (UX2) frequency in Hz? */
#define CONFIG_8260_CLKIN  (66 * 1000 * 1000)

/*-----------------------------------------------------------------------
 * MODCK_H & MODCLK[1-3] - Ref: Section 9.2 in MPC8206 User Manual
 *-----------------------------------------------------------------------
 * What should MODCK_H be? It is dependent on the oscillator
 * frequency, MODCK[1-3], and desired CPM and core frequencies.
 * Here are some example values (all frequencies are in MHz):
 *
 * MODCK_H   MODCK[1-3]  Osc    CPM    Core  S2-6   S2-7   S2-8
 * -------   ----------  ---    ---    ----  -----  -----  -----
 * 0x5       0x5     66 133     133    Open  Close  Open
 * 0x5       0x6     66 133     166    Open  Open   Close
 * 0x5       0x7     66 133     200    Open  Open   Open
 * 0x6       0x0     66 133     233    Close Close  Close
 * 0x6       0x1     66 133     266    Close Close  Open
 * 0x6       0x2     66 133     300    Close Open   Close
 */
#define CFG_SBC_MODCK_H 0x05

/* Define this if you want to boot from 0x00000100. If you don't define
 * this, you will need to program the bootloader to 0xfff00000, and
 * get the hardware reset config words at 0xfe000000. The simplest
 * way to do that is to program the bootloader at both addresses.
 * It is suggested that you just let U-Boot live at 0x00000000.
 */
#define CFG_SBC_BOOT_LOW 1

/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain TEXT_BASE from board/sbc8260/config.mk
 * The main FLASH is whichever is connected to *CS0. U-Boot expects
 * this to be the SIMM.
 */
#define CFG_FLASH0_BASE 0x40000000
#define CFG_FLASH0_SIZE 8

/* Define CFG_FLASH_CHECKSUM to enable flash checksum during boot.
 * Note: the 'flashchecksum' environment variable must also be set to 'y'.
 */
#define CFG_FLASH_CHECKSUM

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?
 */
#define CFG_SDRAM0_BASE 0x00000000
#define CFG_SDRAM0_SIZE 64

/*
 * DRAM tests
 *   CFG_DRAM_TEST - enables the following tests.
 *
 *   CFG_DRAM_TEST_DATA - Enables test for shorted or open data lines
 *                        Environment variable 'test_dram_data' must be
 *                        set to 'y'.
 *   CFG_DRAM_TEST_DATA - Enables test to verify that each word is uniquely
 *                        addressable. Environment variable
 *                        'test_dram_address' must be set to 'y'.
 *   CFG_DRAM_TEST_WALK - Enables test a 64-bit walking ones pattern test.
 *                        This test takes about 6 minutes to test 64 MB.
 *                        Environment variable 'test_dram_walk' must be
 *                        set to 'y'.
 */
#define CFG_DRAM_TEST
#if defined(CFG_DRAM_TEST)
#define CFG_DRAM_TEST_DATA
#define CFG_DRAM_TEST_ADDRESS
#define CFG_DRAM_TEST_WALK
#endif /* CFG_DRAM_TEST */

/*
 * GW8260 with 16 MB DIMM:
 *
 *     0x0000 0000     Exception Vector code, 8k
 *           :
 *     0x0000 1FFF
 *     0x0000 2000     Free for Application Use
 *           :
 *           :
 *
 *           :
 *           :
 *     0x00F5 FF30     Monitor Stack (Growing downward)
 *                     Monitor Stack Buffer (0x80)
 *     0x00F5 FFB0     Board Info Data
 *     0x00F6 0000     Malloc Arena
 *           :          CFG_ENV_SECT_SIZE, 256k
 *           :          CFG_MALLOC_LEN,    128k
 *     0x00FC 0000     RAM Copy of Monitor Code
 *           :              CFG_MONITOR_LEN,   256k
 *     0x00FF FFFF     [End of RAM], CFG_SDRAM_SIZE - 1
 */

/*
 * GW8260 with 64 MB DIMM:
 *
 *     0x0000 0000     Exception Vector code, 8k
 *           :
 *     0x0000 1FFF
 *     0x0000 2000     Free for Application Use
 *           :
 *           :
 *
 *           :
 *           :
 *     0x03F5 FF30     Monitor Stack (Growing downward)
 *                     Monitor Stack Buffer (0x80)
 *     0x03F5 FFB0     Board Info Data
 *     0x03F6 0000     Malloc Arena
 *           :          CFG_ENV_SECT_SIZE, 256k
 *           :          CFG_MALLOC_LEN,    128k
 *     0x03FC 0000     RAM Copy of Monitor Code
 *           :              CFG_MONITOR_LEN,   256k
 *     0x03FF FFFF     [End of RAM], CFG_SDRAM_SIZE - 1
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
 */
#define CONFIG_CONS_ON_SMC  1   /* define if console on SMC */
#undef  CONFIG_CONS_ON_SCC      /* define if console on SCC */
#undef  CONFIG_CONS_NONE        /* define if console on neither */
#define CONFIG_CONS_INDEX   1   /* which SMC/SCC channel for console */

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CFG_CMD_NET must be removed
 * from CONFIG_COMMANDS to remove support for networking.
 */

#undef  CONFIG_ETHER_ON_SCC
#define CONFIG_ETHER_ON_FCC
#undef  CONFIG_ETHER_NONE       /* define if ethernet on neither */

#ifdef  CONFIG_ETHER_ON_SCC
#define CONFIG_ETHER_INDEX  1   /* which SCC/FCC channel for ethernet */
#endif  /* CONFIG_ETHER_ON_SCC */

#ifdef  CONFIG_ETHER_ON_FCC
#define CONFIG_ETHER_INDEX  2   /* which SCC/FCC channel for ethernet */
#define CONFIG_MII              /* MII PHY management           */
#define CONFIG_BITBANGMII       /* bit-bang MII PHY management  */
/*
 * Port pins used for bit-banged MII communictions (if applicable).
 */
#define MDIO_PORT   2       /* Port C */
#define MDIO_ACTIVE    (iop->pdir |=  0x00400000)
#define MDIO_TRISTATE  (iop->pdir &= ~0x00400000)
#define MDIO_READ     ((iop->pdat &  0x00400000) != 0)

#define MDIO(bit)   if(bit) iop->pdat |=  0x00400000; \
	    else            iop->pdat &= ~0x00400000

#define MDC(bit)    if(bit) iop->pdat |=  0x00200000; \
	    else    iop->pdat &= ~0x00200000

#define MIIDELAY    udelay(1)
#endif  /* CONFIG_ETHER_ON_FCC */

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - Select bus for bd/buffers (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#elif defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 3)

/*
 * - Rx-CLK is CLK15
 * - Tx-CLK is CLK16
 * - Select bus for bd/buffers (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC3|CMXFCR_RF3CS_MSK|CMXFCR_TF3CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16)
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_ON_FCC, CONFIG_ETHER_INDEX */

/* Define this to reserve an entire FLASH sector (256 KB) for
 * environment variables. Otherwise, the environment will be
 * put in the same sector as U-Boot, and changing variables
 * will erase U-Boot temporarily
 */
#define CFG_ENV_IN_OWN_SECT

/* Define to allow the user to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* What should the console's baud rate be? */
#define CONFIG_BAUDRATE     115200

/* Ethernet MAC address - This is set to all zeros to force an
 *                        an error if we use BOOTP without setting
 *                        the MAC address
 */
#define CONFIG_ETHADDR      00:00:00:00:00:00

/* Set to a positive value to delay for running BOOTCOMMAND */
#define CONFIG_BOOTDELAY    5   /* autoboot after 5 seconds */

/* Be selective on what keys can delay or stop the autoboot process
 *     To stop  use: " "
 */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT  "Autobooting in %d seconds, press \" \" to stop\n"
#define CONFIG_AUTOBOOT_STOP_STR    " "
#undef  CONFIG_AUTOBOOT_DELAY_STR
#define DEBUG_BOOTKEYS      0

/* Add support for a few extra bootp options like:
 *  - File size
 *  - DNS
 */
#define CONFIG_BOOTP_MASK   (CONFIG_BOOTP_DEFAULT | \
			     CONFIG_BOOTP_BOOTFILESIZE | \
			     CONFIG_BOOTP_DNS)

/* undef this to save memory */
#define CFG_LONGHELP

/* Monitor Command Prompt */
#define CFG_PROMPT      "=> "

/* What U-Boot subsytems do you want enabled? */
#define CONFIG_COMMANDS     (((CONFIG_CMD_DFL & ~(CFG_CMD_KGDB))) | \
			       CFG_CMD_BEDBUG  | \
			       CFG_CMD_ELF | \
			       CFG_CMD_ASKENV  | \
			       CFG_CMD_ECHO    | \
			       CFG_CMD_REGINFO | \
			       CFG_CMD_IMMAP   | \
			       CFG_CMD_MII)

/* Where do the internal registers live? */
#define CFG_IMMR        0xf0000000

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* What is the address of IO controller */
#define CFG_IO_BASE 0xe0000000

/*****************************************************************************
 *
 * You should not have to modify any of the following settings
 *
 *****************************************************************************/

#define CONFIG_MPC8260      1   /* This is an MPC8260 CPU   */
#define CONFIG_GW8260       1   /* on an GW8260 Board  */
#define CONFIG_CPM2		1	/* Has a CPM2 */

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CBSIZE        1024    /* Console I/O Buffer Size       */
#else
#  define CFG_CBSIZE        256     /* Console I/O Buffer Size       */
#endif

/* Print Buffer Size */
#define CFG_PBSIZE    (CFG_CBSIZE + sizeof(CFG_PROMPT)+16)

#define CFG_MAXARGS     8          /* max number of command args   */

#define CFG_BARGSIZE    CFG_CBSIZE /* Boot Argument Buffer Size    */

/* Convert clocks to MHZ when passing board info to kernel.
 * This must be defined for eariler 2.4 kernels (~2.4.4).
 */
#define CONFIG_CLOCKS_IN_MHZ

#define CFG_LOAD_ADDR   0x100000 /* default load address */
#define CFG_HZ          1000     /* decrementer freq: 1 ms ticks */


/* memtest works from the end of the exception vector table
 * to the end of the DRAM less monitor and malloc area
 */
#define CFG_MEMTEST_START   0x2000

#define CFG_STACK_USAGE     0x10000 /* Reserve 64k for the stack usage */

#define CFG_MEM_END_USAGE   ( CFG_MONITOR_LEN \
			    + CFG_MALLOC_LEN \
			    + CFG_ENV_SECT_SIZE \
			    + CFG_STACK_USAGE )

#define CFG_MEMTEST_END     ( CFG_SDRAM_SIZE * 1024 * 1024 \
			    - CFG_MEM_END_USAGE )

/* valid baudrates */
#define CFG_BAUDRATE_TABLE  { 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_FLASH_BASE  CFG_FLASH0_BASE
#define CFG_FLASH_SIZE  CFG_FLASH0_SIZE
#define CFG_SDRAM_BASE  CFG_SDRAM0_BASE
#define CFG_SDRAM_SIZE  CFG_SDRAM0_SIZE

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */
#if defined(CFG_SBC_BOOT_LOW)
#  define  CFG_SBC_HRCW_BOOT_FLAGS  (HRCW_CIP | HRCW_BMS)
#else
#  define  CFG_SBC_HRCW_BOOT_FLAGS  (0)
#endif /* defined(CFG_SBC_BOOT_LOW) */

/* get the HRCW ISB field from CFG_IMMR */
#define CFG_SBC_HRCW_IMMR   ( ((CFG_IMMR & 0x10000000) >> 10) | \
		  ((CFG_IMMR & 0x01000000) >>  7) | \
		  ((CFG_IMMR & 0x00100000) >>  4) )

#define CFG_HRCW_MASTER     ( HRCW_BPS11                | \
		  HRCW_DPPC11               | \
		  CFG_SBC_HRCW_IMMR         | \
		  HRCW_MMR00                | \
		  HRCW_LBPC11               | \
		  HRCW_APPC10               | \
		  HRCW_CS10PC00             | \
		  (CFG_SBC_MODCK_H & HRCW_MODCK_H1111)  | \
		  CFG_SBC_HRCW_BOOT_FLAGS )

/* no slaves */
#define CFG_HRCW_SLAVE1     0
#define CFG_HRCW_SLAVE2     0
#define CFG_HRCW_SLAVE3     0
#define CFG_HRCW_SLAVE4     0
#define CFG_HRCW_SLAVE5     0
#define CFG_HRCW_SLAVE6     0
#define CFG_HRCW_SLAVE7     0

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR    CFG_IMMR
#define CFG_INIT_RAM_END     0x4000  /* End of used area in DPRAM    */
#define CFG_GBL_DATA_SIZE   128 /* bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET   CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 * Note also that the logic that sets CFG_RAMBOOT is platform dependent.
 */
#define CFG_MONITOR_BASE    CFG_FLASH0_BASE

#define CFG_MONITOR_LEN     (256 * 1024) /* Reserve 256 kB for Monitor   */
#define CFG_MALLOC_LEN      (128 * 1024) /* Reserve 128 kB for malloc()  */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ       (8 * 1024 * 1024) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS   1    /* max number of memory banks        */
#define CFG_MAX_FLASH_SECT    32   /* max number of sectors on one chip */

#define CFG_FLASH_ERASE_TOUT  8000 /* Timeout for Flash Erase (in ms)   */
#define CFG_FLASH_WRITE_TOUT  1    /* Timeout for Flash Write (in ms)   */

#define CFG_ENV_IS_IN_FLASH   1

#ifdef CFG_ENV_IN_OWN_SECT
#  define CFG_ENV_ADDR        (CFG_MONITOR_BASE +  (256 * 1024))
#  define CFG_ENV_SECT_SIZE   (256 * 1024)
#else
#  define CFG_ENV_SIZE        (16 * 1024)/* Size of Environment Sector  */
#  define CFG_ENV_ADD  ((CFG_MONITOR_BASE + CFG_MONITOR_LEN) - CFG_ENV_SIZE)
#  define CFG_ENV_SECT_SIZE (256 * 1024)/* see README - env sect real size  */
#endif /* CFG_ENV_IN_OWN_SECT */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE  32      /* For MPC8260 CPU */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
# define CFG_CACHELINE_SHIFT    5   /* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers            2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT   (HID0_ICE  |\
			 HID0_DCE  |\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)

#define CFG_HID0_FINAL  (HID0_ICE  |\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#define CFG_HID2    0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register
 *-----------------------------------------------------------------------
 */
#define CFG_RMR     0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                           4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR     (BCR_ETM)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                 4-31
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR  (SIUMCR_DPPC11  |\
		     SIUMCR_L2CPC00 |\
		     SIUMCR_APPC10  |\
		     SIUMCR_MMR00)


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control                11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#define CFG_SYPCR   (SYPCR_SWTC |\
		     SYPCR_BMT  |\
		     SYPCR_PBME |\
		     SYPCR_LBME |\
		     SYPCR_SWRI |\
		     SYPCR_SWP)

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control             4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC (TMCNTSC_SEC |\
		     TMCNTSC_ALR |\
		     TMCNTSC_TCF |\
		     TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control         4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR   (PISCR_PS  |\
		     PISCR_PTF |\
		     PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control                           9-8
 *-----------------------------------------------------------------------
 */
#define CFG_SCCR    0

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                 13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR    0

/*
 * Initialize Memory Controller:
 *
 * Bank Bus   Machine PortSz  Device
 * ---- ---   ------- ------  ------
 *  0   60x   GPCM    32 bit  FLASH (SIMM - 4MB)
 *  1   60x   GPCM    32 bit  unused
 *  2   60x   SDRAM   64 bit  SDRAM (DIMM - 16MB or 64MB)
 *  3   60x   SDRAM   64 bit  unused
 *  4   Local GPCM     8 bit  IO    (on board - 64k)
 *  5   60x   GPCM     8 bit  unused
 *  6   60x   GPCM     8 bit  unused
 *  7   60x   GPCM     8 bit  unused
 *
 */

/*-----------------------------------------------------------------------
 * BR0 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR0 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 0,1 - FLASH SIMM
 *
 * This expects the FLASH SIMM to be connected to *CS0
 * It consists of 4 AM29F016D parts.
 *
 * Note: For the 8 MB SIMM, *CS1 is unused.
 */

/* BR0 is configured as follows:
 *
 *     - Base address of 0x40000000
 *     - 32 bit port size
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - GPCM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR0_PRELIM  ((CFG_FLASH0_BASE & BRx_BA_MSK) |\
			  BRx_PS_32                     |\
			  BRx_MS_GPCM_P                 |\
			  BRx_V)

/* OR0 is configured as follows:
 *
 *     - 8 MB
 *     - *BCTL0 is asserted upon access to the current memory bank
 *     - *CW / *WE are negated a quarter of a clock earlier
 *     - *CS is output at the same time as the address lines
 *     - Uses a clock cycle length of 5
 *     - *PSDVAL is generated internally by the memory controller
 *       unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *       initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *       current bank and the next access.
 */
#define CFG_OR0_PRELIM  (MEG_TO_AM(CFG_FLASH0_SIZE) |\
			 ORxG_CSNT          |\
			 ORxG_ACS_DIV1      |\
			 ORxG_SCY_5_CLK     |\
			 ORxG_TRLX          |\
			 ORxG_EHTR)

/*-----------------------------------------------------------------------
 * BR2 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR2 - Option Register
 *     Ref: Section 10.3.2 on page 10-16
 *-----------------------------------------------------------------------
 */

/* Bank 2 - SDRAM DIMM
 *
 *     16MB DIMM: P/N
 *     64MB DIMM: P/N  1W-8864X8-4-P1-EST or
 *                     MT4LSDT864AG-10EB1 (Micron)
 *
 * Note: *CS3 is unused for this DIMM
 */

/* With a 16 MB or 64 MB DIMM, the BR2 is configured as follows:
 *
 *     - Base address of 0x00000000
 *     - 64 bit port size (60x bus only)
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - SDRAM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR2_PRELIM  ((CFG_SDRAM0_BASE & BRx_BA_MSK) |\
			  BRx_PS_64          |\
			  BRx_MS_SDRAM_P     |\
			  BRx_V)

/* With a 16 MB DIMM, the OR2 is configured as follows:
 *
 *     - 16 MB
 *     - 2 internal banks per device
 *     - Row start address bit is A9 with PSDMR[PBI] = 0
 *     - 11 row address lines
 *     - Back-to-back page mode
 *     - Internal bank interleaving within save device enabled
 */
#if (CFG_SDRAM0_SIZE == 16)
#define CFG_OR2_PRELIM  (MEG_TO_AM(CFG_SDRAM0_SIZE) |\
			 ORxS_BPD_2         |\
			 ORxS_ROWST_PBI0_A9 |\
			 ORxS_NUMR_11)

/* With a 16 MB DIMM, the PSDMR is configured as follows:
 *
 *     - Page Based Interleaving,
 *     - Refresh Enable,
 *     - Address Multiplexing where A5 is output on A14 pin
 *       (A6 on A15, and so on),
 *     - use address pins A16-A18 as bank select,
 *     - A9 is output on SDA10 during an ACTIVATE command,
 *     - earliest timing for ACTIVATE command after REFRESH command is 7 clocks,
 *     - earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 *       is 3 clocks,
 *     - earliest timing for READ/WRITE command after ACTIVATE command is
 *       2 clocks,
 *     - earliest timing for PRECHARGE after last data was read is 1 clock,
 *     - earliest timing for PRECHARGE after last data was written is 1 clock,
 *     - CAS Latency is 2.
 */

/*-----------------------------------------------------------------------
 * PSDMR - 60x Bus SDRAM Mode Register
 *     Ref: Section 10.3.3 on page 10-21
 *-----------------------------------------------------------------------
 */
#define CFG_PSDMR   (PSDMR_RFEN       |\
		     PSDMR_SDAM_A14_IS_A5 |\
		     PSDMR_BSMA_A16_A18   |\
		     PSDMR_SDA10_PBI0_A9  |\
		     PSDMR_RFRC_7_CLK     |\
		     PSDMR_PRETOACT_3W    |\
		     PSDMR_ACTTORW_2W     |\
		     PSDMR_LDOTOPRE_1C    |\
		     PSDMR_WRC_1C         |\
		     PSDMR_CL_2)
#endif /* (CFG_SDRAM0_SIZE == 16) */

/* With a 64 MB DIMM, the OR2 is configured as follows:
 *
 *     - 64 MB
 *     - 4 internal banks per device
 *     - Row start address bit is A8 with PSDMR[PBI] = 0
 *     - 12 row address lines
 *     - Back-to-back page mode
 *     - Internal bank interleaving within save device enabled
 */
#if (CFG_SDRAM0_SIZE == 64)
#define CFG_OR2_PRELIM  (MEG_TO_AM(CFG_SDRAM0_SIZE) |\
	     ORxS_BPD_4         |\
	     ORxS_ROWST_PBI0_A8     |\
	     ORxS_NUMR_12)

/* With a 64 MB DIMM, the PSDMR is configured as follows:
 *
 *     - Page Based Interleaving,
 *     - Refresh Enable,
 *     - Address Multiplexing where A5 is output on A14 pin
 *       (A6 on A15, and so on),
 *     - use address pins A14-A16 as bank select,
 *     - A9 is output on SDA10 during an ACTIVATE command,
 *     - earliest timing for ACTIVATE command after REFRESH command is 7 clocks,
 *     - earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 *       is 3 clocks,
 *     - earliest timing for READ/WRITE command after ACTIVATE command is
 *       2 clocks,
 *     - earliest timing for PRECHARGE after last data was read is 1 clock,
 *     - earliest timing for PRECHARGE after last data was written is 1 clock,
 *     - CAS Latency is 2.
 */

/*-----------------------------------------------------------------------
 * PSDMR - 60x Bus SDRAM Mode Register
 *     Ref: Section 10.3.3 on page 10-21
 *-----------------------------------------------------------------------
 */
#define CFG_PSDMR   (PSDMR_RFEN       |\
		     PSDMR_SDAM_A14_IS_A5 |\
		     PSDMR_BSMA_A14_A16   |\
		     PSDMR_SDA10_PBI0_A9  |\
		     PSDMR_RFRC_7_CLK     |\
		     PSDMR_PRETOACT_3W    |\
		     PSDMR_ACTTORW_2W     |\
		     PSDMR_LDOTOPRE_1C    |\
		     PSDMR_WRC_1C         |\
		     PSDMR_CL_2)
#endif  /* (CFG_SDRAM0_SIZE == 64) */

#define CFG_PSRT    0x0e
#define CFG_MPTPR   MPTPR_PTP_DIV32


/*-----------------------------------------------------------------------
 * BR4 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR4 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */
/* Bank 4 - Onboard Memory Mapped IO controller
 *
 * This expects the onboard IO controller to connected to *CS4 and
 * the local bus.
 *     - Base address of 0xe0000000
 *     - 8 bit port size (local bus only)
 *     - Read and write access
 *     - GPCM local bus
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 *     - extended hold time
 *     - 11 wait states
 */

#ifdef CFG_IO_BASE
#  define CFG_BR4_PRELIM  ((CFG_IO_BASE & BRx_BA_MSK)  |\
			    BRx_PS_8                   |\
			    BRx_MS_GPCM_L              |\
			    BRx_V)

#  define CFG_OR4_PRELIM   (ORxG_AM_MSK                |\
			    ORxG_SCY_11_CLK            |\
			    ORxG_EHTR)
#endif /* CFG_IO_BASE */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD   0x01    /* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM   0x02    /* Software reboot           */

#endif  /* __CONFIG_H */
