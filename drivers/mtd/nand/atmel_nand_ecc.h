/*
 * Error Corrected Code Controller (ECC) - System peripherals regsters.
 * Based on AT91SAM9260 datasheet revision B.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef ATMEL_NAND_ECC_H
#define ATMEL_NAND_ECC_H

#define ATMEL_ECC_CR		0x00			/* Control register */
#define		ATMEL_ECC_RST		(1 << 0)		/* Reset parity */

#define ATMEL_ECC_MR		0x04			/* Mode register */
#define		ATMEL_ECC_PAGESIZE	(3 << 0)		/* Page Size */
#define			ATMEL_ECC_PAGESIZE_528		(0)
#define			ATMEL_ECC_PAGESIZE_1056		(1)
#define			ATMEL_ECC_PAGESIZE_2112		(2)
#define			ATMEL_ECC_PAGESIZE_4224		(3)

#define ATMEL_ECC_SR		0x08			/* Status register */
#define		ATMEL_ECC_RECERR		(1 << 0)		/* Recoverable Error */
#define		ATMEL_ECC_ECCERR		(1 << 1)		/* ECC Single Bit Error */
#define		ATMEL_ECC_MULERR		(1 << 2)		/* Multiple Errors */

#define ATMEL_ECC_PR		0x0c			/* Parity register */
#define		ATMEL_ECC_BITADDR	(0xf << 0)		/* Bit Error Address */
#define		ATMEL_ECC_WORDADDR	(0xfff << 4)		/* Word Error Address */

#define ATMEL_ECC_NPR		0x10			/* NParity register */
#define		ATMEL_ECC_NPARITY	(0xffff << 0)		/* NParity */

/* Register access macros for PMECC */
#define pmecc_readl(addr, reg) \
	readl(&addr->reg)

#define pmecc_writel(addr, reg, value) \
	writel((value), &addr->reg)

/* PMECC Register Definitions */
#define PMECC_MAX_SECTOR_NUM			8
struct pmecc_regs {
	u32 cfg;		/* 0x00 PMECC Configuration Register */
	u32 sarea;		/* 0x04 PMECC Spare Area Size Register */
	u32 saddr;		/* 0x08 PMECC Start Address Register */
	u32 eaddr;		/* 0x0C PMECC End Address Register */
	u32 clk;		/* 0x10 PMECC Clock Control Register */
	u32 ctrl;		/* 0x14 PMECC Control Register */
	u32 sr;			/* 0x18 PMECC Status Register */
	u32 ier;		/* 0x1C PMECC Interrupt Enable Register */
	u32 idr;		/* 0x20 PMECC Interrupt Disable Register */
	u32 imr;		/* 0x24 PMECC Interrupt Mask Register */
	u32 isr;		/* 0x28 PMECC Interrupt Status Register */
	u32 reserved0[5];	/* 0x2C-0x3C Reserved */

	/* 0x40 + sector_num * (0x40), Redundancy Registers */
	struct {
		u8 ecc[44];	/* PMECC Generated Redundancy Byte Per Sector */
		u32 reserved1[5];
	} ecc_port[PMECC_MAX_SECTOR_NUM];

	/* 0x240 + sector_num * (0x40) Remainder Registers */
	struct {
		u32 rem[12];
		u32 reserved2[4];
	} rem_port[PMECC_MAX_SECTOR_NUM];
	u32 reserved3[16];	/* 0x440-0x47C Reserved */
};

/* For PMECC Configuration Register */
#define		PMECC_CFG_BCH_ERR2		(0 << 0)
#define		PMECC_CFG_BCH_ERR4		(1 << 0)
#define		PMECC_CFG_BCH_ERR8		(2 << 0)
#define		PMECC_CFG_BCH_ERR12		(3 << 0)
#define		PMECC_CFG_BCH_ERR24		(4 << 0)

#define		PMECC_CFG_SECTOR512		(0 << 4)
#define		PMECC_CFG_SECTOR1024		(1 << 4)

#define		PMECC_CFG_PAGE_1SECTOR		(0 << 8)
#define		PMECC_CFG_PAGE_2SECTORS		(1 << 8)
#define		PMECC_CFG_PAGE_4SECTORS		(2 << 8)
#define		PMECC_CFG_PAGE_8SECTORS		(3 << 8)

#define		PMECC_CFG_READ_OP		(0 << 12)
#define		PMECC_CFG_WRITE_OP		(1 << 12)

#define		PMECC_CFG_SPARE_ENABLE		(1 << 16)
#define		PMECC_CFG_SPARE_DISABLE		(0 << 16)

#define		PMECC_CFG_AUTO_ENABLE		(1 << 20)
#define		PMECC_CFG_AUTO_DISABLE		(0 << 20)

/* For PMECC Clock Control Register */
#define		PMECC_CLK_133MHZ		(2 << 0)

/* For PMECC Control Register */
#define		PMECC_CTRL_RST			(1 << 0)
#define		PMECC_CTRL_DATA			(1 << 1)
#define		PMECC_CTRL_USER			(1 << 2)
#define		PMECC_CTRL_ENABLE		(1 << 4)
#define		PMECC_CTRL_DISABLE		(1 << 5)

/* For PMECC Status Register */
#define		PMECC_SR_BUSY			(1 << 0)
#define		PMECC_SR_ENABLE			(1 << 4)

/* PMERRLOC Register Definitions */
struct pmecc_errloc_regs {
	u32 elcfg;	/* 0x00 Error Location Configuration Register */
	u32 elprim;	/* 0x04 Error Location Primitive Register */
	u32 elen;	/* 0x08 Error Location Enable Register */
	u32 eldis;	/* 0x0C Error Location Disable Register */
	u32 elsr;	/* 0x10 Error Location Status Register */
	u32 elier;	/* 0x14 Error Location Interrupt Enable Register */
	u32 elidr;	/* 0x08 Error Location Interrupt Disable Register */
	u32 elimr;	/* 0x0C Error Location Interrupt Mask Register */
	u32 elisr;	/* 0x20 Error Location Interrupt Status Register */
	u32 reserved0;	/* 0x24 Reserved */
	u32 sigma[25];	/* 0x28-0x88 Error Location Sigma Registers */
	u32 el[24];	/* 0x8C-0xE8 Error Location Registers */
	u32 reserved1[5];	/* 0xEC-0xFC Reserved */
};

/* For Error Location Configuration Register */
#define		PMERRLOC_ELCFG_SECTOR_512	(0 << 0)
#define		PMERRLOC_ELCFG_SECTOR_1024	(1 << 0)
#define		PMERRLOC_ELCFG_NUM_ERRORS(n)	((n) << 16)

/* For Error Location Disable Register */
#define		PMERRLOC_DISABLE		(1 << 0)

/* For Error Location Interrupt Status Register */
#define		PMERRLOC_ERR_NUM_MASK		(0x1f << 8)
#define		PMERRLOC_CALC_DONE		(1 << 0)

/* Galois field dimension */
#define PMECC_GF_DIMENSION_13			13
#define PMECC_GF_DIMENSION_14			14

#define PMECC_INDEX_TABLE_SIZE_512		0x2000
#define PMECC_INDEX_TABLE_SIZE_1024		0x4000

#define PMECC_MAX_TIMEOUT_US		(100 * 1000)

#endif
