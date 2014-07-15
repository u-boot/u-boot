/*
 * Copyright (C) 2004 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * U-Boot configuration for Analogue&Micro Rattler boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_MPC8248
#define CPU_ID_STR		"MPC8248"
#else
#define CPU_ID_STR		"MPC8250"
#endif /* CONFIG_MPC8248 */

#define	CONFIG_SYS_TEXT_BASE	0xFE000000

#define CONFIG_CPM2		1	/* Has a CPM2 */

#define CONFIG_RATTLER			/* Analogue&Micro Rattler board */

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
 * must be defined elsewhere (as for the console), or CONFIG_CMD_NET
 * must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* Ethernet is not on SCC */
#define CONFIG_ETHER_ON_FCC		/* Ethernet is on FCC     */
#undef	CONFIG_ETHER_NONE		/* No external Ethernet   */

#ifdef CONFIG_ETHER_ON_FCC

#define CONFIG_ETHER_INDEX	1	/* FCC1 is used for Ethernet */

#if   (CONFIG_ETHER_INDEX == 1)

/* - Rx clock is CLK11
 * - Tx clock is CLK10
 * - BDs/buffers on 60x bus
 * - Full duplex
 */
#define CONFIG_SYS_CMXFCR_MASK1		(CMXFCR_FC1 | CMXFCR_RF1CS_MSK | CMXFCR_TF1CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE1	(CMXFCR_RF1CS_CLK11 | CMXFCR_TF1CS_CLK10)
#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#elif (CONFIG_ETHER_INDEX == 2)

/* - Rx clock is CLK15
 * - Tx clock is CLK14
 * - BDs/buffers on 60x bus
 * - Full duplex
 */
#define CONFIG_SYS_CMXFCR_MASK2		(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK15 | CMXFCR_TF2CS_CLK14)
#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_INDEX */

#define CONFIG_MII			/* MII PHY management        */
#define CONFIG_BITBANGMII		/* Bit-banged MDIO interface */
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT		2	/* Port C */
#define MDIO_DECLARE		volatile ioport_t *iop = ioport_addr ( \
					(immap_t *) CONFIG_SYS_IMMR, MDIO_PORT )
#define MDC_DECLARE		MDIO_DECLARE

#define MDIO_ACTIVE		(iop->pdir |=  0x00400000)
#define MDIO_TRISTATE		(iop->pdir &= ~0x00400000)
#define MDIO_READ		((iop->pdat &  0x00400000) != 0)

#define MDIO(bit)		if(bit) iop->pdat |=  0x00400000; \
				else	iop->pdat &= ~0x00400000

#define MDC(bit)		if(bit) iop->pdat |=  0x00800000; \
				else	iop->pdat &= ~0x00800000

#define MIIDELAY		udelay(1)

#endif /* CONFIG_ETHER_ON_FCC */

#ifndef CONFIG_8260_CLKIN
#define CONFIG_8260_CLKIN	100000000	/* in Hz */
#endif

#define CONFIG_BAUDRATE		38400


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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING


#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_BOOTCOMMAND	"bootm FE040000"	/* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2 rw mtdparts=phys:1M(ROM)ro,-(root)"

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
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size  */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size  */

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of flash banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max num of sects on one chip */

#define	CONFIG_SYS_DIRECT_FLASH_TFTP

#if defined(CONFIG_CMD_JFFS2)
#define CONFIG_SYS_JFFS2_NUM_BANKS	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_SORT_FRAGMENTS

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00100000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=rattler-0"
#define MTDPARTS_DEFAULT	"mtdparts=rattler-0:-@1m(jffs2)"
*/
#endif /* CONFIG_CMD_JFFS2 */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/

#define CONFIG_ENV_IS_IN_FLASH

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#endif /* CONFIG_ENV_IS_IN_FLASH */

#define CONFIG_SYS_DEFAULT_IMMR	0xFF010000

#define CONFIG_SYS_IMMR		0xF0000000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2000	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		32
#define CONFIG_SYS_SDRAM_BR		(CONFIG_SYS_SDRAM_BASE | 0x00000041)
#define CONFIG_SYS_SDRAM_OR		0xFE002EC0

#define CONFIG_SYS_BCSR		0xFC000000

/* Hard reset configuration word */
#define CONFIG_SYS_HRCW_MASTER		0x0A06875A /* Not used - provided by FPGA */
/* No slaves */
#define CONFIG_SYS_HRCW_SLAVE1		0
#define CONFIG_SYS_HRCW_SLAVE2		0
#define CONFIG_SYS_HRCW_SLAVE3		0
#define CONFIG_SYS_HRCW_SLAVE4		0
#define CONFIG_SYS_HRCW_SLAVE5		0
#define CONFIG_SYS_HRCW_SLAVE6		0
#define CONFIG_SYS_HRCW_SLAVE7		0

#define CONFIG_SYS_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8260 CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CONFIG_SYS_HID0_INIT		0
#define CONFIG_SYS_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE)

#define CONFIG_SYS_HID2		0

#define CONFIG_SYS_SIUMCR		0x0E04C000
#define CONFIG_SYS_SYPCR		0xFFFFFFC3
#define CONFIG_SYS_BCR			0x00000000
#define CONFIG_SYS_SCCR		SCCR_DFBRG01

#define CONFIG_SYS_RMR			RMR_CSRE
#define CONFIG_SYS_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)
#define CONFIG_SYS_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)
#define CONFIG_SYS_RCCR		0

#define CONFIG_SYS_PSDMR		0x8249A452
#define CONFIG_SYS_PSRT		0x1F
#define CONFIG_SYS_MPTPR		0x2000

#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE | 0x00001001)
#define CONFIG_SYS_OR0_PRELIM		0xFF001ED6
#define CONFIG_SYS_BR7_PRELIM		(CONFIG_SYS_BCSR | 0x00000801)
#define CONFIG_SYS_OR7_PRELIM		0xFFFF87F6

#define CONFIG_SYS_RESET_ADDRESS	0xC0000000

#endif /* __CONFIG_H */
