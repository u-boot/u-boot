/*
 * Copyright (C) 2003-2005 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * U-Boot configuration for Zephyr Engineering ZPC.1900 board.
 * This port was developed and tested on Revision C board.
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

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU      */
#define CONFIG_ZPC1900		1	/* ...on Zephyr ZPC.1900 board */
#define CPU_ID_STR		"MPC8265"
#define CONFIG_CPM2		1	/* Has a CPM2 */

/* Allow serial number (serial#) and MAC address (ethaddr) to be overwritten */
#define CONFIG_ENV_OVERWRITE

/*
 * Select serial console configuration
 *
 * If either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 */
#define	CONFIG_CONS_ON_SMC		/* Console is on SMC         */
#undef  CONFIG_CONS_ON_SCC		/* It's not on SCC           */
#undef	CONFIG_CONS_NONE		/* It's not on external UART */
#define CONFIG_CONS_INDEX	1	/* SMC1 is used for console  */

/*
 * Select ethernet configuration
 *
 * If either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected,
 * then CONFIG_ETHER_INDEX must be set to the channel number (1-4 for
 * SCC, 1-3 for FCC)
 *
 * If CONFIG_ETHER_NONE is defined, then either the ethernet routines
 * must be defined elsewhere (as for the console), or CFG_CMD_NET must
 * be removed from CONFIG_COMMANDS to remove support for networking.
 */
#undef	CONFIG_ETHER_ON_SCC		/* Ethernet is not on SCC */
#define CONFIG_ETHER_ON_FCC		/* Ethernet is on FCC     */
#undef	CONFIG_ETHER_NONE		/* No external Ethernet   */

#ifdef CONFIG_ETHER_ON_FCC

#define CONFIG_ETHER_INDEX	2	/* FCC2 is used for Ethernet */

#if (CONFIG_ETHER_INDEX == 2)
/*
 * - Rx clock is CLK13
 * - Tx clock is CLK14
 * - Select bus for bd/buffers (see 28-13)
 * - Full duplex
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_INDEX */

#define CONFIG_MII			/* MII PHY management        */
#define CONFIG_BITBANGMII		/* Bit-banged MDIO interface */
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT		2	/* Port C */
#define MDIO_ACTIVE		(iop->pdir |=  0x00400000)
#define MDIO_TRISTATE		(iop->pdir &= ~0x00400000)
#define MDIO_READ		((iop->pdat &  0x00400000) != 0)

#define MDIO(bit)		if(bit) iop->pdat |=  0x00400000; \
				else	iop->pdat &= ~0x00400000

#define MDC(bit)		if(bit) iop->pdat |=  0x00200000; \
				else	iop->pdat &= ~0x00200000

#define MIIDELAY		udelay(1)

#endif /* CONFIG_ETHER_ON_FCC */

#ifndef CONFIG_8260_CLKIN
#define CONFIG_8260_CLKIN	66666666	/* in Hz */
#endif

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


#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_BOOTCOMMAND	"dhcp;bootm"	/* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/nfs rw ip=:::::eth0:dhcp"

#if defined(CONFIG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which serial channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
#endif

#define CONFIG_BZIP2	/* include support for bzip2 compressed images */
#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size  */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size  */

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x03800000	/* 1 ... 56 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x400000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_SIZE		64

#define CFG_IMMR		0xF0000000
#define CFG_LSDRAM_BASE		0xFC000000
#define CFG_FLASH_BASE		0xFE000000
#define CFG_BCSR		0xFEA00000
#define CFG_EEPROM		0xFEB00000
#define CFG_FLSIMM_BASE		0xFF000000

#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_MAX_FLASH_BANKS	2	/* max num of flash banks	*/
#define CFG_MAX_FLASH_SECT	32	/* max num of sects on one chip */

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE, CFG_FLSIMM_BASE }

#define BCSR_PCI_MODE		0x01

#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/* Hard reset configuration word */
#define CFG_HRCW_MASTER		(HRCW_EBM | HRCW_BPS01| HRCW_CIP          |\
				 HRCW_L2CPC10 | HRCW_DPPC00 | HRCW_ISB100 |\
				 HRCW_BMS | HRCW_LBPC00 | HRCW_APPC10     |\
				 HRCW_MODCK_H0111                          \
				) /* 0x16848207 */
/* No slaves */
#define CFG_HRCW_SLAVE1 	0
#define CFG_HRCW_SLAVE2 	0
#define CFG_HRCW_SLAVE3 	0
#define CFG_HRCW_SLAVE4 	0
#define CFG_HRCW_SLAVE5 	0
#define CFG_HRCW_SLAVE6 	0
#define CFG_HRCW_SLAVE7 	0

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot                  */

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#if !defined(CFG_ENV_IS_IN_FLASH) && !defined(CFG_ENV_IS_IN_NVRAM)
#define CFG_ENV_IS_IN_NVRAM	1
#endif

#ifdef CFG_ENV_IS_IN_FLASH
#  define CFG_ENV_SECT_SIZE	0x10000
#  define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#else
#  define CFG_ENV_ADDR		(CFG_EEPROM + 0x400)
#  define CFG_ENV_SIZE		0x1000
#  define CFG_NVRAM_ACCESS_ROUTINE
#endif

#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CFG_HID0_INIT		(HID0_ICFI)
#define CFG_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE)

#define CFG_HID2		0

#define CFG_SIUMCR		0x42200000
#define CFG_SYPCR		0xFFFFFFC3
#define CFG_BCR			0x90000000
#define CFG_SCCR		SCCR_DFBRG01

#define CFG_RMR			RMR_CSRE
#define CFG_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)
#define CFG_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)
#define CFG_RCCR		0

#define CFG_PSDMR		/* 0x834DA43B */0x014DA43A
#define CFG_PSRT		0x0F/* 0x0C */
#define CFG_LSDMR		0x0085A562
#define CFG_LSRT		0x0F
#define CFG_MPTPR		0x4000

#define CFG_PSDRAM_BR		(CFG_SDRAM_BASE | 0x00000041)
#define CFG_PSDRAM_OR		0xFC0028C0
#define CFG_LSDRAM_BR		(CFG_LSDRAM_BASE | 0x00001861)
#define CFG_LSDRAM_OR		0xFF803480

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | 0x00000801)
#define CFG_OR0_PRELIM		0xFFE00856
#define CFG_BR5_PRELIM		(CFG_EEPROM | 0x00000801)
#define CFG_OR5_PRELIM		0xFFFF03F6
#define CFG_BR6_PRELIM		(CFG_FLSIMM_BASE | 0x00001801)
#define CFG_OR6_PRELIM		0xFF000856
#define CFG_BR7_PRELIM		(CFG_BCSR | 0x00000801)
#define CFG_OR7_PRELIM		0xFFFF83F6

#define CFG_RESET_ADDRESS	0xC0000000

#endif /* __CONFIG_H */
