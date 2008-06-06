/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405EP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family   */
#define CONFIG_BUBINGA	        1	/* ...on a BUBINGA board	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		bubinga
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

#define CONFIG_SYS_CLK_FREQ     33333333 /* external frequency to pll   */

#define CONFIG_NO_SERIAL_EEPROM
/*#undef CONFIG_NO_SERIAL_EEPROM*/
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_NO_SERIAL_EEPROM

/*
!-------------------------------------------------------------------------------
! Defines for entry options.
! Note: Because the 405EP SDRAM controller does not support ECC, ECC DIMMs that
!       are plugged in the board will be utilized as non-ECC DIMMs.
!-------------------------------------------------------------------------------
*/
#define        AUTO_MEMORY_CONFIG
#define        DIMM_READ_ADDR 0xAB
#define        DIMM_WRITE_ADDR 0xAA

/*
!-------------------------------------------------------------------------------
! PLL settings for 266MHz CPU, 133MHz PLB/SDRAM, 66MHz EBC, 33MHz PCI,
! assuming a 33MHz input clock to the 405EP from the C9531.
!-------------------------------------------------------------------------------
*/
#define PLLMR0_DEFAULT   PLLMR0_266_133_66
#define PLLMR1_DEFAULT   PLLMR1_266_133_66

#endif
/*----------------------------------------------------------------------------*/

/*
 * Define here the location of the environment variables (FLASH or NVRAM).
 * Note: DENX encourages to use redundant environment in FLASH. NVRAM is only
 *       supported for backward compatibility.
 */
#if 1
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_NVRAM	1	/* use NVRAM for environment vars	*/
#endif

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_PPC						\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fff80000\0"					\
	"ramdisk_addr=fff90000\0"					\
	""

#define	CONFIG_PHY_ADDR		1	/* PHY address			*/
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	2	/* EMAC1 PHY address		*/

#define CONFIG_RTC_DS174x	1	/* use DS1743 RTC in Bubinga	*/

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#define CONFIG_SPD_EEPROM      1       /* use SPD EEPROM for setup    */

/*
 * If CFG_EXT_SERIAL_CLOCK, then the UART divisor is 1.
 * If CFG_405_UART_ERRATA_59, then UART divisor is 31.
 * Otherwise, UART divisor is determined by CPU Clock and CFG_BASE_BAUD value.
 * The Linux BASE_BAUD define should match this configuration.
 *    baseBaud = cpuClock/(uartDivisor*16)
 * If CFG_405_UART_ERRATA_59 and 200MHz CPU clock,
 * set Linux BASE_BAUD to 403200.
 */
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#undef  CFG_EXT_SERIAL_CLOCK           /* external serial clock */
#undef  CFG_405_UART_ERRATA_59         /* 405GP/CR Rev. D silicon */
#define CFG_BASE_BAUD       691200

/*-----------------------------------------------------------------------
 * I2C stuff
 *-----------------------------------------------------------------------
 */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/

#define CFG_I2C_NOPROBES	{ 0x69 }	/* avoid iprobe hangup (why?) */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	6	/* 24C02 requires 5ms delay */

#if defined(CONFIG_CMD_EEPROM)
#define CFG_I2C_EEPROM_ADDR	0x50	/* I2C boot EEPROM (24C02W)	*/
#define CFG_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/
#endif

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0              /* configure ar pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_HOST	PCI_HOST_FORCE  /* select pci host function     */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
					/* resource configuration       */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */
#define CFG_PCI_CLASSCODE       0x0600  /* PCI Class Code: bridge/host  */
#define CFG_PCI_PTM1LA  0x00000000      /* point to sdram               */
#define CFG_PCI_PTM1MS  0x80000001      /* 2GB, enable hard-wired to 1  */
#define CFG_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CFG_PCI_PTM2LA  0x00000000      /* disabled                     */
#define CFG_PCI_PTM2MS  0x00000000      /* disabled                     */
#define CFG_PCI_PTM2PCI 0x04000000      /* Host: use this pci address   */

/*-----------------------------------------------------------------------
 * External peripheral base address
 *-----------------------------------------------------------------------
 */
#define	CFG_KEY_REG_BASE_ADDR	0xF0100000
#define	CFG_IR_REG_BASE_ADDR	0xF0200000
#define	CFG_FPGA_REG_BASE_ADDR	0xF0300000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 */
#define CFG_SRAM_BASE		0xFFF00000
#define CFG_FLASH_BASE		0xFFF80000

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_ADDR0         0x5555
#define CFG_FLASH_ADDR1         0x2aaa
#define CFG_FLASH_WORD_SIZE     unsigned char

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_NVRAM_BASE_ADDR	0xf0000000	/* NVRAM base address	*/
#define CFG_NVRAM_SIZE		0x1ff8		/* NVRAM size	*/

#ifdef CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_SIZE		0x0ff8		/* Size of Environment vars	*/
#define CFG_ENV_ADDR		\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE)	/* Env	*/
#endif

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0		/* FLASH bank #1	*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM        1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/

#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash/SRAM) initialization                                    */
#define CFG_EBC_PB0AP           0x04006000
#define CFG_EBC_PB0CR           0xFFF18000  /* BAS=0xFFF,BS=1MB,BU=R/W,BW=8bit  */

/* Memory Bank 1 (NVRAM/RTC) initialization                                     */
#define CFG_EBC_PB1AP           0x04041000
#define CFG_EBC_PB1CR           0xF0018000  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit  */

/* Memory Bank 2 (not used) initialization                                      */
#define CFG_EBC_PB2AP           0x00000000
#define CFG_EBC_PB2CR           0x00000000

/* Memory Bank 2 (not used) initialization                                      */
#define CFG_EBC_PB3AP           0x00000000
#define CFG_EBC_PB3CR           0x00000000

/* Memory Bank 4 (FPGA regs) initialization                                     */
#define CFG_EBC_PB4AP           0x01815000
#define CFG_EBC_PB4CR           0xF0318000  /* BAS=0xF03,BS=1MB,BU=R/W,BW=8bit  */

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS      0x55

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EP specific)
 *
 * GPIO0[0]     - External Bus Controller BLAST output
 * GPIO0[1-9]   - Instruction trace outputs
 * GPIO0[10-13] - External Bus Controller CS_1 - CS_4 outputs
 * GPIO0[14-16] - External Bus Controller ABUS3-ABUS5 outputs
 * GPIO0[17-23] - External Interrupts IRQ0 - IRQ6 inputs
 * GPIO0[24-27] - UART0 control signal inputs/outputs
 * GPIO0[28-29] - UART1 data signal input/output
 * GPIO0[30-31] - EMAC0 and EMAC1 reject packet inputs
 */
#define CFG_GPIO0_OSRH          0x55555555
#define CFG_GPIO0_OSRL          0x40000110
#define CFG_GPIO0_ISR1H         0x00000000
#define CFG_GPIO0_ISR1L         0x15555445
#define CFG_GPIO0_TSRH          0x00000000
#define CFG_GPIO0_TSRL          0x00000000
#define CFG_GPIO0_TCR           0xFFFF8014

/*-----------------------------------------------------------------------
 * Some BUBINGA stuff...
 */
#define NVRAM_BASE      0xF0000000
#define FPGA_REG0       0xF0300000    /* FPGA Reg 0              */
#define FPGA_REG1       0xF0300001    /* FPGA Reg 1              */
#define NVRVFY1     0x4f532d4f    /* used to determine if state data in */
#define NVRVFY2     0x50454e00    /* NVRAM initialized (ascii for OS-OPEN)*/

#define FPGA_REG0_F_RANGE     0x80       /* SDRAM PLL freq range              */
#define FPGA_REG0_EXT_INT_DIS 0x20       /* External interface disable        */
#define FPGA_REG0_LED_MASK    0x07       /* Board LEDs DS9, DS10, and DS11    */
#define FPGA_REG0_LED0        0x04       /* Turn on LED0                      */
#define FPGA_REG0_LED1        0x02       /* Turn on LED1                      */
#define FPGA_REG0_LED2        0x01       /* Turn on LED2                      */

#define FPGA_REG1_SSPEC_DIS   0x80       /* C9531 Spread Spectrum disabled    */
#define FPGA_REG1_OFFBD_PCICLK 0x40      /* Onboard PCI clock selected       */
#define FPGA_REG1_CLOCK_MASK  0x30       /* Mask for C9531 output freq select */
#define FPGA_REG1_CLOCK_BIT_SHIFT  4
#define FPGA_REG1_PCI_INT_ARB 0x08       /* PCI Internal arbiter selected     */
#define FPGA_REG1_PCI_FREQ    0x04       /* PCI Frequency select              */
#define FPGA_REG1_OFFB_FLASH  0x02       /* Off board flash                   */
#define FPGA_REG1_SRAM_BOOT   0x01       /* SRAM at 0xFFF80000 not Flash      */

#endif	/* __CONFIG_H */
