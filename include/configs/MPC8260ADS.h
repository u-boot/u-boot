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
 * Copyright (C) Freescale Semiconductor, Inc. 2006-2009.
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
#   define CONFIG_SYS_LOWBOOT		1
#endif

/* ADS flavours */
#define CONFIG_SYS_8260ADS		1	/* MPC8260ADS */
#define CONFIG_SYS_8266ADS		2	/* MPC8266ADS */
#define CONFIG_SYS_PQ2FADS		3	/* PQ2FADS-ZU or PQ2FADS-VR */
#define CONFIG_SYS_8272ADS		4	/* MPC8272ADS */

#ifndef CONFIG_ADSTYPE
#define CONFIG_ADSTYPE		CONFIG_SYS_8260ADS
#endif /* CONFIG_ADSTYPE */

#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
#define CONFIG_MPC8272		1
#elif CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS
/*
 * Actually MPC8275, but the code is littered with ifdefs that
 * apply to both, or which use this ifdef to assume board-specific
 * details. :-(
 */
#define CONFIG_MPC8272		1
#else
#define CONFIG_MPC8260		1
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_8272ADS */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/
#define CONFIG_RESET_PHY_R	1	/* Call reset_phy()		*/

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
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC   */
#define CONFIG_ETHER_ON_FCC		/* define if ether on FCC   */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */

#ifdef CONFIG_ETHER_ON_FCC

#define CONFIG_ETHER_INDEX	2	/* which SCC/FCC channel for ethernet */

#if   CONFIG_ETHER_INDEX == 1

# define CONFIG_SYS_PHY_ADDR		0
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF1CS_CLK11 | CMXFCR_TF1CS_CLK10)
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC1 | CMXFCR_RF1CS_MSK | CMXFCR_TF1CS_MSK)

#elif CONFIG_ETHER_INDEX == 2

#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS	/* RxCLK is CLK15, TxCLK is CLK16 */
# define CONFIG_SYS_PHY_ADDR		3
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK15 | CMXFCR_TF2CS_CLK16)
#else					/* RxCLK is CLK13, TxCLK is CLK14 */
# define CONFIG_SYS_PHY_ADDR		0
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_8272ADS */

# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)

#endif	/* CONFIG_ETHER_INDEX */

#define CONFIG_SYS_CPMFCR_RAMTYPE	0		/* BDs and buffers on 60x bus */
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)  /* Full duplex */

#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT	2		/* Port C */
#define MDIO_DECLARE	volatile ioport_t *iop = ioport_addr ( \
				(immap_t *) CONFIG_SYS_IMMR, MDIO_PORT )
#define MDC_DECLARE	MDIO_DECLARE

#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
#define CONFIG_SYS_MDIO_PIN	0x00002000	/* PC18 */
#define CONFIG_SYS_MDC_PIN	0x00001000	/* PC19 */
#else
#define CONFIG_SYS_MDIO_PIN	0x00400000	/* PC9	*/
#define CONFIG_SYS_MDC_PIN	0x00200000	/* PC10 */
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_8272ADS */

#define MDIO_ACTIVE	(iop->pdir |=  CONFIG_SYS_MDIO_PIN)
#define MDIO_TRISTATE	(iop->pdir &= ~CONFIG_SYS_MDIO_PIN)
#define MDIO_READ	((iop->pdat &  CONFIG_SYS_MDIO_PIN) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  CONFIG_SYS_MDIO_PIN; \
			else	iop->pdat &= ~CONFIG_SYS_MDIO_PIN

#define MDC(bit)	if(bit) iop->pdat |=  CONFIG_SYS_MDC_PIN; \
			else	iop->pdat &= ~CONFIG_SYS_MDC_PIN

#define MIIDELAY	udelay(1)

#endif /* CONFIG_ETHER_ON_FCC */

#if CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS
#undef CONFIG_SPD_EEPROM	/* On new boards, SDRAM is soldered */
#else
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

#if defined(CONFIG_SPD_EEPROM) && !defined(CONFIG_SPD_ADDR)
#define CONFIG_SPD_ADDR		0x50
#endif
#endif /* CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS */

/*PCI*/
#if CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_BOOTDELAY 0
#define CONFIG_PCI_SCAN_SHOW
#endif

#ifndef CONFIG_SDRAM_PBI
#define CONFIG_SDRAM_PBI	0 /* By default, use bank-based interleaving */
#endif

#ifndef CONFIG_8260_CLKIN
#if CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS
#define CONFIG_8260_CLKIN	100000000	/* in Hz */
#else
#define CONFIG_8260_CLKIN	66000000	/* in Hz */
#endif
#endif

#define CONFIG_BAUDRATE		115200

#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#if defined(CONFIG_OF_LIBFDT)
#define OF_TBCLK		(bd->bi_busfreq / 4)
#endif

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CDP
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_SDRAM

#undef CONFIG_CMD_XIMG

#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
    #undef CONFIG_CMD_SDRAM
    #undef CONFIG_CMD_I2C

#elif CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS
    #undef CONFIG_CMD_SDRAM
    #undef CONFIG_CMD_I2C

#else
    #undef CONFIG_CMD_PCI

#endif /* CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS */


#define CONFIG_BOOTDELAY	5		/* autoboot after 5 seconds */
#define CONFIG_BOOTCOMMAND	"bootm fff80000"	/* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2"

#if defined(CONFIG_CMD_KGDB)
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
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE	256			/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16			/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x400000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_SYS_FLASH_BASE		0xff800000
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	32	/* max num of sects on one chip */
#define CONFIG_SYS_FLASH_SIZE		8
#define CONFIG_SYS_FLASH_ERASE_TOUT	8000	/* Timeout for Flash Erase (in ms)    */
#define CONFIG_SYS_FLASH_WRITE_TOUT	5	/* Timeout for Flash Write (in ms)    */
#define CONFIG_SYS_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CONFIG_SYS_FLASH_PROTECTION		/* "Real" (hardware) sectors protection */

/*
 * JFFS2 partitions
 *
 * Note: fake mtd_id used, no linux mtd map file
 */
#define MTDIDS_DEFAULT		"nor0=mpc8260ads-0"
#define MTDPARTS_DEFAULT	"mtdparts=mpc8260ads-0:-@1m(jffs2)"
#define CONFIG_SYS_JFFS2_SORT_FRAGMENTS

/* this is stuff came out of the Motorola docs */
#ifndef CONFIG_SYS_LOWBOOT
#define CONFIG_SYS_DEFAULT_IMMR	0x0F010000
#endif

#define CONFIG_SYS_IMMR		0xF0000000
#define CONFIG_SYS_BCSR		0xF4500000
#if CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS
#define CONFIG_SYS_PCI_INT		0xF8200000
#endif
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_LSDRAM_BASE		0xFD000000

#define RS232EN_1		0x02000002
#define RS232EN_2		0x01000001
#define FETHIEN1		0x08000008
#define FETH1_RST		0x04000004
#define FETHIEN2		0x10000000
#define FETH2_RST		0x08000000
#define BCSR_PCI_MODE		0x01000000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_END	0x2000	/* End of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#ifdef CONFIG_SYS_LOWBOOT
/* PQ2FADS flash HRCW = 0x0EB4B645 */
#define CONFIG_SYS_HRCW_MASTER (   ( HRCW_BPS11 | HRCW_CIP )			    |\
			    ( HRCW_L2CPC10 | HRCW_DPPC11 | HRCW_ISB100 )    |\
			    ( HRCW_BMS | HRCW_MMR11 | HRCW_LBPC01 | HRCW_APPC10 ) |\
			    ( HRCW_CS10PC01 | HRCW_MODCK_H0101 )	     \
			)
#else
/* PQ2FADS BCSR HRCW = 0x0CB23645 */
#define CONFIG_SYS_HRCW_MASTER (   ( HRCW_BPS11 | HRCW_CIP )			    |\
			    ( HRCW_L2CPC10 | HRCW_DPPC10 | HRCW_ISB010 )    |\
			    ( HRCW_BMS | HRCW_APPC10 )			    |\
			    ( HRCW_MODCK_H0101 )			     \
			)
#endif
/* no slaves */
#define CONFIG_SYS_HRCW_SLAVE1 0
#define CONFIG_SYS_HRCW_SLAVE2 0
#define CONFIG_SYS_HRCW_SLAVE3 0
#define CONFIG_SYS_HRCW_SLAVE4 0
#define CONFIG_SYS_HRCW_SLAVE5 0
#define CONFIG_SYS_HRCW_SLAVE6 0
#define CONFIG_SYS_HRCW_SLAVE7 0

#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM	0x02	/* Software reboot	     */

#define CONFIG_SYS_MONITOR_BASE    TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#ifdef CONFIG_BZIP2
#define CONFIG_SYS_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#else
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 KB for malloc()	*/
#endif /* CONFIG_BZIP2 */

#ifndef CONFIG_SYS_RAMBOOT
#  define CONFIG_ENV_IS_IN_FLASH	1
#  define CONFIG_ENV_SECT_SIZE	0x40000
#  define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_ENV_SECT_SIZE)
#else
#  define CONFIG_ENV_IS_IN_NVRAM	1
#  define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#  define CONFIG_ENV_SIZE		0x200
#endif /* CONFIG_SYS_RAMBOOT */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8260 CPU */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CONFIG_SYS_HID0_INIT		0
#define CONFIG_SYS_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE )

#define CONFIG_SYS_HID2		0

#define CONFIG_SYS_SYPCR		0xFFFFFFC3
#define CONFIG_SYS_BCR			0x100C0000
#define CONFIG_SYS_SIUMCR		0x0A200000
#define CONFIG_SYS_SCCR		SCCR_DFBRG01
#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE | 0x00001801)
#define CONFIG_SYS_OR0_PRELIM		0xFF800876
#define CONFIG_SYS_BR1_PRELIM		(CONFIG_SYS_BCSR | 0x00001801)
#define CONFIG_SYS_OR1_PRELIM		0xFFFF8010

/*We need to configure chip select to use CPLD PCI IC on MPC8272ADS*/

#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_PCI_INT | 0x1801)	/* PCI interrupt controller */
#define CONFIG_SYS_OR3_PRELIM	0xFFFF8010
#elif CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS
#define CONFIG_SYS_BR8_PRELIM	(CONFIG_SYS_PCI_INT | 0x1801)	/* PCI interrupt controller */
#define CONFIG_SYS_OR8_PRELIM	0xFFFF8010
#endif

#define CONFIG_SYS_RMR			RMR_CSRE
#define CONFIG_SYS_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)
#define CONFIG_SYS_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)
#define CONFIG_SYS_RCCR		0

#if (CONFIG_ADSTYPE == CONFIG_SYS_8266ADS) || (CONFIG_ADSTYPE == CONFIG_SYS_8272ADS)
#undef CONFIG_SYS_LSDRAM_BASE		/* No local bus SDRAM on these boards */
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_8266ADS */

#if CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS
#define CONFIG_SYS_OR2			0xFE002EC0
#define CONFIG_SYS_PSDMR		0x824B36A3
#define CONFIG_SYS_PSRT		0x13
#define CONFIG_SYS_LSDMR		0x828737A3
#define CONFIG_SYS_LSRT		0x13
#define CONFIG_SYS_MPTPR		0x2800
#elif CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
#define CONFIG_SYS_OR2			0xFC002CC0
#define CONFIG_SYS_PSDMR		0x834E24A3
#define CONFIG_SYS_PSRT		0x13
#define CONFIG_SYS_MPTPR		0x2800
#else
#define CONFIG_SYS_OR2			0xFF000CA0
#define CONFIG_SYS_PSDMR		0x016EB452
#define CONFIG_SYS_PSRT		0x21
#define CONFIG_SYS_LSDMR		0x0086A522
#define CONFIG_SYS_LSRT		0x21
#define CONFIG_SYS_MPTPR		0x1900
#endif /* CONFIG_ADSTYPE == CONFIG_SYS_PQ2FADS */

#define CONFIG_SYS_RESET_ADDRESS	0x04400000

#if CONFIG_ADSTYPE >= CONFIG_SYS_PQ2FADS

/* PCI Memory map (if different from default map */
#define CONFIG_SYS_PCI_SLV_MEM_LOCAL	CONFIG_SYS_SDRAM_BASE		/* Local base */
#define CONFIG_SYS_PCI_SLV_MEM_BUS		0x00000000		/* PCI base */
#define CONFIG_SYS_PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
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

#define CONFIG_SYS_PCI_MSTR_MEM_LOCAL	0x80000000          /* Local base */
#define CONFIG_SYS_PCI_MSTR_MEM_BUS	0x80000000          /* PCI base   */
#define	CONFIG_SYS_CPU_PCI_MEM_START	PCI_MSTR_MEM_LOCAL
#define CONFIG_SYS_PCI_MSTR_MEM_SIZE	0x20000000          /* 512MB */
#define CONFIG_SYS_POCMR0_MASK_ATTRIB	(POCMR_MASK_512MB | POCMR_ENABLE | POCMR_PREFETCH_EN)

/*
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CONFIG_SYS_PCI_MSTR_MEMIO_LOCAL    0xA0000000          /* Local base */
#define CONFIG_SYS_PCI_MSTR_MEMIO_BUS      0xA0000000          /* PCI base   */
#define CONFIG_SYS_CPU_PCI_MEMIO_START     PCI_MSTR_MEMIO_LOCAL
#define CONFIG_SYS_PCI_MSTR_MEMIO_SIZE     0x20000000          /* 512MB */
#define CONFIG_SYS_POCMR1_MASK_ATTRIB      (POCMR_MASK_512MB | POCMR_ENABLE)

/*
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#define CONFIG_SYS_PCI_MSTR_IO_LOCAL       0xF6000000          /* Local base */
#define CONFIG_SYS_PCI_MSTR_IO_BUS         0x00000000          /* PCI base   */
#define CONFIG_SYS_CPU_PCI_IO_START        PCI_MSTR_IO_LOCAL
#define CONFIG_SYS_PCI_MSTR_IO_SIZE        0x02000000          /* 64MB */
#define CONFIG_SYS_POCMR2_MASK_ATTRIB      (POCMR_MASK_32MB | POCMR_ENABLE | POCMR_PCI_IO)


/* PCIBR0 - for PCI IO*/
#define CONFIG_SYS_PCI_MSTR0_LOCAL		CONFIG_SYS_PCI_MSTR_IO_LOCAL		/* Local base */
#define CONFIG_SYS_PCIMSK0_MASK		~(CONFIG_SYS_PCI_MSTR_IO_SIZE - 1U)	/* Size of window */
/* PCIBR1 - prefetch and non-prefetch regions joined together */
#define CONFIG_SYS_PCI_MSTR1_LOCAL		CONFIG_SYS_PCI_MSTR_MEM_LOCAL
#define CONFIG_SYS_PCIMSK1_MASK		~(CONFIG_SYS_PCI_MSTR_MEM_SIZE + CONFIG_SYS_PCI_MSTR_MEMIO_SIZE - 1U)

#endif /* CONFIG_ADSTYPE == CONFIG_8272ADS*/

#define CONFIG_HAS_ETH0

#if CONFIG_ADSTYPE == CONFIG_SYS_8272ADS
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_NETDEV eth0
#define CONFIG_LOADADDR 500000 /* default location for tftp and bootm */

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" MK_STR(CONFIG_NETDEV) "\0"				\
	"tftpflash=tftpboot $loadaddr $uboot; "				\
		"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
		"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
		"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
		"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
		"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
	"fdtaddr=400000\0"						\
	"console=ttyCPM0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	 \
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setipargs;"						\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv rootdev /dev/ram;"					\
	"run setbootargs;"						\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#undef MK_STR
#undef XMK_STR

#endif /* __CONFIG_H */
