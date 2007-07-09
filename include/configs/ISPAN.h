/*
 * Copyright (C) 2004 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Support for Interphase iSPAN Communications Controllers
 * (453x and others). Tested on 4532.
 *
 * Derived from iSPAN 4539 port (iphase4539) by
 * Wolfgang Grandegger <wg@denx.de>
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

#define CONFIG_MPC8260			/* This is an MPC8260 CPU               */
#define CONFIG_ISPAN			/* ...on one of Interphase iSPAN boards */
#define CONFIG_CPM2		1	/* Has a CPM2 */

/*-----------------------------------------------------------------------
 * Select serial console configuration
 *
 * If either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * If CONFIG_CONS_NONE is defined, then the serial console routines must be
 * defined elsewhere (for example, on the cogent platform, there are serial
 * ports on the motherboard which are used for the serial console - see
 * cogent/cma101/serial.[ch]).
 */
#define	CONFIG_CONS_ON_SMC		/* Define if console on SMC		*/
#undef	CONFIG_CONS_ON_SCC		/* Define if console on SCC		*/
#undef	CONFIG_CONS_NONE		/* Define if console on something else	*/
#define CONFIG_CONS_INDEX	1	/* Which serial channel for console	*/

/*-----------------------------------------------------------------------
 * Select Ethernet configuration
 *
 * If either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC).
 *
 * If CONFIG_ETHER_NONE is defined, then either the Ethernet routines must
 * be defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* Define if Ethernet on SCC		*/
#define CONFIG_ETHER_ON_FCC		/* Define if Ethernet on FCC		*/
#undef	CONFIG_ETHER_NONE		/* Define if Ethernet on something else */
#define CONFIG_ETHER_INDEX	3	/* Which channel for Ethernrt		*/

#ifdef CONFIG_ETHER_ON_FCC

#if CONFIG_ETHER_INDEX == 3

#define CFG_PHY_ADDR		0
#define CFG_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK14 | CMXFCR_TF3CS_CLK16)
#define CFG_CMXFCR_MASK		(CMXFCR_FC3 | CMXFCR_RF3CS_MSK | CMXFCR_TF3CS_MSK)

#endif /* CONFIG_ETHER_INDEX == 3 */

#define CFG_CPMFCR_RAMTYPE	0
#define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#define CONFIG_MII				/* MII PHY management		*/
#define CONFIG_BITBANGMII			/* Bit-bang MII PHY management	*/
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT		3		/* Port D */

#define CFG_MDIO_PIN		0x00040000	/* PD13 */
#define CFG_MDC_PIN		0x00080000	/* PD12 */

#define MDIO_ACTIVE		(iop->pdir |=  CFG_MDIO_PIN)
#define MDIO_TRISTATE		(iop->pdir &= ~CFG_MDIO_PIN)
#define MDIO_READ		((iop->pdat &  CFG_MDIO_PIN) != 0)

#define MDIO(bit)		if(bit) iop->pdat |=  CFG_MDIO_PIN; \
				else	iop->pdat &= ~CFG_MDIO_PIN

#define MDC(bit)		if(bit) iop->pdat |=  CFG_MDC_PIN; \
				else	iop->pdat &= ~CFG_MDC_PIN

#define MIIDELAY		udelay(1)

#endif /* CONFIG_ETHER_ON_FCC */

#define CONFIG_8260_CLKIN	65536000	/* in Hz */
#define CONFIG_BAUDRATE		38400


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO


#define CONFIG_BOOTDELAY	5		/* autoboot after 5 seconds	*/
#define CONFIG_BOOTCOMMAND	"bootm fe010000"	/* autoboot command	*/
#define CONFIG_BOOTARGS		"root=/dev/ram rw"

#define CONFIG_BZIP2		/* Include support for bzip2 compressed images  */
#undef	CONFIG_WATCHDOG		/* Disable platform specific watchdog		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt	*/
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CFG_LONGHELP				/* #undef to save memory	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)  /* Print Buffer Size */
#define CFG_MAXARGS		16		/* Max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x03B00000	/* 1 ... 59 MB in SDRAM		*/

#define CFG_LOAD_ADDR		0x100000	/* Default load address		*/

#define CFG_HZ			1000		/* Decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_RESET_ADDRESS	0x09900000

#define CONFIG_MISC_INIT_R			/* We need misc_init_r()	*/

/*-----------------------------------------------------------------------
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor   */
#ifdef CONFIG_BZIP2
#define CFG_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()    */
#else
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 KB for malloc()  */
#endif /* CONFIG_BZIP2 */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_BASE		0xFE000000
#define CFG_FLASH_CFI				/* The flash is CFI compatible  */
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver        */
#define CFG_MAX_FLASH_BANKS	1		/* Max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	142		/* Max num of sects on one chip */

/* Environment is in flash, there is little space left in Serial EEPROM */
#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000 	/* We use one complete sector	*/
#define CFG_ENV_SIZE		(CFG_ENV_SECT_SIZE)
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * If you change bits in the HRCW, you must also change the CFG_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CFG_SIUMCR.
 */
/* 0x1686B245 */
#define CFG_HRCW_MASTER (HRCW_EBM      | HRCW_BPS01       | HRCW_CIP    |\
			 HRCW_L2CPC10  | HRCW_ISB110                    |\
			 HRCW_BMS      | HRCW_MMR11       | HRCW_APPC10 |\
			 HRCW_CS10PC01 | HRCW_MODCK_H0101                \
			)
/* No slaves */
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
#define CFG_IMMR		0xF0F00000
#ifdef CFG_REV_B
#define CFG_DEFAULT_IMMR	0xFF000000
#endif /* CFG_REV_B */
/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x4000		/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128  /* Size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from flash	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU			*/

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers		2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT		(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#define CFG_HID0_FINAL		(HID0_ICE|HID0_IFEM|HID0_ABE)
#define CFG_HID2		0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register					 5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CFG_RMR			RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR			0xA01C0000

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR		0x42250000/* 0x4205C000 */

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				 4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined (CONFIG_WATCHDOG)
#define CFG_SYPCR		(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
				SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CFG_SYPCR		(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
				SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 *-----------------------------------------------------------------------
 */
#define CFG_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CFG_SCCR		SCCR_DFBRG01

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR		0

/*-----------------------------------------------------------------------
 * Init Memory Controller:
 *
 * Bank Bus	Machine PortSize                        Device
 * ---- ---	------- -----------------------------   ------
 *  0	60x	GPCM	 8 bit (Rev.B)/16 bit (Rev.D)   Flash
 *  1	60x	SDRAM	64 bit                          SDRAM
 *  2	Local	SDRAM	32 bit	                        SDRAM
 */
#define CFG_USE_FIRMWARE	/* If defined - do not initialise memory
				   controller, rely on initialisation
				   performed by the Interphase boot firmware.
				 */

#define CFG_OR0_PRELIM		0xFE000882
#ifdef CFG_REV_B
#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | BRx_PS_8  | BRx_V)
#else  /* Rev. D */
#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | BRx_PS_16 | BRx_V)
#endif /* CFG_REV_B */

#define CFG_MPTPR		0x7F00

/* Please note that 60x SDRAM MUST start at 0 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_60x_BR		0x00000041
#define CFG_60x_OR		0xF0002CD0
#define CFG_PSDMR		0x0049929A
#define CFG_PSRT		0x07

#define CFG_LSDRAM_BASE		0xF7000000
#define CFG_LOC_BR		0x00001861
#define CFG_LOC_OR		0xFF803280
#define CFG_LSDMR		0x8285A552
#define CFG_LSRT		0x07

#endif /* __CONFIG_H */
