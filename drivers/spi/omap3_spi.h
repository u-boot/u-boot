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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _OMAP3_SPI_H_
#define _OMAP3_SPI_H_

#define OMAP3_MCSPI1_BASE	0x48098000
#define OMAP3_MCSPI2_BASE	0x4809A000
#define OMAP3_MCSPI3_BASE	0x480B8000
#define OMAP3_MCSPI4_BASE	0x480BA000

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
#define OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP (1 << 2)
#define OMAP3_MCSPI_SYSCONFIG_AUTOIDLE	(1 << 0)
#define OMAP3_MCSPI_SYSCONFIG_SOFTRESET (1 << 1)

#define OMAP3_MCSPI_SYSSTATUS_RESETDONE (1 << 0)

#define OMAP3_MCSPI_MODULCTRL_SINGLE	(1 << 0)
#define OMAP3_MCSPI_MODULCTRL_MS	(1 << 2)
#define OMAP3_MCSPI_MODULCTRL_STEST	(1 << 3)

#define OMAP3_MCSPI_CHCONF_PHA		(1 << 0)
#define OMAP3_MCSPI_CHCONF_POL		(1 << 1)
#define OMAP3_MCSPI_CHCONF_CLKD_MASK	(0x0f << 2)
#define OMAP3_MCSPI_CHCONF_EPOL		(1 << 6)
#define OMAP3_MCSPI_CHCONF_WL_MASK	(0x1f << 7)
#define OMAP3_MCSPI_CHCONF_TRM_RX_ONLY	(0x01 << 12)
#define OMAP3_MCSPI_CHCONF_TRM_TX_ONLY	(0x02 << 12)
#define OMAP3_MCSPI_CHCONF_TRM_MASK	(0x03 << 12)
#define OMAP3_MCSPI_CHCONF_DMAW		(1 << 14)
#define OMAP3_MCSPI_CHCONF_DMAR		(1 << 15)
#define OMAP3_MCSPI_CHCONF_DPE0		(1 << 16)
#define OMAP3_MCSPI_CHCONF_DPE1		(1 << 17)
#define OMAP3_MCSPI_CHCONF_IS		(1 << 18)
#define OMAP3_MCSPI_CHCONF_TURBO	(1 << 19)
#define OMAP3_MCSPI_CHCONF_FORCE	(1 << 20)

#define OMAP3_MCSPI_CHSTAT_RXS		(1 << 0)
#define OMAP3_MCSPI_CHSTAT_TXS		(1 << 1)
#define OMAP3_MCSPI_CHSTAT_EOT		(1 << 2)

#define OMAP3_MCSPI_CHCTRL_EN		(1 << 0)

#define OMAP3_MCSPI_WAKEUPENABLE_WKEN	(1 << 0)

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

int omap3_spi_write(struct spi_slave *slave, unsigned int len, const u8 *txp,
		    unsigned long flags);
int omap3_spi_read(struct spi_slave *slave, unsigned int len, u8 *rxp,
		   unsigned long flags);

#endif /* _OMAP3_SPI_H_ */
