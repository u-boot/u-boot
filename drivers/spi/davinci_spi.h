/*
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Register definitions for the DaVinci SPI Controller
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DAVINCI_SPI_H_
#define _DAVINCI_SPI_H_

struct davinci_spi_regs {
	dv_reg	gcr0;		/* 0x00 */
	dv_reg	gcr1;		/* 0x04 */
	dv_reg	int0;		/* 0x08 */
	dv_reg	lvl;		/* 0x0c */
	dv_reg	flg;		/* 0x10 */
	dv_reg	pc0;		/* 0x14 */
	dv_reg	pc1;		/* 0x18 */
	dv_reg	pc2;		/* 0x1c */
	dv_reg	pc3;		/* 0x20 */
	dv_reg	pc4;		/* 0x24 */
	dv_reg	pc5;		/* 0x28 */
	dv_reg	rsvd[3];
	dv_reg	dat0;		/* 0x38 */
	dv_reg	dat1;		/* 0x3c */
	dv_reg	buf;		/* 0x40 */
	dv_reg	emu;		/* 0x44 */
	dv_reg	delay;		/* 0x48 */
	dv_reg	def;		/* 0x4c */
	dv_reg	fmt0;		/* 0x50 */
	dv_reg	fmt1;		/* 0x54 */
	dv_reg	fmt2;		/* 0x58 */
	dv_reg	fmt3;		/* 0x5c */
	dv_reg	intvec0;	/* 0x60 */
	dv_reg	intvec1;	/* 0x64 */
};

#define BIT(x)			(1 << (x))

/* SPIGCR0 */
#define SPIGCR0_SPIENA_MASK	0x1
#define SPIGCR0_SPIRST_MASK	0x0

/* SPIGCR0 */
#define SPIGCR1_CLKMOD_MASK	BIT(1)
#define SPIGCR1_MASTER_MASK	BIT(0)
#define SPIGCR1_SPIENA_MASK	BIT(24)

/* SPIPC0 */
#define SPIPC0_DIFUN_MASK	BIT(11)		/* SIMO */
#define SPIPC0_DOFUN_MASK	BIT(10)		/* SOMI */
#define SPIPC0_CLKFUN_MASK	BIT(9)		/* CLK */
#define SPIPC0_EN0FUN_MASK	BIT(0)

/* SPIFMT0 */
#define SPIFMT_SHIFTDIR_SHIFT	20
#define SPIFMT_POLARITY_SHIFT	17
#define SPIFMT_PHASE_SHIFT	16
#define SPIFMT_PRESCALE_SHIFT	8

/* SPIDAT1 */
#define SPIDAT1_CSHOLD_SHIFT	28
#define SPIDAT1_CSNR_SHIFT	16

/* SPIDELAY */
#define SPI_C2TDELAY_SHIFT	24
#define SPI_T2CDELAY_SHIFT	16

/* SPIBUF */
#define SPIBUF_RXEMPTY_MASK	BIT(31)
#define SPIBUF_TXFULL_MASK	BIT(29)

/* SPIDEF */
#define SPIDEF_CSDEF0_MASK	BIT(0)

struct davinci_spi_slave {
	struct spi_slave slave;
	struct davinci_spi_regs *regs;
	unsigned int freq;
};

static inline struct davinci_spi_slave *to_davinci_spi(struct spi_slave *slave)
{
	return container_of(slave, struct davinci_spi_slave, slave);
}

#endif /* _DAVINCI_SPI_H_ */
