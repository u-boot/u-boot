/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/************************************************************************
 * acadia.h - configuration for AMCC Acadia (405EZ)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_ACADIA		1		/* Board is Acadia	*/
#define CONFIG_405EZ		1		/* Specifc 405EZ support*/

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF80000
#endif

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		acadia
#include "amcc-common.h"

/* Detect Acadia PLL input clock automatically via CPLD bit		*/
#define CONFIG_SYS_CLK_FREQ    ((in8(CONFIG_SYS_CPLD_BASE + 0) == 0x0c) ? \
				66666666 : 33333000)

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_F	1		/* Call misc_init_f	*/

#define CONFIG_NO_SERIAL_EEPROM
/*#undef CONFIG_NO_SERIAL_EEPROM*/

#ifdef CONFIG_NO_SERIAL_EEPROM
/*----------------------------------------------------------------------------
 * PLL settings for 266MHz CPU, 133MHz PLB/SDRAM, 66MHz EBC, 33MHz PCI,
 * assuming a 66MHz input clock to the 405EZ.
 *---------------------------------------------------------------------------*/
/* #define PLLMR0_100_100_12 */
#define PLLMR0_200_133_66
/* #define PLLMR0_266_160_80 */
/* #define PLLMR0_333_166_83 */
#endif

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0xfe000000
#define CONFIG_SYS_CPLD_BASE		0x80000000
#define CONFIG_SYS_NAND_ADDR		0xd0000000
#define CONFIG_SYS_USB_HOST		0xef603000	/* USB OHCI 1.1 controller	*/

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_TEMP_STACK_OCM	1		/* OCM as init ram	*/

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xf8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x4000			/* 16K of onchip SRAM		*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR	/* inside of SRAM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE	/* Size of used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#undef	CONFIG_SYS_EXT_SERIAL_CLOCK			/* external serial clock */
#define CONFIG_SYS_BASE_BAUD		691200

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
#define CONFIG_ENV_SECT_SIZE	0x40000 /* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * RAM (CRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MBYTES_RAM		64		/* 64MB			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000

#define CONFIG_SYS_I2C_EEPROM_ADDR	(0xa8>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_AD7414	1		/* use AD7414		*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define	CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_HAS_ETH0		1

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fff10000\0"					\
	"ramdisk_addr=fff20000\0"					\
	"kozio=bootm ffc60000\0"					\
	""

#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_SUPPORT_VFAT

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DTT
#define CONFIG_CMD_NAND

/*-----------------------------------------------------------------------
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_ADDR + CONFIG_SYS_NAND_CS)
#define CONFIG_SYS_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_NAND_CS		3
/* Memory Bank 0 (Flash) initialization						*/
#define CONFIG_SYS_EBC_PB0AP		0x03337200
#define CONFIG_SYS_EBC_PB0CR		0xfe0bc000

/* Memory Bank 3 (NAND-FLASH) initialization					*/
#define CONFIG_SYS_EBC_PB3AP		0x018003c0
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_NAND_ADDR | 0x1c000)

/* Just initial configuration for CRAM. Will be changed in memory.c to sync mode*/
/* Memory Bank 1 (CRAM) initialization						*/
#define CONFIG_SYS_EBC_PB1AP		0x030400c0
#define CONFIG_SYS_EBC_PB1CR		0x000bc000

/* Memory Bank 2 (CRAM) initialization						*/
#define CONFIG_SYS_EBC_PB2AP		0x030400c0
#define CONFIG_SYS_EBC_PB2CR		0x020bc000

/* Memory Bank 4 (CPLD) initialization						*/
#define CONFIG_SYS_EBC_PB4AP		0x04006000
#define CONFIG_SYS_EBC_PB4CR		(CONFIG_SYS_CPLD_BASE | 0x18000)

#define CONFIG_SYS_EBC_CFG		0xf8400000

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_GPIO_CRAM_CLK	8
#define CONFIG_SYS_GPIO_CRAM_WAIT	9		/* GPIO-In		*/
#define CONFIG_SYS_GPIO_CRAM_ADV	10
#define CONFIG_SYS_GPIO_CRAM_CRE	(32 + 21)	/* GPIO-Out		*/

/*-----------------------------------------------------------------------
 * Definitions for GPIO_0 setup (PPC405EZ specific)
 *
 * GPIO0[0-2]	- External Bus Controller CS_4 - CS_6 Outputs
 * GPIO0[3]	- NAND FLASH Controller CE3 (NFCE3) Output
 * GPIO0[4]	- External Bus Controller Hold Input
 * GPIO0[5]	- External Bus Controller Priority Input
 * GPIO0[6]	- External Bus Controller HLDA Output
 * GPIO0[7]	- External Bus Controller Bus Request Output
 * GPIO0[8]	- CRAM Clk Output
 * GPIO0[9]	- External Bus Controller Ready Input
 * GPIO0[10]	- CRAM Adv Output
 * GPIO0[11-24]	- NAND Flash Control Data -> Bypasses GPIO when enabled
 * GPIO0[25]	- External DMA Request Input
 * GPIO0[26]	- External DMA EOT I/O
 * GPIO0[25]	- External DMA Ack_n Output
 * GPIO0[17-23]	- External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[28-30]	- Trace Outputs / PWM Inputs
 * GPIO0[31]	- PWM_8 I/O
 */
#define CONFIG_SYS_GPIO0_TCR		0xC0A00000
#define CONFIG_SYS_GPIO0_OSRL		0x50004400
#define CONFIG_SYS_GPIO0_OSRH		0x02000055
#define CONFIG_SYS_GPIO0_ISR1L		0x00001000
#define CONFIG_SYS_GPIO0_ISR1H		0x00000055
#define CONFIG_SYS_GPIO0_TSRL		0x02000000
#define CONFIG_SYS_GPIO0_TSRH		0x00000055

/*-----------------------------------------------------------------------
 * Definitions for GPIO_1 setup (PPC405EZ specific)
 *
 * GPIO1[0-6]	- PWM_9 to PWM_15 I/O
 * GPIO1[7]	- PWM_DIV_CLK (Out) / IRQ4 Input
 * GPIO1[8]	- TS5 Output / DAC_IP_TRIG Input
 * GPIO1[9]	- TS6 Output / ADC_IP_TRIG Input
 * GPIO1[10-12]	- UART0 Control Inputs
 * GPIO1[13]	- UART0_DTR_N Output/IEEE_1588_TS Output/TMRCLK Input
 * GPIO1[14]	- UART0_RTS_N Output/SPI_SS_2_N Output
 * GPIO1[15]	- SPI_SS_3_N Output/UART0_RI_N Input
 * GPIO1[16]	- SPI_SS_1_N Output
 * GPIO1[17-20]	- Trace Output/External Interrupts IRQ0 - IRQ3 inputs
 */
#define CONFIG_SYS_GPIO1_TCR		0xFFFF8414
#define CONFIG_SYS_GPIO1_OSRL		0x40000110
#define CONFIG_SYS_GPIO1_OSRH		0x55455555
#define CONFIG_SYS_GPIO1_ISR1L		0x15555445
#define CONFIG_SYS_GPIO1_ISR1H		0x00000000
#define CONFIG_SYS_GPIO1_TSRL		0x00000000
#define CONFIG_SYS_GPIO1_TSRH		0x00000000

#endif	/* __CONFIG_H */
