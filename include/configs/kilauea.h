/*
 * Copyright (c) 2008 Nuovation System Designs, LLC
 *   Grant Erickson <gerickson@nuovations.com>
 *
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/************************************************************************
 * kilauea.h - configuration for AMCC Kilauea (405EX)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_KILAUEA		1		/* Board is Kilauea	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EX		1		/* Specifc 405EX support*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* ext frequency to pll	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		kilauea
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/
#define CONFIG_BOARD_EMAC_COUNT

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_FLASH_BASE		0xFC000000
#define CFG_NAND_ADDR		0xF8000000
#define CFG_FPGA_BASE		0xF0000000
#define CFG_PERIPHERAL_BASE	0xEF600000      /* internal peripherals*/

/*-----------------------------------------------------------------------
 * Initial RAM & Stack Pointer Configuration Options
 *
 *   There are traditionally three options for the primordial
 *   (i.e. initial) stack usage on the 405-series:
 *
 *      1) On-chip Memory (OCM) (i.e. SRAM)
 *      2) Data cache
 *      3) SDRAM
 *
 *   For the 405EX(r), there is no OCM, so we are left with (2) or (3)
 *   the latter of which is less than desireable since it requires
 *   setting up the SDRAM and ECC in assembly code.
 *
 *   To use (2), define 'CFG_INIT_DCACHE_CS' to be an unused chip
 *   select on the External Bus Controller (EBC) and then select a
 *   value for 'CFG_INIT_RAM_ADDR' outside of the range of valid,
 *   physical SDRAM. Otherwise, undefine 'CFG_INIT_DCACHE_CS' and
 *   select a value for 'CFG_INIT_RAM_ADDR' within the range of valid,
 *   physical SDRAM to use (3).
 *-----------------------------------------------------------------------*/

#define CFG_INIT_DCACHE_CS	4

#if defined(CFG_INIT_DCACHE_CS)
#define CFG_INIT_RAM_ADDR	(CFG_SDRAM_BASE + ( 1 << 30))	/*  1 GiB */
#else
#define CFG_INIT_RAM_ADDR	(CFG_SDRAM_BASE + (32 << 20))	/* 32 MiB */
#endif /* defined(CFG_INIT_DCACHE_CS) */

#define CFG_INIT_RAM_END        (4 << 10)			/*  4 KiB */
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

/*
 * If the data cache is being used for the primordial stack and global
 * data area, the POST word must be placed somewhere else. The General
 * Purpose Timer (GPT) is unused by u-boot and the kernel and preserves
 * its compare and mask register contents across reset, so it is used
 * for the POST word.
 */

#if defined(CFG_INIT_DCACHE_CS)
# define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET
# define CFG_POST_ALT_WORD_ADDR	(CFG_PERIPHERAL_BASE + GPT0_COMP6)
#else
# define CFG_INIT_EXTRA_SIZE	16
# define CFG_INIT_SP_OFFSET	(CFG_GBL_DATA_OFFSET - CFG_INIT_EXTRA_SIZE)
# define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 4)
# define CFG_OCM_DATA_ADDR	CFG_INIT_RAM_ADDR
#endif /* defined(CFG_INIT_DCACHE_CS) */

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CFG_EXT_SERIAL_CLOCK	11059200	/* ext. 11.059MHz clk	*/
/* define this if you want console on UART1 */
#undef CONFIG_UART1_CONSOLE

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars	*/
#define CFG_ENV_IS_EMBEDDED	1	/* use embedded environment */
#endif

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI			/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST    {CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000	/* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 4k from NAND (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (NUB) from NAND and execute it. This SPL
 * has to fit into 4kByte. It sets up the CPU and configures the SDRAM
 * controller and the NAND controller so that the special U-Boot image can be
 * loaded from NAND to SDRAM.
 *
 * NUB (NAND U-Boot)
 * This NAND U-Boot (NUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 * On 405EX the SPL is copied to SDRAM before the NAND controller is
 * set up. While still running from location 0xfffff000...0xffffffff the
 * NAND controller cannot be accessed since it is attached to CS0 too.
 */
#define CFG_NAND_BOOT_SPL_SRC	0xfffff000	/* SPL location			*/
#define CFG_NAND_BOOT_SPL_SIZE	(4 << 10)	/* SPL size			*/
#define CFG_NAND_BOOT_SPL_DST	0x00800000	/* Copy SPL here		*/
#define CFG_NAND_U_BOOT_DST	0x01000000	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/
#define CFG_NAND_BOOT_SPL_DELTA	(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(16 << 10)	/* Offset to RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	(384 << 10)	/* Size of RAM U-Boot image	*/

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CFG_NAND_PAGE_SIZE	512		/* NAND chip page size		*/
#define CFG_NAND_BLOCK_SIZE	(16 << 10)	/* NAND chip block size		*/
#define CFG_NAND_PAGE_COUNT	32		/* NAND chip page count		*/
#define CFG_NAND_BAD_BLOCK_POS	5		/* Location of bad block marker	*/
#define CFG_NAND_4_ADDR_CYCLE	1		/* Fourth addr used (>32MB)	*/

#define CFG_NAND_ECCSIZE	256
#define CFG_NAND_ECCBYTES	3
#define CFG_NAND_ECCSTEPS	(CFG_NAND_PAGE_SIZE / CFG_NAND_ECCSIZE)
#define CFG_NAND_OOBSIZE	16
#define CFG_NAND_ECCTOTAL	(CFG_NAND_ECCBYTES * CFG_NAND_ECCSTEPS)
#define CFG_NAND_ECCPOS		{0, 1, 2, 3, 6, 7}

#ifdef CFG_ENV_IS_IN_NAND
/*
 * For NAND booting the environment is embedded in the U-Boot image. Please take
 * look at the file board/amcc/sequoia/u-boot-nand.lds for details.
 */
#define CFG_ENV_SIZE		CFG_NAND_BLOCK_SIZE
#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_ENV_SIZE)
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET + CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		(CFG_NAND_ADDR + CFG_NAND_CS)
#define CFG_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM        (256)		/* 256MB			*/

#define	CFG_SDRAM0_MB0CF_BASE	((  0 << 20) + CFG_SDRAM_BASE)

/* DDR1/2 SDRAM Device Control Register Data Values */
#define CFG_SDRAM0_MB0CF	((CFG_SDRAM0_MB0CF_BASE >> 3)	| \
				 SDRAM_RXBAS_SDSZ_256MB		| \
				 SDRAM_RXBAS_SDAM_MODE7		| \
				 SDRAM_RXBAS_SDBE_ENABLE)
#define CFG_SDRAM0_MB1CF	SDRAM_RXBAS_SDBE_DISABLE
#define CFG_SDRAM0_MB2CF	SDRAM_RXBAS_SDBE_DISABLE
#define CFG_SDRAM0_MB3CF	SDRAM_RXBAS_SDBE_DISABLE
#define CFG_SDRAM0_MCOPT1	0x04322000
#define CFG_SDRAM0_MCOPT2	0x00000000
#define CFG_SDRAM0_MODT0	0x01800000
#define CFG_SDRAM0_MODT1	0x00000000
#define CFG_SDRAM0_CODT		0x0080f837
#define CFG_SDRAM0_RTR		0x06180000
#define CFG_SDRAM0_INITPLR0	0xa8380000
#define CFG_SDRAM0_INITPLR1	0x81900400
#define CFG_SDRAM0_INITPLR2	0x81020000
#define CFG_SDRAM0_INITPLR3	0x81030000
#define CFG_SDRAM0_INITPLR4	0x81010404
#define CFG_SDRAM0_INITPLR5	0x81000542
#define CFG_SDRAM0_INITPLR6	0x81900400
#define CFG_SDRAM0_INITPLR7	0x8D080000
#define CFG_SDRAM0_INITPLR8	0x8D080000
#define CFG_SDRAM0_INITPLR9	0x8D080000
#define CFG_SDRAM0_INITPLR10	0x8D080000
#define CFG_SDRAM0_INITPLR11	0x81000442
#define CFG_SDRAM0_INITPLR12	0x81010780
#define CFG_SDRAM0_INITPLR13	0x81010400
#define CFG_SDRAM0_INITPLR14	0x00000000
#define CFG_SDRAM0_INITPLR15	0x00000000
#define CFG_SDRAM0_RQDC		0x80000038
#define CFG_SDRAM0_RFDC		0x00000209
#define CFG_SDRAM0_RDCC		0x40000000
#define CFG_SDRAM0_DLCR		0x030000a5
#define CFG_SDRAM0_CLKTR	0x80000000
#define CFG_SDRAM0_WRDTR	0x00000000
#define CFG_SDRAM0_SDTR1	0x80201000
#define CFG_SDRAM0_SDTR2	0x32204232
#define CFG_SDRAM0_SDTR3	0x080b0d1a
#define CFG_SDRAM0_MMODE	0x00000442
#define CFG_SDRAM0_MEMODE	0x00000404

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/

#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	6	/* 24C02 requires 5ms delay */
#define CFG_I2C_EEPROM_ADDR	0x52	/* I2C boot EEPROM (24C02BN)	*/
#define CFG_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/

/* Standard DTT sensor configuration */
#define CONFIG_DTT_DS1775	1
#define CONFIG_DTT_SENSORS	{ 0 }
#define CFG_I2C_DTT_ADDR	0x48

/* RTC configuration */
#define CONFIG_RTC_DS1338	1
#define CFG_I2C_RTC_ADDR	0x68

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_M88E1111_PHY	1
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0		1

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	2

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	CONFIG_AMCC_DEF_ENV_NAND_UPD					\
	"logversion=2\0"						\
	"kernel_addr=fc000000\0"					\
	"fdt_addr=fc1e0000\0"						\
	"ramdisk_addr=fc200000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP\0"						\
	""

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_LOG
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP

/* POST support */
#define CONFIG_POST		(CFG_POST_CACHE		| \
				 CFG_POST_CPU		| \
				 CFG_POST_ETHER		| \
				 CFG_POST_I2C		| \
				 CFG_POST_MEMORY	| \
				 CFG_POST_UART)

/* Define here the base-addresses of the UARTs to test in POST */
#define CFG_POST_UART_TABLE	{UART0_BASE, UART1_BASE}

#define CONFIG_LOGBUFFER
#define CFG_POST_CACHE_ADDR	0x00800000 /* free virtual address	*/

#define CFG_CONSOLE_IS_IN_ENV /* Otherwise it catches logbuffer as output */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP		1	/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup	*/
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/*-----------------------------------------------------------------------
 * PCIe stuff
 *----------------------------------------------------------------------*/
#define CFG_PCIE_MEMBASE	0x90000000	/* mapped PCIe memory	*/
#define CFG_PCIE_MEMSIZE	0x08000000      /* 128 Meg, smallest incr per port */

#define	CFG_PCIE0_CFGBASE	0xa0000000      /* remote access */
#define	CFG_PCIE0_XCFGBASE	0xb0000000      /* local access */
#define	CFG_PCIE0_CFGMASK	0xe0000001      /* 512 Meg */

#define	CFG_PCIE1_CFGBASE	0xc0000000      /* remote access */
#define	CFG_PCIE1_XCFGBASE	0xd0000000      /* local access */
#define	CFG_PCIE1_CFGMASK	0xe0000001      /* 512 Meg */

#define	CFG_PCIE0_UTLBASE	0xef502000
#define	CFG_PCIE1_UTLBASE	0xef503000

/* base address of inbound PCIe window */
#define CFG_PCIE_INBOUND_BASE	0x0000000000000000ULL

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
/* booting from NAND, so NAND chips select has to be on CS 0 */
#define CFG_NAND_CS		0		/* NAND chip connected to CSx	*/

/* Memory Bank 1 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB1AP		0x05806500
#define CFG_EBC_PB1CR           0xFC0DA000  /* BAS=0xFC0,BS=64MB,BU=R/W,BW=16bit*/

/* Memory Bank 0 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x018003c0
#define CFG_EBC_PB0CR		(CFG_NAND_ADDR | 0x1e000)
#else
#define CFG_NAND_CS		1		/* NAND chip connected to CSx	*/

/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x05806500
#define CFG_EBC_PB0CR           0xFC0DA000  /* BAS=0xFC0,BS=64MB,BU=R/W,BW=16bit*/

/* Memory Bank 1 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB1AP		0x018003c0
#define CFG_EBC_PB1CR		(CFG_NAND_ADDR | 0x1e000)
#endif

/* Memory Bank 2 (FPGA) initialization						*/
#define CFG_EBC_PB2AP           0x9400C800
#define CFG_EBC_PB2CR		(CFG_FPGA_BASE | 0x18000)

#define CFG_EBC_CFG		0x7FC00000 /*  EBC0_CFG */

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CFG_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO0	EBC_DATA_PAR(0)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO1	EBC_DATA_PAR(1)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO2	EBC_DATA_PAR(2)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO3	EBC_DATA_PAR(3)			*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO4	EBC_DATA(20)	USB2_DATA(4)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO5	EBC_DATA(21)	USB2_DATA(5)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO6	EBC_DATA(22)	USB2_DATA(6)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO7	EBC_DATA(23)	USB2_DATA(7)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	CS(1)/NFCE(1)	IRQ(7)		*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	CS(2)/NFCE(2)	IRQ(8)		*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 CS(3)/NFCE(3)	IRQ(9)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0}, /* GPIO11 IRQ(6)				*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO12 EBC_DATA(16)	USB2_DATA(0)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO13 EBC_DATA(17)	USB2_DATA(1)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO14 EBC_DATA(18)	USB2_DATA(2)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO15 EBC_DATA(19)	USB2_DATA(3)	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 UART0_DCD	UART1_CTS	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 UART0_DSR	UART1_RTS	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 UART0_CTS			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 UART0_RTS			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_0}, /* GPIO20 UART0_DTR	UART1_TX	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO21 UART0_RI	UART1_RX	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO22 EBC_HOLD_REQ	DMA_ACK2	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO23 EBC_HOLD_ACK	DMA_REQ2	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO24 EBC_EXT_REQ	DMA_EOT2	IRQ(4) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO25 EBC_EXT_ACK	DMA_ACK3	IRQ(3) */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 EBC_ADDR(5)	DMA_EOT0	TS(3) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO27 EBC_BUS_REQ	DMA_EOT3	IRQ(5) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO28				*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO29 DMA_EOT1	IRQ(2)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO30 DMA_REQ1	IRQ(1)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO31 DMA_ACK1	IRQ(0)		*/	\
}												\
}

/*-----------------------------------------------------------------------
 * Some Kilauea stuff..., mainly fpga registers
 */
#define CFG_FPGA_REG_BASE		CFG_FPGA_BASE
#define CFG_FPGA_FIFO_BASE		(in32(CFG_FPGA_BASE) | (1 << 10))

/* interrupt */
#define CFG_FPGA_SLIC0_R_DPRAM_INT	0x80000000
#define CFG_FPGA_SLIC0_W_DPRAM_INT	0x40000000
#define CFG_FPGA_SLIC1_R_DPRAM_INT	0x20000000
#define CFG_FPGA_SLIC1_W_DPRAM_INT	0x10000000
#define CFG_FPGA_PHY0_INT		0x08000000
#define CFG_FPGA_PHY1_INT		0x04000000
#define CFG_FPGA_SLIC0_INT		0x02000000
#define CFG_FPGA_SLIC1_INT		0x01000000

/* DPRAM setting */
/* 00: 32B; 01: 64B; 10: 128B; 11: 256B  */
#define CFG_FPGA_DPRAM_R_INT_LINE	0x00400000	/* 64 B */
#define CFG_FPGA_DPRAM_W_INT_LINE	0x00100000	/* 64 B */
#define CFG_FPGA_DPRAM_RW_TYPE		0x00080000
#define CFG_FPGA_DPRAM_RST		0x00040000
#define CFG_FPGA_UART0_FO		0x00020000
#define CFG_FPGA_UART1_FO		0x00010000

/* loopback */
#define CFG_FPGA_CHIPSIDE_LOOPBACK	0x00004000
#define CFG_FPGA_LINESIDE_LOOPBACK	0x00008000
#define CFG_FPGA_SLIC0_ENABLE		0x00002000
#define CFG_FPGA_SLIC1_ENABLE		0x00001000
#define CFG_FPGA_SLIC0_CS		0x00000800
#define CFG_FPGA_SLIC1_CS		0x00000400
#define CFG_FPGA_USER_LED0		0x00000200
#define CFG_FPGA_USER_LED1		0x00000100

#endif	/* __CONFIG_H */
