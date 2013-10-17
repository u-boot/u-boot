/*****************************************************************************
 * (C) Copyright 2003;  Tundra Semiconductor Corp.
 * (C) Copyright 2006;  Freescale Semiconductor Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *****************************************************************************/

/*
 * FILENAME: tsi108.h
 *
 * Originator: Alex Bounine
 *
 * DESCRIPTION:
 * Common definitions for the Tundra Tsi108 bridge chip
 *
 */

#ifndef _TSI108_H_
#define _TSI108_H_

#define TSI108_HLP_REG_OFFSET	(0x0000)
#define TSI108_PCI_REG_OFFSET	(0x1000)
#define TSI108_CLK_REG_OFFSET	(0x2000)
#define TSI108_PB_REG_OFFSET	(0x3000)
#define TSI108_SD_REG_OFFSET	(0x4000)
#define TSI108_MPIC_REG_OFFSET	(0x7400)

#define PB_ID			(0x000)
#define PB_RSR			(0x004)
#define PB_BUS_MS_SELECT	(0x008)
#define PB_ISR			(0x00C)
#define PB_ARB_CTRL		(0x018)
#define PB_PVT_CTRL2		(0x034)
#define PB_SCR			(0x400)
#define PB_ERRCS		(0x404)
#define PB_AERR			(0x408)
#define PB_REG_BAR		(0x410)
#define PB_OCN_BAR1		(0x414)
#define PB_OCN_BAR2		(0x418)
#define PB_SDRAM_BAR1		(0x41C)
#define PB_SDRAM_BAR2		(0x420)
#define PB_MCR			(0xC00)
#define PB_MCMD			(0xC04)

#define HLP_B0_ADDR		(0x000)
#define HLP_B1_ADDR		(0x010)
#define HLP_B2_ADDR		(0x020)
#define HLP_B3_ADDR		(0x030)

#define HLP_B0_MASK		(0x004)
#define HLP_B1_MASK		(0x014)
#define HLP_B2_MASK		(0x024)
#define HLP_B3_MASK		(0x034)

#define HLP_B0_CTRL0		(0x008)
#define HLP_B1_CTRL0		(0x018)
#define HLP_B2_CTRL0		(0x028)
#define HLP_B3_CTRL0		(0x038)

#define HLP_B0_CTRL1		(0x00C)
#define HLP_B1_CTRL1		(0x01C)
#define HLP_B2_CTRL1		(0x02C)
#define HLP_B3_CTRL1		(0x03C)

#define PCI_CSR			(0x004)
#define PCI_P2O_BAR0		(0x010)
#define PCI_P2O_BAR0_UPPER	(0x014)
#define PCI_P2O_BAR2		(0x018)
#define PCI_P2O_BAR2_UPPER	(0x01C)
#define PCI_P2O_BAR3		(0x020)
#define PCI_P2O_BAR3_UPPER	(0x024)

#define PCI_MISC_CSR		(0x040)
#define PCI_P2O_PAGE_SIZES	(0x04C)

#define PCI_PCIX_STAT		(0x0F4)

#define PCI_IRP_STAT		(0x184)

#define PCI_PFAB_BAR0		(0x204)
#define PCI_PFAB_BAR0_UPPER	(0x208)
#define PCI_PFAB_IO		(0x20C)
#define PCI_PFAB_IO_UPPER	(0x210)

#define PCI_PFAB_MEM32		(0x214)
#define PCI_PFAB_MEM32_REMAP	(0x218)
#define PCI_PFAB_MEM32_MASK	(0x21C)

#define CG_PLL0_CTRL0		(0x210)
#define CG_PLL0_CTRL1		(0x214)
#define CG_PLL1_CTRL0		(0x220)
#define CG_PLL1_CTRL1		(0x224)
#define CG_PWRUP_STATUS		(0x234)

#define MPIC_CSR(n) (0x30C + (n * 0x40))

#define SD_CTRL			(0x000)
#define SD_STATUS		(0x004)
#define SD_TIMING		(0x008)
#define SD_REFRESH		(0x00C)
#define SD_INT_STATUS		(0x010)
#define SD_INT_ENABLE		(0x014)
#define SD_INT_SET		(0x018)
#define SD_D0_CTRL		(0x020)
#define SD_D1_CTRL		(0x024)
#define SD_D0_BAR		(0x028)
#define SD_D1_BAR		(0x02C)
#define SD_ECC_CTRL		(0x040)
#define SD_DLL_STATUS		(0x250)

#define TS_SD_CTRL_ENABLE	(1 << 31)

#define PB_ERRCS_ES		(1 << 1)
#define PB_ISR_PBS_RD_ERR	(1 << 8)
#define PCI_IRP_STAT_P_CSR	(1 << 23)

/*
 * I2C : Register address offset definitions
 */
#define I2C_CNTRL1		(0x00000000)
#define I2C_CNTRL2		(0x00000004)
#define I2C_RD_DATA		(0x00000008)
#define I2C_TX_DATA		(0x0000000c)

/*
 * I2C : Register Bit Masks and Reset Values
 * definitions for every register
 */

/* I2C_CNTRL1 : Reset Value */
#define I2C_CNTRL1_RESET_VALUE				(0x0000000a)

/* I2C_CNTRL1 : Register Bits Masks Definitions */
#define I2C_CNTRL1_DEVCODE				(0x0000000f)
#define I2C_CNTRL1_PAGE					(0x00000700)
#define I2C_CNTRL1_BYTADDR				(0x00ff0000)
#define I2C_CNTRL1_I2CWRITE				(0x01000000)

/* I2C_CNTRL1 : Read/Write Bit Mask Definition */
#define I2C_CNTRL1_RWMASK				(0x01ff070f)

/* I2C_CNTRL1 : Unused/Reserved bits Definition */
#define I2C_CNTRL1_RESERVED				(0xfe00f8f0)

/* I2C_CNTRL2 : Reset Value */
#define I2C_CNTRL2_RESET_VALUE				(0x00000000)

/* I2C_CNTRL2 : Register Bits Masks Definitions */
#define I2C_CNTRL2_SIZE					(0x00000003)
#define I2C_CNTRL2_LANE					(0x0000000c)
#define I2C_CNTRL2_MULTIBYTE				(0x00000010)
#define I2C_CNTRL2_START				(0x00000100)
#define I2C_CNTRL2_WR_STATUS				(0x00010000)
#define I2C_CNTRL2_RD_STATUS				(0x00020000)
#define I2C_CNTRL2_I2C_TO_ERR				(0x04000000)
#define I2C_CNTRL2_I2C_CFGERR				(0x08000000)
#define I2C_CNTRL2_I2C_CMPLT				(0x10000000)

/* I2C_CNTRL2 : Read/Write Bit Mask Definition */
#define I2C_CNTRL2_RWMASK				(0x0000011f)

/* I2C_CNTRL2 : Unused/Reserved bits Definition */
#define I2C_CNTRL2_RESERVED				(0xe3fcfee0)

/* I2C_RD_DATA : Reset Value */
#define I2C_RD_DATA_RESET_VALUE				(0x00000000)

/* I2C_RD_DATA : Register Bits Masks Definitions */
#define I2C_RD_DATA_RBYTE0				(0x000000ff)
#define I2C_RD_DATA_RBYTE1				(0x0000ff00)
#define I2C_RD_DATA_RBYTE2				(0x00ff0000)
#define I2C_RD_DATA_RBYTE3				(0xff000000)

/* I2C_RD_DATA : Read/Write Bit Mask Definition */
#define I2C_RD_DATA_RWMASK				(0x00000000)

/* I2C_RD_DATA : Unused/Reserved bits Definition */
#define I2C_RD_DATA_RESERVED				(0x00000000)

/* I2C_TX_DATA : Reset Value */
#define I2C_TX_DATA_RESET_VALUE				(0x00000000)

/* I2C_TX_DATA : Register Bits Masks Definitions */
#define I2C_TX_DATA_TBYTE0				(0x000000ff)
#define I2C_TX_DATA_TBYTE1				(0x0000ff00)
#define I2C_TX_DATA_TBYTE2				(0x00ff0000)
#define I2C_TX_DATA_TBYTE3				(0xff000000)

/* I2C_TX_DATA : Read/Write Bit Mask Definition */
#define I2C_TX_DATA_RWMASK				(0xffffffff)

/* I2C_TX_DATA : Unused/Reserved bits Definition */
#define I2C_TX_DATA_RESERVED				(0x00000000)

#define TSI108_I2C_OFFSET	0x7000	/* offset for general use I2C channel */
#define TSI108_I2C_SDRAM_OFFSET	0x4400	/* offset for SPD I2C channel */

#define I2C_EEPROM_DEVCODE	0xA	/* standard I2C EEPROM device code */

/* I2C status codes */

#define TSI108_I2C_SUCCESS	0
#define TSI108_I2C_PARAM_ERR	1
#define TSI108_I2C_TIMEOUT_ERR	2
#define TSI108_I2C_IF_BUSY	3
#define TSI108_I2C_IF_ERROR	4

#endif		/* _TSI108_H_ */
