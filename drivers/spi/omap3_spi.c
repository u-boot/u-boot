/*
 * Copyright (C) 2010 Dirk Behme <dirk.behme@googlemail.com>
 *
 * Driver for McSPI controller on OMAP3. Based on davinci_spi.c
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Copyright (C) 2007 Atmel Corporation
 *
 * Parts taken from linux/drivers/spi/omap2_mcspi.c
 * Copyright (C) 2005, 2006 Nokia Corporation
 *
 * Modified by Ruslan Araslanov <ruslan.araslanov@vitecmm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
#include "omap3_spi.h"

#define SPI_WAIT_TIMEOUT 10

static void spi_reset(struct omap3_spi_slave *ds)
{
	unsigned int tmp;

	writel(OMAP3_MCSPI_SYSCONFIG_SOFTRESET, &ds->regs->sysconfig);
	do {
		tmp = readl(&ds->regs->sysstatus);
	} while (!(tmp & OMAP3_MCSPI_SYSSTATUS_RESETDONE));

	writel(OMAP3_MCSPI_SYSCONFIG_AUTOIDLE |
				 OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP |
				 OMAP3_MCSPI_SYSCONFIG_SMARTIDLE,
				 &ds->regs->sysconfig);

	writel(OMAP3_MCSPI_WAKEUPENABLE_WKEN, &ds->regs->wakeupenable);
}

static void omap3_spi_write_chconf(struct omap3_spi_slave *ds, int val)
{
	writel(val, &ds->regs->channel[ds->slave.cs].chconf);
	/* Flash post writes to make immediate effect */
	readl(&ds->regs->channel[ds->slave.cs].chconf);
}

static void omap3_spi_set_enable(struct omap3_spi_slave *ds, int enable)
{
	writel(enable, &ds->regs->channel[ds->slave.cs].chctrl);
	/* Flash post writes to make immediate effect */
	readl(&ds->regs->channel[ds->slave.cs].chctrl);
}

void spi_init()
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct omap3_spi_slave	*ds;
	struct mcspi *regs;

	/*
	 * OMAP3 McSPI (MultiChannel SPI) has 4 busses (modules)
	 * with different number of chip selects (CS, channels):
	 * McSPI1 has 4 CS (bus 0, cs 0 - 3)
	 * McSPI2 has 2 CS (bus 1, cs 0 - 1)
	 * McSPI3 has 2 CS (bus 2, cs 0 - 1)
	 * McSPI4 has 1 CS (bus 3, cs 0)
	 */

	switch (bus) {
	case 0:
		regs = (struct mcspi *)OMAP3_MCSPI1_BASE;
		break;
#ifdef OMAP3_MCSPI2_BASE
	case 1:
		regs = (struct mcspi *)OMAP3_MCSPI2_BASE;
		break;
#endif
#ifdef OMAP3_MCSPI3_BASE
	case 2:
		regs = (struct mcspi *)OMAP3_MCSPI3_BASE;
		break;
#endif
#ifdef OMAP3_MCSPI4_BASE
	case 3:
		regs = (struct mcspi *)OMAP3_MCSPI4_BASE;
		break;
#endif
	default:
		printf("SPI error: unsupported bus %i. \
			Supported busses 0 - 3\n", bus);
		return NULL;
	}

	if (((bus == 0) && (cs > 3)) ||
			((bus == 1) && (cs > 1)) ||
			((bus == 2) && (cs > 1)) ||
			((bus == 3) && (cs > 0))) {
		printf("SPI error: unsupported chip select %i \
			on bus %i\n", cs, bus);
		return NULL;
	}

	if (max_hz > OMAP3_MCSPI_MAX_FREQ) {
		printf("SPI error: unsupported frequency %i Hz. \
			Max frequency is 48 Mhz\n", max_hz);
		return NULL;
	}

	if (mode > SPI_MODE_3) {
		printf("SPI error: unsupported SPI mode %i\n", mode);
		return NULL;
	}

	ds = spi_alloc_slave(struct omap3_spi_slave, bus, cs);
	if (!ds) {
		printf("SPI error: malloc of SPI structure failed\n");
		return NULL;
	}

	ds->regs = regs;
	ds->freq = max_hz;
	ds->mode = mode;

	return &ds->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);

	free(ds);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);
	unsigned int conf, div = 0;

	/* McSPI global module configuration */

	/*
	 * setup when switching from (reset default) slave mode
	 * to single-channel master mode
	 */
	spi_reset(ds);
	conf = readl(&ds->regs->modulctrl);
	conf &= ~(OMAP3_MCSPI_MODULCTRL_STEST | OMAP3_MCSPI_MODULCTRL_MS);
	conf |= OMAP3_MCSPI_MODULCTRL_SINGLE;
	writel(conf, &ds->regs->modulctrl);

	/* McSPI individual channel configuration */

	/* Calculate clock divisor. Valid range: 0x0 - 0xC ( /1 - /4096 ) */
	if (ds->freq) {
		while (div <= 0xC && (OMAP3_MCSPI_MAX_FREQ / (1 << div))
					 > ds->freq)
			div++;
	} else
		div = 0xC;

	conf = readl(&ds->regs->channel[ds->slave.cs].chconf);

	/* standard 4-wire master mode:	SCK, MOSI/out, MISO/in, nCS
	 * REVISIT: this controller could support SPI_3WIRE mode.
	 */
#ifdef CONFIG_OMAP3_SPI_D0_D1_SWAPPED
	/*
	 * Some boards have D0 wired as MOSI / D1 as MISO instead of
	 * The normal D0 as MISO / D1 as MOSI.
	 */
	conf &= ~OMAP3_MCSPI_CHCONF_DPE0;
	conf |= OMAP3_MCSPI_CHCONF_IS|OMAP3_MCSPI_CHCONF_DPE1;
#else
	conf &= ~(OMAP3_MCSPI_CHCONF_IS|OMAP3_MCSPI_CHCONF_DPE1);
	conf |= OMAP3_MCSPI_CHCONF_DPE0;
#endif

	/* wordlength */
	conf &= ~OMAP3_MCSPI_CHCONF_WL_MASK;
	conf |= (ds->slave.wordlen - 1) << 7;

	/* set chipselect polarity; manage with FORCE */
	if (!(ds->mode & SPI_CS_HIGH))
		conf |= OMAP3_MCSPI_CHCONF_EPOL; /* active-low; normal */
	else
		conf &= ~OMAP3_MCSPI_CHCONF_EPOL;

	/* set clock divisor */
	conf &= ~OMAP3_MCSPI_CHCONF_CLKD_MASK;
	conf |= div << 2;

	/* set SPI mode 0..3 */
	if (ds->mode & SPI_CPOL)
		conf |= OMAP3_MCSPI_CHCONF_POL;
	else
		conf &= ~OMAP3_MCSPI_CHCONF_POL;
	if (ds->mode & SPI_CPHA)
		conf |= OMAP3_MCSPI_CHCONF_PHA;
	else
		conf &= ~OMAP3_MCSPI_CHCONF_PHA;

	/* Transmit & receive mode */
	conf &= ~OMAP3_MCSPI_CHCONF_TRM_MASK;

	omap3_spi_write_chconf(ds,conf);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);

	/* Reset the SPI hardware */
	spi_reset(ds);
}

int omap3_spi_write(struct spi_slave *slave, unsigned int len, const void *txp,
		    unsigned long flags)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);
	int i;
	ulong start;
	int chconf = readl(&ds->regs->channel[ds->slave.cs].chconf);

	/* Enable the channel */
	omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_EN);

	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (ds->slave.wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_TRM_TX_ONLY;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(ds,chconf);

	for (i = 0; i < len; i++) {
		/* wait till TX register is empty (TXS == 1) */
		start = get_timer(0);
		while (!(readl(&ds->regs->channel[ds->slave.cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_TXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI TXS timed out, status=0x%08x\n",
				       readl(&ds->regs->channel[ds->slave.cs].chstat));
				return -1;
			}
		}
		/* Write the data */
		unsigned int *tx = &ds->regs->channel[ds->slave.cs].tx;
		if (ds->slave.wordlen > 16)
			writel(((u32 *)txp)[i], tx);
		else if (ds->slave.wordlen > 8)
			writel(((u16 *)txp)[i], tx);
		else
			writel(((u8 *)txp)[i], tx);
	}

	/* wait to finish of transfer */
	while ((readl(&ds->regs->channel[ds->slave.cs].chstat) &
			 (OMAP3_MCSPI_CHSTAT_EOT | OMAP3_MCSPI_CHSTAT_TXS)) !=
			 (OMAP3_MCSPI_CHSTAT_EOT | OMAP3_MCSPI_CHSTAT_TXS));

	/* Disable the channel otherwise the next immediate RX will get affected */
	omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_DIS);

	if (flags & SPI_XFER_END) {

		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(ds,chconf);
	}
	return 0;
}

int omap3_spi_read(struct spi_slave *slave, unsigned int len, void *rxp,
		   unsigned long flags)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);
	int i;
	ulong start;
	int chconf = readl(&ds->regs->channel[ds->slave.cs].chconf);

	/* Enable the channel */
	omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_EN);

	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (ds->slave.wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_TRM_RX_ONLY;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(ds,chconf);

	writel(0, &ds->regs->channel[ds->slave.cs].tx);

	for (i = 0; i < len; i++) {
		start = get_timer(0);
		/* Wait till RX register contains data (RXS == 1) */
		while (!(readl(&ds->regs->channel[ds->slave.cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_RXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI RXS timed out, status=0x%08x\n",
				       readl(&ds->regs->channel[ds->slave.cs].chstat));
				return -1;
			}
		}

		/* Disable the channel to prevent furher receiving */
		if(i == (len - 1))
			omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_DIS);

		/* Read the data */
		unsigned int *rx = &ds->regs->channel[ds->slave.cs].rx;
		if (ds->slave.wordlen > 16)
			((u32 *)rxp)[i] = readl(rx);
		else if (ds->slave.wordlen > 8)
			((u16 *)rxp)[i] = (u16)readl(rx);
		else
			((u8 *)rxp)[i] = (u8)readl(rx);
	}

	if (flags & SPI_XFER_END) {
		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(ds,chconf);
	}

	return 0;
}

/*McSPI Transmit Receive Mode*/
int omap3_spi_txrx(struct spi_slave *slave, unsigned int len,
		   const void *txp, void *rxp, unsigned long flags)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);
	ulong start;
	int chconf = readl(&ds->regs->channel[ds->slave.cs].chconf);
	int i=0;

	/*Enable SPI channel*/
	omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_EN);

	/*set TRANSMIT-RECEIVE Mode*/
	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (ds->slave.wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(ds,chconf);

	/*Shift in and out 1 byte at time*/
	for (i=0; i < len; i++){
		/* Write: wait for TX empty (TXS == 1)*/
		start = get_timer(0);
		while (!(readl(&ds->regs->channel[ds->slave.cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_TXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI TXS timed out, status=0x%08x\n",
				       readl(&ds->regs->channel[ds->slave.cs].chstat));
				return -1;
			}
		}
		/* Write the data */
		unsigned int *tx = &ds->regs->channel[ds->slave.cs].tx;
		if (ds->slave.wordlen > 16)
			writel(((u32 *)txp)[i], tx);
		else if (ds->slave.wordlen > 8)
			writel(((u16 *)txp)[i], tx);
		else
			writel(((u8 *)txp)[i], tx);

		/*Read: wait for RX containing data (RXS == 1)*/
		start = get_timer(0);
		while (!(readl(&ds->regs->channel[ds->slave.cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_RXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI RXS timed out, status=0x%08x\n",
				       readl(&ds->regs->channel[ds->slave.cs].chstat));
				return -1;
			}
		}
		/* Read the data */
		unsigned int *rx = &ds->regs->channel[ds->slave.cs].rx;
		if (ds->slave.wordlen > 16)
			((u32 *)rxp)[i] = readl(rx);
		else if (ds->slave.wordlen > 8)
			((u16 *)rxp)[i] = (u16)readl(rx);
		else
			((u8 *)rxp)[i] = (u8)readl(rx);
	}
	/* Disable the channel */
	omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_DIS);

	/*if transfer must be terminated disable the channel*/
	if (flags & SPI_XFER_END) {
		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(ds,chconf);
	}

	return 0;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	struct omap3_spi_slave *ds = to_omap3_spi(slave);
	unsigned int	len;
	int ret = -1;

	if (ds->slave.wordlen < 4 || ds->slave.wordlen > 32) {
		printf("omap3_spi: invalid wordlen %d\n", ds->slave.wordlen);
		return -1;
	}

	if (bitlen % ds->slave.wordlen)
		return -1;

	len = bitlen / ds->slave.wordlen;

	if (bitlen == 0) {	 /* only change CS */
		int chconf = readl(&ds->regs->channel[ds->slave.cs].chconf);

		if (flags & SPI_XFER_BEGIN) {
			omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_EN);
			chconf |= OMAP3_MCSPI_CHCONF_FORCE;
			omap3_spi_write_chconf(ds,chconf);
		}
		if (flags & SPI_XFER_END) {
			chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
			omap3_spi_write_chconf(ds,chconf);
			omap3_spi_set_enable(ds,OMAP3_MCSPI_CHCTRL_DIS);
		}
		ret = 0;
	} else {
		if (dout != NULL && din != NULL)
			ret = omap3_spi_txrx(slave, len, dout, din, flags);
		else if (dout != NULL)
			ret = omap3_spi_write(slave, len, dout, flags);
		else if (din != NULL)
			ret = omap3_spi_read(slave, len, din, flags);
	}
	return ret;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
}

void spi_cs_deactivate(struct spi_slave *slave)
{
}
