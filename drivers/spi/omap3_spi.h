/*
 * Register definitions for the OMAP3 McSPI Controller
 *
 * Copyright (C) 2010 Dirk Behme <dirk.behme@googlemail.com>
 *
 * Parts taken from linux/drivers/spi/omap2_mcspi.c
 * Copyright (C) 2005, 2006 Nokia Corporation
 *
 * Modified by Ruslan Araslanov <ruslan.araslanov@vitecmm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _OMAP3_SPI_H_
#define _OMAP3_SPI_H_

#if defined(CONFIG_AM33XX) || defined(CONFIG_AM43XX)
#define OMAP3_MCSPI1_BASE	0x48030100
#define OMAP3_MCSPI2_BASE	0x481A0100
#else
#define OMAP3_MCSPI1_BASE	0x48098000
#define OMAP3_MCSPI2_BASE	0x4809A000
#define OMAP3_MCSPI3_BASE	0x480B8000
#define OMAP3_MCSPI4_BASE	0x480BA000
#endif

#define OMAP3_MCSPI_MAX_FREQ	48000000

/* OMAP3 McSPI registers */
struct mcspi_channel {
	unsigned int chconf;		/* 0x2C, 0x40, 0x54, 0x68 */
	unsigned int chstat;		/* 0x30, 0x44, 0x58, 0x6C */
	unsigned int chctrl;		/* 0x34, 0x48, 0x5C, 0x70 */
	unsigned int tx;		/* 0x38, 0x4C, 0x60, 0x74 */
	unsigned int rx;		/* 0x3C, 0x50, 0x64, 0x78 */
};

struct mcspi {
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned int sysstatus;		/* 0x14 */
	unsigned int irqstatus;		/* 0x18 */
	unsigned int irqenable;		/* 0x1C */
	unsigned int wakeupenable;	/* 0x20 */
	unsigned int syst;		/* 0x24 */
	unsigned int modulctrl;		/* 0x28 */
	struct mcspi_channel channel[4]; /* channel0: 0x2C - 0x3C, bus 0 & 1 & 2 & 3 */
					/* channel1: 0x40 - 0x50, bus 0 & 1 */
					/* channel2: 0x54 - 0x64, bus 0 & 1 */
					/* channel3: 0x68 - 0x78, bus 0 */
};

/* per-register bitmasks */
#define OMAP3_MCSPI_SYSCONFIG_SMARTIDLE (2 << 3)
#define OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP BIT(2)
#define OMAP3_MCSPI_SYSCONFIG_AUTOIDLE	BIT(0)
#define OMAP3_MCSPI_SYSCONFIG_SOFTRESET BIT(1)

#define OMAP3_MCSPI_SYSSTATUS_RESETDONE BIT(0)

#define OMAP3_MCSPI_MODULCTRL_SINGLE	BIT(0)
#define OMAP3_MCSPI_MODULCTRL_MS	BIT(2)
#define OMAP3_MCSPI_MODULCTRL_STEST	BIT(3)

#define OMAP3_MCSPI_CHCONF_PHA		BIT(0)
#define OMAP3_MCSPI_CHCONF_POL		BIT(1)
#define OMAP3_MCSPI_CHCONF_CLKD_MASK	GENMASK(5, 2)
#define OMAP3_MCSPI_CHCONF_EPOL		BIT(6)
#define OMAP3_MCSPI_CHCONF_WL_MASK	GENMASK(11, 7)
#define OMAP3_MCSPI_CHCONF_TRM_RX_ONLY	BIT(12)
#define OMAP3_MCSPI_CHCONF_TRM_TX_ONLY	BIT(13)
#define OMAP3_MCSPI_CHCONF_TRM_MASK	GENMASK(13, 12)
#define OMAP3_MCSPI_CHCONF_DMAW		BIT(14)
#define OMAP3_MCSPI_CHCONF_DMAR		BIT(15)
#define OMAP3_MCSPI_CHCONF_DPE0		BIT(16)
#define OMAP3_MCSPI_CHCONF_DPE1		BIT(17)
#define OMAP3_MCSPI_CHCONF_IS		BIT(18)
#define OMAP3_MCSPI_CHCONF_TURBO	BIT(19)
#define OMAP3_MCSPI_CHCONF_FORCE	BIT(20)

#define OMAP3_MCSPI_CHSTAT_RXS		BIT(0)
#define OMAP3_MCSPI_CHSTAT_TXS		BIT(1)
#define OMAP3_MCSPI_CHSTAT_EOT		BIT(2)

#define OMAP3_MCSPI_CHCTRL_EN		BIT(0)
#define OMAP3_MCSPI_CHCTRL_DIS		(0 << 0)

#define OMAP3_MCSPI_WAKEUPENABLE_WKEN	BIT(0)

struct omap3_spi_slave {
	struct spi_slave slave;
	struct mcspi *regs;
	unsigned int freq;
	unsigned int mode;
};

static inline struct omap3_spi_slave *to_omap3_spi(struct spi_slave *slave)
{
	return container_of(slave, struct omap3_spi_slave, slave);
}

int omap3_spi_txrx(struct spi_slave *slave, unsigned int len, const void *txp,
			void *rxp, unsigned long flags);
int omap3_spi_write(struct spi_slave *slave, unsigned int len, const void *txp,
		    unsigned long flags);
int omap3_spi_read(struct spi_slave *slave, unsigned int len, void *rxp,
		   unsigned long flags);

#endif /* _OMAP3_SPI_H_ */
