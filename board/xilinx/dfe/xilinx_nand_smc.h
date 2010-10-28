/* arch/arm/mach-xilinx/include/mach/smc.h
 *
 *  Copyright (C) 2009 Xilinx, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef __ASM_ARCH_SMC_H__
#define __ASM_ARCH_SMC_H__

/*
 * This file smc.h in \arch\arm\mach-xilinx\include\mach folder is designed to
 * provide same register set definitions to NAND and NOR flash driver
 */

/* Memory controller configuration register offset */
#define XSMCPSS_MC_STATUS		0x000	/* Controller status reg, RO */
#define XSMCPSS_MC_INTERFACE_CONFIG	0x004	/* Interface config reg, RO */
#define XSMCPSS_MC_SET_CONFIG		0x008	/* Set configuration reg, WO */
#define XSMCPSS_MC_CLR_CONFIG		0x00C	/* Clear config reg, WO */
#define XSMCPSS_MC_DIRECT_CMD		0x010	/* Direct command reg, WO */
#define XSMCPSS_MC_SET_CYCLES		0x014	/* Set cycles register, WO */
#define XSMCPSS_MC_SET_OPMODE		0x018	/* Set opmode register, WO */
#define XSMCPSS_MC_REFRESH_PERIOD_0	0x020	/* Refresh period_0 reg, RW */
#define XSMCPSS_MC_REFRESH_PERIOD_1	0x024	/* Refresh period_1 reg, RW */

/* Chip select configuration register offset */
#define XSMCPSS_CS_IF0_CHIP_0_OFFSET	0x100	/* Interface 0 chip 0 config */
#define XSMCPSS_CS_IF0_CHIP_1_OFFSET	0x120	/* Interface 0 chip 1 config */
#define XSMCPSS_CS_IF0_CHIP_2_OFFSET	0x140	/* Interface 0 chip 2 config */
#define XSMCPSS_CS_IF0_CHIP_3_OFFSET	0x160	/* Interface 0 chip 3 config */
#define XSMCPSS_CS_IF1_CHIP_0_OFFSET	0x180	/* Interface 1 chip 0 config */
#define XSMCPSS_CS_IF1_CHIP_1_OFFSET	0x1A0	/* Interface 1 chip 1 config */
#define XSMCPSS_CS_IF1_CHIP_2_OFFSET	0x1C0	/* Interface 1 chip 2 config */
#define XSMCPSS_CS_IF1_CHIP_3_OFFSET	0x1E0	/* Interface 1 chip 3 config */

/* User configuration register offset */
#define XSMCPSS_UC_STATUS_OFFSET	0x200	/* User status reg, RO */
#define XSMCPSS_UC_CONFIG_OFFSET	0x204	/* User config reg, WO */

/* Integration test register offset */
#define XSMCPSS_IT_OFFSET		0xE00

/* ID configuration register offset */
#define XSMCPSS_ID_PERIP_0_OFFSET	0xFE0
#define XSMCPSS_ID_PERIP_1_OFFSET	0xFE4
#define XSMCPSS_ID_PERIP_2_OFFSET	0xFE8
#define XSMCPSS_ID_PERIP_3_OFFSET	0xFEC
#define XSMCPSS_ID_PCELL_0_OFFSET	0xFF0
#define XSMCPSS_ID_PCELL_1_OFFSET	0xFF4
#define XSMCPSS_ID_PCELL_2_OFFSET	0xFF8
#define XSMCPSS_ID_PCELL_3_OFFSET	0xFFC

/* Add ECC reg, nand_cycles, sram_cycles and opmode_x_n registers
   There are multiple interfaces also we need to take care.*/

/* Register definitions */
/* ECC register offset */
#define XSMCPSS_ECC_IF0_OFFSET		0x300   /* Interface 0 ECC register */
#define XSMCPSS_ECC_IF1_OFFSET		0x400   /* Interface 1 ECC register */
#define XSMCPSS_ECC_STATUS_OFFSET(addr)	(0x000 + addr) /* ECC status register */
#define XSMCPSS_ECC_MEMCFG_OFFSET(addr)	(0x004 + addr) /* ECC mem config reg */
#define XSMCPSS_ECC_MEMCMD1_OFFSET(addr)(0x008 + addr) /*ECC mem cmd1 reg*/
#define XSMCPSS_ECC_MEMCMD2_OFFSET(addr)(0x00C + addr) /*ECC mem cmd2 reg*/
#define XSMCPSS_ECC_ADDR0_OFFSET(addr)	(0x010 + addr) /* ECC address0 reg */
#define XSMCPSS_ECC_ADDR1_OFFSET(addr)	(0x014 + addr) /* ECC address1 reg */
#define XSMCPSS_ECC_VALUE0_OFFSET(addr)	(0x018 + addr) /* ECC value 0 reg */
#define XSMCPSS_ECC_VALUE1_OFFSET(addr)	(0x01C + addr) /* ECC value 1 reg */
#define XSMCPSS_ECC_VALUE2_OFFSET(addr)	(0x020 + addr) /* ECC value 2 reg */
#define XSMCPSS_ECC_VALUE3_OFFSET(addr)	(0x024 + addr) /* ECC value 3 reg */
#define XSMCPSS_ECC_VALUE4_OFFSET(addr)	(0x028 + addr) /* ECC value 4 reg */

/* nand_cycles, sram_cycles and opmode_x_n registers offsets*/
#define XSMCPSS_FLASH_CYCLES(addr)	(0x000 + addr) /* NAND and SRAM cycle */
#define XSMCPSS_OPMODE(addr)		(0x004 + addr) /* Chip opmode reg */

#endif /* __ASM_ARCH_SMC_H__ */
