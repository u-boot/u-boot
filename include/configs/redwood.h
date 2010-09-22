/*
 * Configuration for AMCC 460SX Ref (redwood)
 *
 * (C) Copyright 2008
 * Feng Kan, Applied Micro Circuits Corp., fkan@amcc.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_4xx			1	/* ... PPC4xx family	*/
#define CONFIG_440			1	/* ... PPC460 family	*/
#define CONFIG_460SX			1	/* ... PPC460 family	*/
#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init	*/

/*-----------------------------------------------------------------------
 * Include common defines/options for all AMCC boards
 *----------------------------------------------------------------------*/
#define CONFIG_HOSTNAME		redwood

#include "amcc-common.h"

#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0xfff00000	/* start of FLASH	*/
#define CONFIG_SYS_PERIPHERAL_BASE	0xa0000000	/* internal peripherals	*/
#define CONFIG_SYS_ISRAM_BASE		0x90000000	/* internal SRAM	*/

#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs	*/

#define CONFIG_SYS_PCIE_MEMBASE	0x90000000	/* mapped PCIe memory	*/
#define CONFIG_SYS_PCIE0_MEMBASE	0x90000000	/* mapped PCIe memory	*/
#define CONFIG_SYS_PCIE1_MEMBASE	0xa0000000	/* mapped PCIe memory	*/
#define CONFIG_SYS_PCIE_MEMSIZE	0x01000000

#define CONFIG_SYS_PCIE0_XCFGBASE	0xb0000000
#define CONFIG_SYS_PCIE1_XCFGBASE	0xb2000000
#define CONFIG_SYS_PCIE2_XCFGBASE	0xb4000000
#define CONFIG_SYS_PCIE0_CFGBASE	0xb6000000
#define CONFIG_SYS_PCIE1_CFGBASE	0xb8000000
#define CONFIG_SYS_PCIE2_CFGBASE	0xba000000

/* PCIe mapped UTL registers */
#define CONFIG_SYS_PCIE0_REGBASE   0xd0000000
#define CONFIG_SYS_PCIE1_REGBASE   0xd0010000
#define CONFIG_SYS_PCIE2_REGBASE   0xd0020000

/* System RAM mapped to PCI space */
#define CONFIG_PCI_SYS_MEM_BUS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#define CONFIG_SYS_FPGA_BASE		0xe2000000	/* epld			*/
#define CONFIG_SYS_OPER_FLASH		0xe7000000	/* SRAM - OPER Flash	*/

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_TEMP_STACK_OCM	1
#define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_ISRAM_BASE	/* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_END	0x2000		/* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* num bytes initial data */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define CONFIG_DDR_ECC		1	/* with ECC support		*/

#define CONFIG_SYS_SPD_MAX_DIMMS	2

/* SPD i2c spd addresses */
#define SPD_EEPROM_ADDRESS     {IIC0_DIMM0_ADDR, IIC0_DIMM1_ADDR}
#define IIC0_DIMM0_ADDR		       0x53
#define IIC0_DIMM1_ADDR		       0x52

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed			*/

#define IIC0_BOOTPROM_ADDR	0x50
#define IIC0_ALT_BOOTPROM_ADDR	0x54

/* Don't probe these addrs */
#define CONFIG_SYS_I2C_NOPROBES	{0x50, 0x52, 0x53, 0x54}

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address		*/

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#undef	CONFIG_ENV_IS_IN_NVRAM		/* ... not in NVRAM		*/
#define	CONFIG_ENV_IS_IN_FLASH	1	/* Environment uses flash	*/
#undef	CONFIG_ENV_IS_IN_EEPROM		/* ... not in EEPROM		*/

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	CONFIG_AMCC_DEF_ENV_NAND_UPD					\
	"kernel_addr=fc000000\0"					\
	"fdt_addr=fc1e0000\0"						\
	"ramdisk_addr=fc200000\0"					\
	""

/*----------------------------------------------------------------------------+
| Commands in addition to amcc-common.h
+----------------------------------------------------------------------------*/
#define CONFIG_CMD_SDRAM

#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_RESET_DELAY	1000
#define CONFIG_M88E1141_PHY	1	/* Enable phy */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR	1	/* PHY address, See schematics	*/

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_CFI_AMD_RESET 1	/* Use AMD (Spansion) reset cmd */

#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* number of banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* sectors per device		*/

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		0xfffa0000
#define CONFIG_ENV_SIZE		0x10000	/* Size of Environment vars	*/
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*---------------------------------------------------------------------------*/

#endif	/* __CONFIG_H */
