/*
 * (C) Copyright 2009
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd.eu
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_405EP		1	/* This is a PPC405 CPU		*/
#define CONFIG_PMC405DE		1	/* ...on a PMC405DE board	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/
#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#define CONFIG_SYS_CLK_FREQ	33330000 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#undef  CONFIG_BOOTARGS
#undef  CONFIG_BOOTCOMMAND

#define CONFIG_PREBOOT			/* enable preboot variable	*/

#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change*/

#define CONFIG_HAS_ETH1

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address			*/
#define CONFIG_PHY1_ADDR	2	/* 2nd PHY address		*/

#define CONFIG_SYS_RX_ETH_BUFFER	16 /* use 16 rx buffer on 405 emac */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME

/*
 * Command line configuration.
 */
#define CONFIG_CMD_BSP
#define CONFIG_CMD_CHIP_CONFIG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

#undef  CONFIG_WATCHDOG			/* watchdog disabled */
#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0 */
#define CONFIG_PRAM		0

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP

#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device */
#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console info */

#define CONFIG_SYS_MEMTEST_START	0x0100000 /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x3000000 /* 1 ... 48 MB in DRAM */

#define CONFIG_CONS_INDEX	2	/* Use UART1			*/
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef  CONFIG_SYS_EXT_SERIAL_CLOCK
#define CONFIG_SYS_BASE_BAUD		691200

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1	/* To use extended board_into (bd_t) */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history     */
#define CONFIG_LOOPW		1	/* enable loopw command         */
#define CONFIG_MX_CYCLIC	1	/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*
 * PCI stuff
 */
#define PCI_HOST_ADAPTER	0	/* configure as pci adapter	*/
#define PCI_HOST_FORCE		1	/* configure as pci host	*/
#define PCI_HOST_AUTO		2	/* detected via arbiter enable	*/

#define CONFIG_PCI		/* include pci support			*/
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST	PCI_HOST_AUTO  /* select pci host function	*/
#define CONFIG_PCI_PNP		/* do (not) pci plug-and-play		*/

#define CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup		*/

/*
 * PCI identification
 */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID		PCI_VENDOR_ID_ESDGMBH
#define CONFIG_SYS_PCI_SUBSYS_ID_NONMONARCH 0x040e /* Dev ID: Non-Monarch */
#define CONFIG_SYS_PCI_SUBSYS_ID_MONARCH 0x040f	/* Dev ID: Monarch */
#define CONFIG_SYS_PCI_CLASSCODE_NONMONARCH	PCI_CLASS_PROCESSOR_POWERPC
#define CONFIG_SYS_PCI_CLASSCODE_MONARCH	PCI_CLASS_BRIDGE_HOST

#define CONFIG_SYS_PCI_CLASSCODE CONFIG_SYS_PCI_CLASSCODE_MONARCH
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID CONFIG_SYS_PCI_SUBSYS_ID_MONARCH

#define CONFIG_SYS_PCI_PTM1LA  0x00000000      /* point to sdram */
#define CONFIG_SYS_PCI_PTM1MS  0xfc000001      /* 64MB, enable=1 */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000      /* Host: use this pci address */
#define CONFIG_SYS_PCI_PTM2LA  0xef000000      /* point to CPLD, GPIO */
#define CONFIG_SYS_PCI_PTM2MS  0xff000001      /* 16MB, enable=1 */
#define CONFIG_SYS_PCI_PTM2PCI 0x04000000      /* Host: use this pci address */

#define CONFIG_PCI_4xx_PTM_OVERWRITE	1 /* overwrite PTMx settings by env */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)
/*
 * FLASH organization
 */
#define CONFIG_SYS_FLASH_CFI		1	/* CFI compatible */
#define CONFIG_FLASH_CFI_DRIVER		1	/* Use common CFI driver */

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max. no. memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max sectors per chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* erase timeout (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* write timeout (in ms) */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* buffered writes (faster) */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* hardware flash protection */

#define CONFIG_SYS_FLASH_EMPTY_INFO	1 /* 'E' for empty sector (flinfo) */
#define CONFIG_SYS_FLASH_QUIET_TEST	1 /* don't warn upon unknown flash */


/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xfe000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(~(CONFIG_SYS_TEXT_BASE) + 1)
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)

/*
 * Environment in EEPROM setup
 */
#define CONFIG_ENV_IS_IN_EEPROM		1
#define CONFIG_ENV_OFFSET		0x100
#define CONFIG_ENV_SIZE			0x700

/*
 * I2C EEPROM (24W16) for environment
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM 24W16	*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address" */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has */
					/* 16 byte page write mode using*/
					/* last 4 bits of the address */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10 /* and takes up to 10 msec */
#define CONFIG_SYS_EEPROM_WREN		1

#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x50
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET	0x40
#define CONFIG_4xx_CONFIG_BLOCKSIZE		0x20

/*
 * RTC
 */
#define CONFIG_RTC_RX8025

/*
 * External Bus Controller (EBC) Setup
 * (max. 55MHZ EBC clock)
 */
/* Memory Bank 0 (NOR flash) BAS=0xFE0,BS=32MB,BU=R/W,BW=16bit */
#define CONFIG_SYS_EBC_PB0AP		0x03017200
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH_BASE | 0xba000)

/* Memory Bank 1 (CPLD) BAS=0xEF0,BS=16MB,BU=R/W,BW=16bit */
#define CONFIG_SYS_CPLD_BASE		0xef000000
#define CONFIG_SYS_EBC_PB1AP		0x00800000
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_CPLD_BASE | 0x18000)

/*
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	  1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
/* inside SDRAM */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR
/* End of used area in RAM */
#define CONFIG_SYS_INIT_RAM_SIZE		CONFIG_SYS_OCM_DATA_SIZE

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * GPIO Configuration
 */
#define CONFIG_SYS_4xx_GPIO_TABLE {                  /* GPIO    Alt1       */ \
{                                                                             \
/* GPIO Core 0 */                                                             \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO0   PerBLast   */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO1   TS1E       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO2   TS2E       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO3   TS1O       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO4   TS2O       */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1 },      /* GPIO5   TS3        */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1 },      /* GPIO6   TS4        */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1 },      /* GPIO7   TS5        */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO8   TS6        */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1 },      /* GPIO9   TrcClk     */ \
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO10  PerCS1     */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO11  PerCS2     */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO12  PerCS3     */ \
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO13  PerCS4     */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO14  PerAddr03  */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO15  PerAddr04  */ \
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO16  PerAddr05  */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO17  IRQ0       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO18  IRQ1       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO19  IRQ2       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO20  IRQ3       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO21  IRQ4       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO22  IRQ5       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO23  IRQ6       */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO24  UART0_DCD  */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO25  UART0_DSR  */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO26  UART0_RI   */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO27  UART0_DTR  */ \
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO28  UART1_Rx   */ \
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO29  UART1_Tx   */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO30  RejectPkt0 */ \
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO31  RejectPkt1 */ \
}                                                                             \
}

#define CONFIG_SYS_GPIO_HWREV_MASK	(0xf0000000 >> 1)	/* GPIO1..4 */
#define CONFIG_SYS_GPIO_HWREV_SHIFT	27
#define CONFIG_SYS_GPIO_LEDRUN_N	(0x80000000 >> 5)	/* GPIO5 */
#define CONFIG_SYS_GPIO_LEDA_N		(0x80000000 >> 6)	/* GPIO6 */
#define CONFIG_SYS_GPIO_LEDB_N		(0x80000000 >> 7)	/* GPIO7 */
#define CONFIG_SYS_GPIO_SELFRST_N	(0x80000000 >> 8)	/* GPIO8 */
#define CONFIG_SYS_GPIO_EEPROM_WP	(0x80000000 >> 9)	/* GPIO9 */
#define CONFIG_SYS_GPIO_MONARCH_N	(0x80000000 >> 11)	/* GPIO11 */
#define CONFIG_SYS_GPIO_EREADY		(0x80000000 >> 12)	/* GPIO12 */
#define CONFIG_SYS_GPIO_M66EN		(0x80000000 >> 13)	/* GPIO13 */

/*
 * Default speed selection (cpu_plb_opb_ebc) in mhz.
 * This value will be set if iic boot eprom is disabled.
 */
#undef CONFIG_SYS_FCPU333MHZ
#define CONFIG_SYS_FCPU266MHZ
#undef CONFIG_SYS_FCPU133MHZ

#if defined(CONFIG_SYS_FCPU333MHZ)
/*
 * CPU: 333MHz
 * PLB/SDRAM/MAL: 111MHz
 * OPB: 55MHz
 * EBC: 55MHz
 * PCI: 55MHz (111MHz on M66EN=1)
 */
#define PLLMR0_DEFAULT (PLL_CPUDIV_1 | PLL_PLBDIV_3 |		\
			PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |	\
			PLL_MALDIV_1 | PLL_PCIDIV_2)
#define PLLMR1_DEFAULT (PLL_FBKDIV_10  |			\
			PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |		\
			PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)
#endif

#if defined(CONFIG_SYS_FCPU266MHZ)
/*
 * CPU: 266MHz
 * PLB/SDRAM/MAL: 133MHz
 * OPB: 66MHz
 * EBC: 44MHz
 * PCI: 44MHz (66MHz on M66EN=1)
 */
#define PLLMR0_DEFAULT (PLL_CPUDIV_1 | PLL_PLBDIV_2 |		\
			PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
			PLL_MALDIV_1 | PLL_PCIDIV_3)
#define PLLMR1_DEFAULT (PLL_FBKDIV_8  |  \
			PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |		\
			PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#endif

#if defined(CONFIG_SYS_FCPU133MHZ)
/*
 * CPU: 133MHz
 * PLB/SDRAM/MAL: 133MHz
 * OPB: 66MHz
 * EBC: 44MHz
 * PCI: 44MHz (66MHz on M66EN=1)
 */
#define PLLMR0_DEFAULT (PLL_CPUDIV_1 | PLL_PLBDIV_1 |		\
			PLL_OPBDIV_2 | PLL_EXTBUSDIV_3 |	\
			PLL_MALDIV_1 | PLL_PCIDIV_3)
#define PLLMR1_DEFAULT (PLL_FBKDIV_4  |  \
			PLL_FWDDIVA_6 | PLL_FWDDIVB_6 |		\
			PLL_TUNE_15_M_40 | PLL_TUNE_VCO_LOW)
#endif

#endif /* __CONFIG_H */
