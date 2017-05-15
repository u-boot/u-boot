/*
 * Faraday I2C Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * NOTE: This driver should be converted to driver model before June 2017.
 * Please see doc/driver-model/i2c-howto.txt for instructions.
 */

#include <common.h>
#include <asm/io.h>
#include <i2c.h>

#include "fti2c010.h"

#ifndef CONFIG_SYS_I2C_SPEED
#define CONFIG_SYS_I2C_SPEED    5000
#endif

#ifndef CONFIG_SYS_I2C_SLAVE
#define CONFIG_SYS_I2C_SLAVE    0
#endif

#ifndef CONFIG_FTI2C010_CLOCK
#define CONFIG_FTI2C010_CLOCK   clk_get_rate("I2C")
#endif

#ifndef CONFIG_FTI2C010_TIMEOUT
#define CONFIG_FTI2C010_TIMEOUT 10 /* ms */
#endif

/* 7-bit dev address + 1-bit read/write */
#define I2C_RD(dev)             ((((dev) << 1) & 0xfe) | 1)
#define I2C_WR(dev)             (((dev) << 1) & 0xfe)

struct fti2c010_chip {
	struct fti2c010_regs *regs;
};

static struct fti2c010_chip chip_list[] = {
	{
		.regs = (struct fti2c010_regs *)CONFIG_FTI2C010_BASE,
	},
#ifdef CONFIG_FTI2C010_BASE1
	{
		.regs = (struct fti2c010_regs *)CONFIG_FTI2C010_BASE1,
	},
#endif
#ifdef CONFIG_FTI2C010_BASE2
	{
		.regs = (struct fti2c010_regs *)CONFIG_FTI2C010_BASE2,
	},
#endif
#ifdef CONFIG_FTI2C010_BASE3
	{
		.regs = (struct fti2c010_regs *)CONFIG_FTI2C010_BASE3,
	},
#endif
};

static int fti2c010_reset(struct fti2c010_chip *chip)
{
	ulong ts;
	int ret = -1;
	struct fti2c010_regs *regs = chip->regs;

	writel(CR_I2CRST, &regs->cr);
	for (ts = get_timer(0); get_timer(ts) < CONFIG_FTI2C010_TIMEOUT; ) {
		if (!(readl(&regs->cr) & CR_I2CRST)) {
			ret = 0;
			break;
		}
	}

	if (ret)
		printf("fti2c010: reset timeout\n");

	return ret;
}

static int fti2c010_wait(struct fti2c010_chip *chip, uint32_t mask)
{
	int ret = -1;
	uint32_t stat, ts;
	struct fti2c010_regs *regs = chip->regs;

	for (ts = get_timer(0); get_timer(ts) < CONFIG_FTI2C010_TIMEOUT; ) {
		stat = readl(&regs->sr);
		if ((stat & mask) == mask) {
			ret = 0;
			break;
		}
	}

	return ret;
}

static unsigned int set_i2c_bus_speed(struct fti2c010_chip *chip,
	unsigned int speed)
{
	struct fti2c010_regs *regs = chip->regs;
	unsigned int clk = CONFIG_FTI2C010_CLOCK;
	unsigned int gsr = 0;
	unsigned int tsr = 32;
	unsigned int div, rate;

	for (div = 0; div < 0x3ffff; ++div) {
		/* SCLout = PCLK/(2*(COUNT + 2) + GSR) */
		rate = clk / (2 * (div + 2) + gsr);
		if (rate <= speed)
			break;
	}

	writel(TGSR_GSR(gsr) | TGSR_TSR(tsr), &regs->tgsr);
	writel(CDR_DIV(div), &regs->cdr);

	return rate;
}

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
static void fti2c010_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
	struct fti2c010_chip *chip = chip_list + adap->hwadapnr;

	if (adap->init_done)
		return;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/* Call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	*/
	i2c_init_board();
#endif

	/* master init */

	fti2c010_reset(chip);

	set_i2c_bus_speed(chip, speed);

	/* slave init, don't care */
}

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
static int fti2c010_probe(struct i2c_adapter *adap, u8 dev)
{
	struct fti2c010_chip *chip = chip_list + adap->hwadapnr;
	struct fti2c010_regs *regs = chip->regs;
	int ret;

	/* 1. Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_WR(dev), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(chip, SR_DT);
	if (ret)
		return ret;

	/* 2. Select device register */
	writel(0, &regs->dr);
	writel(CR_ENABLE | CR_TBEN, &regs->cr);
	ret = fti2c010_wait(chip, SR_DT);

	return ret;
}

static void to_i2c_addr(u8 *buf, uint32_t addr, int alen)
{
	int i, shift;

	if (!buf || alen <= 0)
		return;

	/* MSB first */
	i = 0;
	shift = (alen - 1) * 8;
	while (alen-- > 0) {
		buf[i] = (u8)(addr >> shift);
		shift -= 8;
	}
}

static int fti2c010_read(struct i2c_adapter *adap,
			u8 dev, uint addr, int alen, uchar *buf, int len)
{
	struct fti2c010_chip *chip = chip_list + adap->hwadapnr;
	struct fti2c010_regs *regs = chip->regs;
	int ret, pos;
	uchar paddr[4] = { 0 };

	to_i2c_addr(paddr, addr, alen);

	/*
	 * Phase A. Set register address
	 */

	/* A.1 Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_WR(dev), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(chip, SR_DT);
	if (ret)
		return ret;

	/* A.2 Select device register */
	for (pos = 0; pos < alen; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		writel(paddr[pos], &regs->dr);
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(chip, SR_DT);
		if (ret)
			return ret;
	}

	/*
	 * Phase B. Get register data
	 */

	/* B.1 Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_RD(dev), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(chip, SR_DT);
	if (ret)
		return ret;

	/* B.2 Get register data */
	for (pos = 0; pos < len; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;
		uint32_t stat = SR_DR;

		if (pos == len - 1) {
			ctrl |= CR_NAK | CR_STOP;
			stat |= SR_ACK;
		}
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(chip, stat);
		if (ret)
			break;
		buf[pos] = (uchar)(readl(&regs->dr) & 0xFF);
	}

	return ret;
}

static int fti2c010_write(struct i2c_adapter *adap,
			u8 dev, uint addr, int alen, u8 *buf, int len)
{
	struct fti2c010_chip *chip = chip_list + adap->hwadapnr;
	struct fti2c010_regs *regs = chip->regs;
	int ret, pos;
	uchar paddr[4] = { 0 };

	to_i2c_addr(paddr, addr, alen);

	/*
	 * Phase A. Set register address
	 *
	 * A.1 Select slave device (7bits Address + 1bit R/W)
	 */
	writel(I2C_WR(dev), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(chip, SR_DT);
	if (ret)
		return ret;

	/* A.2 Select device register */
	for (pos = 0; pos < alen; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		writel(paddr[pos], &regs->dr);
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(chip, SR_DT);
		if (ret)
			return ret;
	}

	/*
	 * Phase B. Set register data
	 */
	for (pos = 0; pos < len; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		if (pos == len - 1)
			ctrl |= CR_STOP;
		writel(buf[pos], &regs->dr);
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(chip, SR_DT);
		if (ret)
			break;
	}

	return ret;
}

static unsigned int fti2c010_set_bus_speed(struct i2c_adapter *adap,
			unsigned int speed)
{
	struct fti2c010_chip *chip = chip_list + adap->hwadapnr;
	int ret;

	fti2c010_reset(chip);
	ret = set_i2c_bus_speed(chip, speed);

	return ret;
}

/*
 * Register i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(i2c_0, fti2c010_init, fti2c010_probe, fti2c010_read,
			fti2c010_write, fti2c010_set_bus_speed,
			CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
			0)
#ifdef CONFIG_FTI2C010_BASE1
U_BOOT_I2C_ADAP_COMPLETE(i2c_1, fti2c010_init, fti2c010_probe, fti2c010_read,
			fti2c010_write, fti2c010_set_bus_speed,
			CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
			1)
#endif
#ifdef CONFIG_FTI2C010_BASE2
U_BOOT_I2C_ADAP_COMPLETE(i2c_2, fti2c010_init, fti2c010_probe, fti2c010_read,
			fti2c010_write, fti2c010_set_bus_speed,
			CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
			2)
#endif
#ifdef CONFIG_FTI2C010_BASE3
U_BOOT_I2C_ADAP_COMPLETE(i2c_3, fti2c010_init, fti2c010_probe, fti2c010_read,
			fti2c010_write, fti2c010_set_bus_speed,
			CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
			3)
#endif
