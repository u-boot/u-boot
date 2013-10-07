/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2005-2007
 * Beijing UD Technology Co., Ltd., taihusupport@amcc.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H


#define CONFIG_405EP		1	/* this is a PPC405 CPU */
#define CONFIG_4xx		1	/*  member of PPC4xx family */
#define CONFIG_TAIHU	        1	/*  on a taihu board */

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		taihu
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f */

#define CONFIG_SYS_CLK_FREQ     33000000 /* external frequency to pll   */

#define CONFIG_NO_SERIAL_EEPROM

/*----------------------------------------------------------------------------*/
#ifdef CONFIG_NO_SERIAL_EEPROM

/*
!-------------------------------------------------------------------------------
! PLL settings for 333MHz CPU, 111MHz PLB/SDRAM, 55MHz EBC, 33MHz PCI,
! assuming a 33MHz input clock to the 405EP from the C9531.
!-------------------------------------------------------------------------------
*/
#define PLLMR0_333_111_55_37 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			      PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			      PLL_MALDIV_1 | PLL_PCIDIV_3)
#define PLLMR1_333_111_55_37 (PLL_FBKDIV_10  |  \
			      PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			      PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)
#define PLLMR0_333_111_55_111 (PLL_CPUDIV_1 | PLL_PLBDIV_3 |  \
			       PLL_OPBDIV_2 | PLL_EXTBUSDIV_2 |  \
			       PLL_MALDIV_1 | PLL_PCIDIV_1)
#define PLLMR1_333_111_55_111 (PLL_FBKDIV_10  |  \
			       PLL_FWDDIVA_3 | PLL_FWDDIVB_3 |  \
			       PLL_TUNE_15_M_40 | PLL_TUNE_VCO_HI)

#define PLLMR0_DEFAULT		PLLMR0_333_111_55_37
#define PLLMR1_DEFAULT		PLLMR1_333_111_55_37
#define PLLMR0_DEFAULT_PCI66	PLLMR0_333_111_55_111
#define PLLMR1_DEFAULT_PCI66	PLLMR1_333_111_55_111

#endif
/*----------------------------------------------------------------------------*/

#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars */

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_PPC						\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=FC000000\0"					\
	"ramdisk_addr=FC180000\0"					\
	""

#define CONFIG_PHY_ADDR		0x14	/* PHY address			*/
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	0x10	/* EMAC1 PHY address		*/
#define CONFIG_PHY_RESET	1

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SPI

#undef CONFIG_SPD_EEPROM		/* use SPD EEPROM for setup */
#define CONFIG_SYS_SDRAM_SIZE_PER_BANK 0x04000000 /* 64MB */
#define CONFIG_SYS_SDRAM_BANKS	        2

/*
 * SDRAM configuration (please see cpu/ppc/sdram.[ch])
 */
#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0 */
#define CONFIG_SDRAM_BANK1	1	/* init onboard SDRAM bank 1 */

/* SDRAM timings used in datasheet */
#define CONFIG_SYS_SDRAM_CL            3	/* CAS latency */
#define CONFIG_SYS_SDRAM_tRP           20	/* PRECHARGE command period */
#define CONFIG_SYS_SDRAM_tRC           66	/* ACTIVE-to-ACTIVE command period */
#define CONFIG_SYS_SDRAM_tRCD          20	/* ACTIVE-to-READ delay */
#define CONFIG_SYS_SDRAM_tRFC		66	/* Auto refresh period */

/*
 * If CONFIG_SYS_EXT_SERIAL_CLOCK, then the UART divisor is 1.
 * If CONFIG_SYS_405_UART_ERRATA_59, then UART divisor is 31.
 * Otherwise, UART divisor is determined by CPU Clock and CONFIG_SYS_BASE_BAUD value.
 * The Linux BASE_BAUD define should match this configuration.
 *    baseBaud = cpuClock/(uartDivisor*16)
 * If CONFIG_SYS_405_UART_ERRATA_59 and 200MHz CPU clock,
 * set Linux BASE_BAUD to 403200.
 */
#define CONFIG_CONS_INDEX	2	/* Use UART1			*/
#undef  CONFIG_SYS_EXT_SERIAL_CLOCK           /* external serial clock */
#undef  CONFIG_SYS_405_UART_ERRATA_59         /* 405GP/CR Rev. D silicon */
#define CONFIG_SYS_BASE_BAUD		691200

/*-----------------------------------------------------------------------
 * I2C stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

#define CONFIG_SYS_I2C_NOPROBES	{ {0, 0x69} } /* avoid i2c probe hangup (?) */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	6 /* 24C02 requires 5ms delay */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* I2C boot EEPROM (24C02W)	*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/

#define CONFIG_SOFT_SPI
#define SPI_SCL  spi_scl
#define SPI_SDA  spi_sda
#define SPI_READ spi_read()
#define SPI_DELAY udelay(2)
#ifndef __ASSEMBLY__
void spi_scl(int);
void spi_sda(int);
unsigned char spi_read(void);
#endif

/* standard dtt sensor configuration */
#define CONFIG_DTT_DS1775	1
#define CONFIG_DTT_SENSORS	{ 0 }
#define CONFIG_SYS_I2C_DTT_ADDR	0x49

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter    */
#define PCI_HOST_FORCE   1		/* configure as pci host       */
#define PCI_HOST_AUTO    2		/* detected via arbiter enable */

#define CONFIG_PCI			/* include pci support	       */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST	PCI_HOST_FORCE  /* select pci host function    */
#define CONFIG_PCI_PNP			/* do pci plug-and-play        */
					/* resource configuration      */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */
#define CONFIG_SYS_PCI_CLASSCODE       0x0600  /* PCI Class Code: bridge/host */
#define CONFIG_SYS_PCI_PTM1LA	    0x00000000	/* point to sdram              */
#define CONFIG_SYS_PCI_PTM1MS      0x80000001	/* 2GB, enable hard-wired to 1 */
#define CONFIG_SYS_PCI_PTM1PCI     0x00000000	/* Host: use this pci address  */
#define CONFIG_SYS_PCI_PTM2LA      0x00000000	/* disabled                    */
#define CONFIG_SYS_PCI_PTM2MS	    0x00000000	/* disabled                    */
#define CONFIG_SYS_PCI_PTM2PCI     0x04000000	/* Host: use this pci address  */
#define CONFIG_EEPRO100		1

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 */
#define CONFIG_SYS_FLASH_BASE		0xFFE00000

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_ADDR0         0x555
#define CONFIG_SYS_FLASH_ADDR1         0x2aa
#define CONFIG_SYS_FLASH_WORD_SIZE     unsigned short

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CONFIG_SYS_NVRAM_BASE_ADDR	0xf0000000	/* NVRAM base address */
#define CONFIG_SYS_NVRAM_SIZE		0x1ff8		/* NVRAM size */

#ifdef CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_SIZE		0x0ff8		/* Size of Environment vars */
#define CONFIG_ENV_ADDR		\
	(CONFIG_SYS_NVRAM_BASE_ADDR+CONFIG_SYS_NVRAM_SIZE-CONFIG_ENV_SIZE)	/* Env*/
#endif

/*-----------------------------------------------------------------------
 * PPC405 GPIO Configuration
 */
#define CONFIG_SYS_4xx_GPIO_TABLE { /*				GPIO	Alternate1		*/	\
{												\
/* GPIO Core 0 */										\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO0	PerBLast    SPI CS	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO1	TS1E			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO2	TS2E			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO3	TS1O			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO4	TS2O			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO5	TS3			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO6	TS4			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO7	TS5			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO8	TS6			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO9	TrcClk			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO10	PerCS1			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO11	PerCS2			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO12	PerCS3			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO13	PerCS4			*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO14	PerAddr03   SPI SCLK	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO15	PerAddr04   SPI DI	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO16	PerAddr05   SPI DO	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO17	IRQ0	    PCI INTA	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO18	IRQ1	    PCI INTB	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO19	IRQ2	    PCI INTC	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO20	IRQ3	    PCI INTD	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO21	IRQ4	    USB		*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO22	IRQ5	    EBC		*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO23	IRQ6	    unused	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO24	UART0_DCD   UART1	*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO25	UART0_DSR		*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO26	UART0_RI		*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO27	UART0_DTR		*/	\
{ GPIO_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO28	UART1_Rx    UART0	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG }, /* GPIO29	UART1_Tx		*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO30	RejectPkt0  User LED1	*/	\
{ GPIO_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG }, /* GPIO31	RejectPkt1  User LED2	*/	\
}												\
}

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM CONFIG_SYS_FLASH_BASE /* FLASH bank #0 */
#define FLASH_BASE1_PRELIM  0xFC000000	/* FLASH bank #1 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory (OCM) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM        1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET      CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash/SRAM) initialization */
#define CONFIG_SYS_EBC_PB0AP           0x03815600
#define CONFIG_SYS_EBC_PB0CR           0xFFE3A000  /* BAS=0xFFE,BS=2MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (NVRAM/RTC) initialization */
#define CONFIG_SYS_EBC_PB1AP           0x05815600
#define CONFIG_SYS_EBC_PB1CR           0xFC0BA000  /* BAS=0xFc0,BS=32MB,BU=R/W,BW=16bit */

/* Memory Bank 2 (USB device) initialization */
#define CONFIG_SYS_EBC_PB2AP           0x03016600
#define CONFIG_SYS_EBC_PB2CR           0x50018000 /* BAS=0x500,BS=1MB,BU=R/W,BW=8bit */

/* Memory Bank 3 (LCM and D-flip-flop) initialization */
#define CONFIG_SYS_EBC_PB3AP           0x158FF600
#define CONFIG_SYS_EBC_PB3CR           0x50118000 /* BAS=0x501,BS=1MB,BU=R/W,BW=8bit */

/* Memory Bank 4 (not install) initialization */
#define CONFIG_SYS_EBC_PB4AP           0x158FF600
#define CONFIG_SYS_EBC_PB4CR           0x5021A000

#define CPLD_REG0_ADDR	0x50100000
#define CPLD_REG1_ADDR	0x50100001

#endif	/* __CONFIG_H */
