/*
 * Copyright (c) 2008 Nuovation System Designs, LLC
 *   Grant Erickson <gerickson@nuovations.com>
 *
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

/************************************************************************
 * makalu.h - configuration for AMCC Makalu (405EX)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_MAKALU		1		/* Board is Makalu	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EX		1		/* Specifc 405EX support*/
#define CONFIG_SYS_CLK_FREQ	33330000	/* ext frequency to pll	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFA0000

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME	makalu
#define CONFIG_ADDMISC	"addmisc=setenv bootargs ${bootargs} rtc-x1205.probe=0,0x6f\0"
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0xFC000000
#define CONFIG_SYS_FPGA_BASE		0xF0000000

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
 *   To use (2), define 'CONFIG_SYS_INIT_DCACHE_CS' to be an unused chip
 *   select on the External Bus Controller (EBC) and then select a
 *   value for 'CONFIG_SYS_INIT_RAM_ADDR' outside of the range of valid,
 *   physical SDRAM. Otherwise, undefine 'CONFIG_SYS_INIT_DCACHE_CS' and
 *   select a value for 'CONFIG_SYS_INIT_RAM_ADDR' within the range of valid,
 *   physical SDRAM to use (3).
 *-----------------------------------------------------------------------*/

#define CONFIG_SYS_INIT_DCACHE_CS	4

#if defined(CONFIG_SYS_INIT_DCACHE_CS)
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_SDRAM_BASE + ( 1 << 30))	/*  1 GiB */
#else
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_SDRAM_BASE + (32 << 20))	/* 32 MiB */
#endif /* defined(CONFIG_SYS_INIT_DCACHE_CS) */

#define CONFIG_SYS_INIT_RAM_SIZE        (4 << 10)			/*  4 KiB */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * If the data cache is being used for the primordial stack and global
 * data area, the POST word must be placed somewhere else. The General
 * Purpose Timer (GPT) is unused by u-boot and the kernel and preserves
 * its compare and mask register contents across reset, so it is used
 * for the POST word.
 */

#if defined(CONFIG_SYS_INIT_DCACHE_CS)
# define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
# define CONFIG_SYS_POST_WORD_ADDR	(CONFIG_SYS_PERIPHERAL_BASE + GPT0_COMP6)
#else
# define CONFIG_SYS_INIT_EXTRA_SIZE	16
# define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - CONFIG_SYS_INIT_EXTRA_SIZE)
# define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_INIT_RAM_ADDR
#endif /* defined(CONFIG_SYS_INIT_DCACHE_CS) */

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#undef CONFIG_SYS_EXT_SERIAL_CLOCK			/* no ext. clk		*/

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/

#define CONFIG_SYS_FLASH_BANKS_LIST    {CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000	/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MBYTES_SDRAM        (256)		/* 256MB			*/

#define	CONFIG_SYS_SDRAM0_MB0CF_BASE	((  0 << 20) + CONFIG_SYS_SDRAM_BASE)
#define	CONFIG_SYS_SDRAM0_MB1CF_BASE	((128 << 20) + CONFIG_SYS_SDRAM_BASE)

/* DDR1/2 SDRAM Device Control Register Data Values */
#define CONFIG_SYS_SDRAM0_MB0CF	((CONFIG_SYS_SDRAM0_MB0CF_BASE >> 3)	| \
				 SDRAM_RXBAS_SDSZ_128MB 	| \
				 SDRAM_RXBAS_SDAM_MODE2 	| \
				 SDRAM_RXBAS_SDBE_ENABLE)
#define CONFIG_SYS_SDRAM0_MB1CF	((CONFIG_SYS_SDRAM0_MB1CF_BASE >> 3)	| \
				 SDRAM_RXBAS_SDSZ_128MB 	| \
				 SDRAM_RXBAS_SDAM_MODE2 	| \
				 SDRAM_RXBAS_SDBE_ENABLE)
#define CONFIG_SYS_SDRAM0_MB2CF	SDRAM_RXBAS_SDBE_DISABLE
#define CONFIG_SYS_SDRAM0_MB3CF	SDRAM_RXBAS_SDBE_DISABLE
#define CONFIG_SYS_SDRAM0_MCOPT1	0x04322000
#define CONFIG_SYS_SDRAM0_MCOPT2	0x00000000
#define CONFIG_SYS_SDRAM0_MODT0	0x01800000
#define CONFIG_SYS_SDRAM0_MODT1	0x00000000
#define CONFIG_SYS_SDRAM0_CODT		0x0080f837
#define CONFIG_SYS_SDRAM0_RTR		0x06180000
#define CONFIG_SYS_SDRAM0_INITPLR0	0xa8380000
#define CONFIG_SYS_SDRAM0_INITPLR1	0x81900400
#define CONFIG_SYS_SDRAM0_INITPLR2	0x81020000
#define CONFIG_SYS_SDRAM0_INITPLR3	0x81030000
#define CONFIG_SYS_SDRAM0_INITPLR4	0x81010404
#define CONFIG_SYS_SDRAM0_INITPLR5	0x81000542
#define CONFIG_SYS_SDRAM0_INITPLR6	0x81900400
#define CONFIG_SYS_SDRAM0_INITPLR7	0x8D080000
#define CONFIG_SYS_SDRAM0_INITPLR8	0x8D080000
#define CONFIG_SYS_SDRAM0_INITPLR9	0x8D080000
#define CONFIG_SYS_SDRAM0_INITPLR10	0x8D080000
#define CONFIG_SYS_SDRAM0_INITPLR11	0x81000442
#define CONFIG_SYS_SDRAM0_INITPLR12	0x81010780
#define CONFIG_SYS_SDRAM0_INITPLR13	0x81010400
#define CONFIG_SYS_SDRAM0_INITPLR14	0x00000000
#define CONFIG_SYS_SDRAM0_INITPLR15	0x00000000
#define CONFIG_SYS_SDRAM0_RQDC		0x80000038
#define CONFIG_SYS_SDRAM0_RFDC		0x00000209
#define CONFIG_SYS_SDRAM0_RDCC		0x40000000
#define CONFIG_SYS_SDRAM0_DLCR		0x030000a5
#define CONFIG_SYS_SDRAM0_CLKTR	0x80000000
#define CONFIG_SYS_SDRAM0_WRDTR	0x00000000
#define CONFIG_SYS_SDRAM0_SDTR1	0x80201000
#define CONFIG_SYS_SDRAM0_SDTR2	0x32204232
#define CONFIG_SYS_SDRAM0_SDTR3	0x080b0d1a
#define CONFIG_SYS_SDRAM0_MMODE	0x00000442
#define CONFIG_SYS_SDRAM0_MEMODE	0x00000404

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	6	/* 24C02 requires 5ms delay */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x52	/* I2C boot EEPROM (24C02BN)	*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/

/* Standard DTT sensor configuration */
#define CONFIG_DTT_DS1775	1
#define CONFIG_DTT_SENSORS	{ 0 }
#define CONFIG_SYS_I2C_DTT_ADDR	0x48

/* RTC configuration */
#define CONFIG_RTC_X1205	1
#define CONFIG_SYS_I2C_RTC_ADDR	0x6f

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_M88E1111_PHY	1
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_EMAC_PHY_MODE	EMAC_PHY_MODE_RGMII_RGMII
#define CONFIG_PHY_ADDR		6	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0		1

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	0

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
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
#define CONFIG_CMD_DTT
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SNTP

/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_CACHE		| \
				 CONFIG_SYS_POST_CPU		| \
				 CONFIG_SYS_POST_ETHER		| \
				 CONFIG_SYS_POST_I2C		| \
				 CONFIG_SYS_POST_MEMORY	| \
				 CONFIG_SYS_POST_UART)

/* Define here the base-addresses of the UARTs to test in POST */
#define CONFIG_SYS_POST_UART_TABLE	{ CONFIG_SYS_NS16550_COM1, \
			CONFIG_SYS_NS16550_COM2 }

#define CONFIG_LOGBUFFER
#define CONFIG_SYS_POST_CACHE_ADDR	0x00800000 /* free virtual address	*/

#define CONFIG_SYS_CONSOLE_IS_IN_ENV /* Otherwise it catches logbuffer as output */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP		1	/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup	*/
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/*-----------------------------------------------------------------------
 * PCIe stuff
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_PCIE_MEMBASE	0x90000000	/* mapped PCIe memory	*/
#define CONFIG_SYS_PCIE_MEMSIZE	0x08000000      /* 128 Meg, smallest incr per port */

#define	CONFIG_SYS_PCIE0_CFGBASE	0xa0000000      /* remote access */
#define	CONFIG_SYS_PCIE0_XCFGBASE	0xb0000000      /* local access */
#define	CONFIG_SYS_PCIE0_CFGMASK	0xe0000001      /* 512 Meg */

#define	CONFIG_SYS_PCIE1_CFGBASE	0xc0000000      /* remote access */
#define	CONFIG_SYS_PCIE1_XCFGBASE	0xd0000000      /* local access */
#define	CONFIG_SYS_PCIE1_CFGMASK	0xe0000001      /* 512 Meg */

#define	CONFIG_SYS_PCIE0_UTLBASE	0xef502000
#define	CONFIG_SYS_PCIE1_UTLBASE	0xef503000

/* base address of inbound PCIe window */
#define CONFIG_SYS_PCIE_INBOUND_BASE	0x0000000000000000ULL

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CONFIG_SYS_EBC_PB0AP		0x08033700
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH_BASE | 0xda000)

/* Memory Bank 2 (CPLD) initialization						*/
#define CONFIG_SYS_EBC_PB2AP           0x9400C800
#define CONFIG_SYS_EBC_PB2CR           0xF0018000 /*  BAS=0x800,BS=1MB,BU=R/W,BW=8bit	*/

#define CONFIG_SYS_EBC_CFG		0x7FC00000 /*  EBC0_CFG */

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
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
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO11 IRQ(6)				*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO12 EBC_DATA(16)	USB2_DATA(0)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO13 EBC_DATA(17)	USB2_DATA(1)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO14 EBC_DATA(18)	USB2_DATA(2)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO15 EBC_DATA(19)	USB2_DATA(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 UART0_DCD	UART1_CTS	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 UART0_DSR	UART1_RTS	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 UART0_CTS			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 UART0_RTS			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO20 UART0_DTR	UART1_TX	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO21 UART0_RI	UART1_RX	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO22 EBC_HOLD_REQ	DMA_ACK2	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_0}, /* GPIO23 EBC_HOLD_ACK	DMA_REQ2	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO24 EBC_EXT_REQ	DMA_EOT2	IRQ(4) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO25 EBC_EXT_ACK	DMA_ACK3	IRQ(3) */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 EBC_ADDR(5)	DMA_EOT0	TS(3) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL , GPIO_OUT_0}, /* GPIO27 EBC_BUS_REQ	DMA_EOT3	IRQ(5) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL , GPIO_OUT_0}, /* GPIO28				*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO29 DMA_EOT1	IRQ(2)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO30 DMA_REQ1	IRQ(1)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO31 DMA_ACK1	IRQ(0)		*/	\
}												\
}

#define CONFIG_SYS_GPIO_PCIE_RST	23
#define CONFIG_SYS_GPIO_PCIE_CLKREQ	27
#define CONFIG_SYS_GPIO_PCIE_WAKE	28

#endif	/* __CONFIG_H */
