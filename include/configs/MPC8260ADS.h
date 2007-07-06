/*
 * (C) Copyright 2001
 * Stuart Hughes <stuarth@lineo.com>
 * This file is based on similar values for other boards found in other
 * U-Boot config files, and some that I found in the mpc8260ads manual.
 *
 * Note: my board is a PILOT rev.
 * Note: the mpc8260ads doesn't come with a proper Ethernet MAC address.
 *
 * (C) Copyright 2003-2004 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 * Added support for SDRAM DIMMs SPD EEPROM, MII, JFFS2.
 * Ported to PQ2FADS-ZU and PQ2FADS-VR boards.
 * Ported to MPC8272ADS board.
 *
 * Copyright (c) 2005 MontaVista Software, Inc.
 * Vitaly Bordug <vbordug@ru.mvista.com>
 * Added support for PCI bridge on MPC8272ADS
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

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260ADS	1	/* Motorola PQ2 ADS family board */

#define CONFIG_CPM2		1	/* Has a CPM2 */

/*
 * Figure out if we are booting low via flash HRCW or high via the BCSR.
 */
#if (TEXT_BASE != 0xFFF00000)		/* Boot low (flash HRCW) */
#   define CFG_LOWBOOT		1
#endif

/* ADS flavours */
#define CFG_8260ADS		1	/* MPC8260ADS */
#define CFG_8266ADS		2	/* MPC8266ADS */
#define CFG_PQ2FADS		3	/* PQ2FADS-ZU or PQ2FADS-VR */
#define CFG_8272ADS		4	/* MPC8272ADS */

#ifndef CONFIG_ADSTYPE
#define CONFIG_ADSTYPE		CFG_8260ADS
#endif /* CONFIG_ADSTYPE */

#if CONFIG_ADSTYPE == CFG_8272ADS
#define CONFIG_MPC8272		1
#else
#define CONFIG_MPC8260		1
#endif /* CONFIG_ADSTYPE == CFG_8272ADS */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

/* allow serial and ethaddr to be overwritten */
#define CONFIG_ENV_OVERWRITE

/*
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
#undef	CONFIG_CONS_ON_SMC		/* define if console on SMC */
#define CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on something else */
#define CONFIG_CONS_INDEX	1	/* which serial channel for console */

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
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC   */
#define CONFIG_ETHER_ON_FCC		/* define if ether on FCC   */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */

#ifdef CONFIG_ETHER_ON_FCC

#define CONFIG_ETHER_INDEX	2	/* which SCC/FCC channel for ethernet */

#if   CONFIG_ETHER_INDEX == 1

# define CFG_PHY_ADDR		0
# define CFG_CMXFCR_VALUE	(CMXFCR_RF1CS_CLK11 | CMXFCR_TF1CS_CLK10)
# define CFG_CMXFCR_MASK	(CMXFCR_FC1 | CMXFCR_RF1CS_MSK | CMXFCR_TF1CS_MSK)

#elif CONFIG_ETHER_INDEX == 2

#if CONFIG_ADSTYPE == CFG_8272ADS	/* RxCLK is CLK15, TxCLK is CLK16 */
# define CFG_PHY_ADDR		3
# define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK15 | CMXFCR_TF2CS_CLK16)
#else					/* RxCLK is CLK13, TxCLK is CLK14 */
# define CFG_PHY_ADDR		0
# define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
#endif /* CONFIG_ADSTYPE == CFG_8272ADS */

# define CFG_CMXFCR_MASK	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)

#endif	/* CONFIG_ETHER_INDEX */

#define CFG_CPMFCR_RAMTYPE	0		/* BDs and buffers on 60x bus */
#define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)  /* Full duplex */

#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT	2		/* Port C */

#if CONFIG_ADSTYPE == CFG_8272ADS
#define CFG_MDIO_PIN	0x00002000	/* PC18 */
#define CFG_MDC_PIN	0x00001000	/* PC19 */
#else
#define CFG_MDIO_PIN	0x00400000	/* PC9	*/
#define CFG_MDC_PIN	0x00200000	/* PC10 */
#endif /* CONFIG_ADSTYPE == CFG_8272ADS */

#define MDIO_ACTIVE	(iop->pdir |=  CFG_MDIO_PIN)
#define MDIO_TRISTATE	(iop->pdir &= ~CFG_MDIO_PIN)
#define MDIO_READ	((iop->pdat &  CFG_MDIO_PIN) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  CFG_MDIO_PIN; \
			else	iop->pdat &= ~CFG_MDIO_PIN

#define MDC(bit)	if(bit) iop->pdat |=  CFG_MDC_PIN; \
			else	iop->pdat &= ~CFG_MDC_PIN

#define MIIDELAY	udelay(1)

#endif /* CONFIG_ETHER_ON_FCC */

#if CONFIG_ADSTYPE >= CFG_PQ2FADS
#undef CONFIG_SPD_EEPROM	/* On new boards, SDRAM is soldered */
#else
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#if defined(CONFIG_SPD_EEPROM) && !defined(CONFIG_SPD_ADDR)
#define CONFIG_SPD_ADDR		0x50
#endif
#endif /* CONFIG_ADSTYPE >= CFG_PQ2FADS */

/*PCI*/
#ifdef CONFIG_MPC8272
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_BOOTDELAY 0
#define CONFIG_PCI_SCAN_SHOW
#endif

#ifndef CONFIG_SDRAM_PBI
#define CONFIG_SDRAM_PBI	0 /* By default, use bank-based interleaving */
#endif

#ifndef CONFIG_8260_CLKIN
#if CONFIG_ADSTYPE >= CFG_PQ2FADS
#define CONFIG_8260_CLKIN	100000000	/* in Hz */
#else
#define CONFIG_8260_CLKIN	66000000	/* in Hz */
#endif
#endif

#define CONFIG_BAUDRATE		115200

/*
 * Command line configuration.
 */
#include <config_cmd_all.h>

#undef CONFIG_CMD_BEDBUG
#undef CONFIG_CMD_BMP
#undef CONFIG_CMD_BSP
#undef CONFIG_CMD_DATE
#undef CONFIG_CMD_DISPLAY
#undef CONFIG_CMD_DOC
#undef CONFIG_CMD_DTT
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_ELF
#undef CONFIG_CMD_EXT2
#undef CONFIG_CMD_FAT
#undef CONFIG_CMD_FDC
#undef CONFIG_CMD_FDOS
#undef CONFIG_CMD_HWFLOW
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_KGDB
#undef CONFIG_CMD_MMC
#undef CONFIG_CMD_NAND
#undef CONFIG_CMD_PCMCIA
#undef CONFIG_CMD_REISER
#undef CONFIG_CMD_SCSI
#undef CONFIG_CMD_SPI
#undef CONFIG_CMD_SNTP
#undef CONFIG_CMD_UNIVERSE
#undef CONFIG_CMD_USB
#undef CONFIG_CMD_VFD
#undef CONFIG_CMD_XIMG

#if CONFIG_ADSTYPE == CFG_8272ADS
    #undef CONFIG_CMD_SDRAM
    #undef CONFIG_CMD_I2C

#elif CONFIG_ADSTYPE >= CFG_PQ2FADS
    #undef CONFIG_CMD_SDRAM
    #undef CONFIG_CMD_I2C
    #undef CONFIG_CMD_PCI

#else
    #undef CONFIG_CMD_PCI

#endif /* CONFIG_ADSTYPE >= CFG_PQ2FADS */


#define CONFIG_BOOTDELAY	5		/* autoboot after 5 seconds */
#define CONFIG_BOOTCOMMAND	"bootm fff80000"	/* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2"

#if defined(CONFIG_CMD_KGDB) || (CONFIG_COMMANDS & CFG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which serial channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
#endif

#define CONFIG_BZIP2	/* include support for bzip2 compressed images */
#undef	CONFIG_WATCHDOG /* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB) || (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE	256			/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16			/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x400000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CFG_FLASH_BASE		0xff800000
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	32	/* max num of sects on one chip */
#define CFG_FLASH_SIZE		8
#define CFG_FLASH_ERASE_TOUT	8000	/* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT	5	/* Timeout for Flash Write (in ms)    */
#define CFG_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CFG_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CFG_FLASH_PROTECTION		/* "Real" (hardware) sectors protection */

/*
 * JFFS2 partitions
 *
 * Note: fake mtd_id used, no linux mtd map file
 */
#define MTDIDS_DEFAULT		"nor0=mpc8260ads-0"
#define MTDPARTS_DEFAULT	"mtdparts=mpc8260ads-0:-@1m(jffs2)"
#define CFG_JFFS2_SORT_FRAGMENTS

/* this is stuff came out of the Motorola docs */
#ifndef CFG_LOWBOOT
#define CFG_DEFAULT_IMMR	0x0F010000
#endif

#define CFG_IMMR		0xF0000000
#define CFG_BCSR		0xF4500000
#if CONFIG_ADSTYPE == CFG_8272ADS
#define CFG_PCI_INT		0xF8200000
#endif
#define CFG_SDRAM_BASE		0x00000000
#define CFG_LSDRAM_BASE		0xFD000000

#define RS232EN_1		0x02000002
#define RS232EN_2		0x01000001
#define FETHIEN1		0x08000008
#define FETH1_RST		0x04000004
#define FETHIEN2		0x10000000
#define FETH2_RST		0x08000000
#define BCSR_PCI_MODE		0x01000000

#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#ifdef CFG_LOWBOOT
/* PQ2FADS flash HRCW = 0x0EB4B645 */
#define CFG_HRCW_MASTER (   ( HRCW_BPS11 | HRCW_CIP )			    |\
			    ( HRCW_L2CPC10 | HRCW_DPPC11 | HRCW_ISB100 )    |\
			    ( HRCW_BMS | HRCW_MMR11 | HRCW_LBPC01 | HRCW_APPC10 ) |\
			    ( HRCW_CS10PC01 | HRCW_MODCK_H0101 )	     \
			)
#else
/* PQ2FADS BCSR HRCW = 0x0CB23645 */
#define CFG_HRCW_MASTER (   ( HRCW_BPS11 | HRCW_CIP )			    |\
			    ( HRCW_L2CPC10 | HRCW_DPPC10 | HRCW_ISB010 )    |\
			    ( HRCW_BMS | HRCW_APPC10 )			    |\
			    ( HRCW_MODCK_H0101 )			     \
			)
#endif
/* no slaves */
#define CFG_HRCW_SLAVE1 0
#define CFG_HRCW_SLAVE2 0
#define CFG_HRCW_SLAVE3 0
#define CFG_HRCW_SLAVE4 0
#define CFG_HRCW_SLAVE5 0
#define CFG_HRCW_SLAVE6 0
#define CFG_HRCW_SLAVE7 0

#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM	0x02	/* Software reboot	     */

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#ifdef CONFIG_BZIP2
#define CFG_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#else
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 KB for malloc()	*/
#endif /* CONFIG_BZIP2 */

#ifndef CFG_RAMBOOT
#  define CFG_ENV_IS_IN_FLASH	1
#  define CFG_ENV_SECT_SIZE	0x40000
#  define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_ENV_SECT_SIZE)
#else
#  define CFG_ENV_IS_IN_NVRAM	1
#  define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
#  define CFG_ENV_SIZE		0x200
#endif /* CFG_RAMBOOT */

#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE )

#define CFG_HID2		0

#define CFG_SYPCR		0xFFFFFFC3
#define CFG_BCR			0x100C0000
#define CFG_SIUMCR		0x0A200000
#define CFG_SCCR		SCCR_DFBRG01
#define CFG_BR0_PRELIM		CFG_FLASH_BASE | 0x00001801
#define CFG_OR0_PRELIM		0xFF800876
#define CFG_BR1_PRELIM		CFG_BCSR | 0x00001801
#define CFG_OR1_PRELIM		0xFFFF8010

/*We need to configure chip select to use CPLD PCI IC on MPC8272ADS*/

#if CONFIG_ADSTYPE == CFG_8272ADS
#define CFG_BR3_PRELIM	(CFG_PCI_INT | 0x1801)	/* PCI interrupt controller */
#define CFG_OR3_PRELIM	0xFFFF8010
#endif

#define CFG_RMR			RMR_CSRE
#define CFG_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)
#define CFG_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)
#define CFG_RCCR		0

#if (CONFIG_ADSTYPE == CFG_8266ADS) || (CONFIG_ADSTYPE == CFG_8272ADS)
#undef CFG_LSDRAM_BASE		/* No local bus SDRAM on these boards */
#endif /* CONFIG_ADSTYPE == CFG_8266ADS */

#if CONFIG_ADSTYPE == CFG_PQ2FADS
#define CFG_OR2			0xFE002EC0
#define CFG_PSDMR		0x824B36A3
#define CFG_PSRT		0x13
#define CFG_LSDMR		0x828737A3
#define CFG_LSRT		0x13
#define CFG_MPTPR		0x2800
#elif CONFIG_ADSTYPE == CFG_8272ADS
#define CFG_OR2			0xFC002CC0
#define CFG_PSDMR		0x834E24A3
#define CFG_PSRT		0x13
#define CFG_MPTPR		0x2800
#else
#define CFG_OR2			0xFF000CA0
#define CFG_PSDMR		0x016EB452
#define CFG_PSRT		0x21
#define CFG_LSDMR		0x0086A522
#define CFG_LSRT		0x21
#define CFG_MPTPR		0x1900
#endif /* CONFIG_ADSTYPE == CFG_PQ2FADS */

#define CFG_RESET_ADDRESS	0x04400000

#if CONFIG_ADSTYPE == CFG_8272ADS

/* PCI Memory map (if different from default map */
#define CFG_PCI_SLV_MEM_LOCAL	CFG_SDRAM_BASE		/* Local base */
#define CFG_PCI_SLV_MEM_BUS		0x00000000		/* PCI base */
#define CFG_PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
				 PICMR_PREFETCH_EN)

/*
 * These are the windows that allow the CPU to access PCI address space.
 * All three PCI master windows, which allow the CPU to access PCI
 * prefetch, non prefetch, and IO space (see below), must all fit within
 * these windows.
 */

/*
 * Master window that allows the CPU to access PCI Memory (prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_MEM_LOCAL	0x80000000          /* Local base */
#define CFG_PCI_MSTR_MEM_BUS	0x80000000          /* PCI base   */
#define	CFG_CPU_PCI_MEM_START	PCI_MSTR_MEM_LOCAL
#define CFG_PCI_MSTR_MEM_SIZE	0x20000000          /* 512MB */
#define CFG_POCMR0_MASK_ATTRIB	(POCMR_MASK_512MB | POCMR_ENABLE | POCMR_PREFETCH_EN)

/*
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_MEMIO_LOCAL    0xA0000000          /* Local base */
#define CFG_PCI_MSTR_MEMIO_BUS      0xA0000000          /* PCI base   */
#define CFG_CPU_PCI_MEMIO_START     PCI_MSTR_MEMIO_LOCAL
#define CFG_PCI_MSTR_MEMIO_SIZE     0x20000000          /* 512MB */
#define CFG_POCMR1_MASK_ATTRIB      (POCMR_MASK_512MB | POCMR_ENABLE)

/*
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_IO_LOCAL       0xF6000000          /* Local base */
#define CFG_PCI_MSTR_IO_BUS         0x00000000          /* PCI base   */
#define CFG_CPU_PCI_IO_START        PCI_MSTR_IO_LOCAL
#define CFG_PCI_MSTR_IO_SIZE        0x02000000          /* 64MB */
#define CFG_POCMR2_MASK_ATTRIB      (POCMR_MASK_32MB | POCMR_ENABLE | POCMR_PCI_IO)


/* PCIBR0 - for PCI IO*/
#define CFG_PCI_MSTR0_LOCAL		CFG_PCI_MSTR_IO_LOCAL		/* Local base */
#define CFG_PCIMSK0_MASK		~(CFG_PCI_MSTR_IO_SIZE - 1U)	/* Size of window */
/* PCIBR1 - prefetch and non-prefetch regions joined together */
#define CFG_PCI_MSTR1_LOCAL		CFG_PCI_MSTR_MEM_LOCAL
#define CFG_PCIMSK1_MASK		~(CFG_PCI_MSTR_MEM_SIZE + CFG_PCI_MSTR_MEMIO_SIZE - 1U)

#endif /* CONFIG_ADSTYPE == CONFIG_8272ADS*/

#if CONFIG_ADSTYPE == CFG_8272ADS
#define CONFIG_HAS_ETH1
#endif

#endif /* __CONFIG_H */
