/*
 * i2c driver for Freescale i.MX series
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (c) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on i2c-imx.c from linux kernel:
 *  Copyright (C) 2005 Torsten Koschorrek <koschorrek at synertronixx.de>
 *  Copyright (C) 2005 Matthias Blaschke <blaschke at synertronixx.de>
 *  Copyright (C) 2007 RightHand Technologies, Inc.
 *  Copyright (C) 2008 Darius Augulis <darius.augulis at teltonika.lt>
 *
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

#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_HARD_I2C)

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <i2c.h>

struct mxc_i2c_regs {
	uint32_t	iadr;
	uint32_t	ifdr;
	uint32_t	i2cr;
	uint32_t	i2sr;
	uint32_t	i2dr;
};

#define I2CR_IEN	(1 << 7)
#define I2CR_IIEN	(1 << 6)
#define I2CR_MSTA	(1 << 5)
#define I2CR_MTX	(1 << 4)
#define I2CR_TX_NO_AK	(1 << 3)
#define I2CR_RSTA	(1 << 2)

#define I2SR_ICF	(1 << 7)
#define I2SR_IBB	(1 << 5)
#define I2SR_IIF	(1 << 1)
#define I2SR_RX_NO_AK	(1 << 0)

#ifdef CONFIG_SYS_I2C_BASE
#define I2C_BASE	CONFIG_SYS_I2C_BASE
#else
#error "define CONFIG_SYS_I2C_BASE to use the mxc_i2c driver"
#endif

#define I2C_MAX_TIMEOUT		10000

static u16 i2c_clk_div[50][2] = {
	{ 22,	0x20 }, { 24,	0x21 }, { 26,	0x22 }, { 28,	0x23 },
	{ 30,	0x00 }, { 32,	0x24 }, { 36,	0x25 }, { 40,	0x26 },
	{ 42,	0x03 }, { 44,	0x27 }, { 48,	0x28 }, { 52,	0x05 },
	{ 56,	0x29 }, { 60,	0x06 }, { 64,	0x2A }, { 72,	0x2B },
	{ 80,	0x2C }, { 88,	0x09 }, { 96,	0x2D }, { 104,	0x0A },
	{ 112,	0x2E }, { 128,	0x2F }, { 144,	0x0C }, { 160,	0x30 },
	{ 192,	0x31 }, { 224,	0x32 }, { 240,	0x0F }, { 256,	0x33 },
	{ 288,	0x10 }, { 320,	0x34 }, { 384,	0x35 }, { 448,	0x36 },
	{ 480,	0x13 }, { 512,	0x37 }, { 576,	0x14 }, { 640,	0x38 },
	{ 768,	0x39 }, { 896,	0x3A }, { 960,	0x17 }, { 1024,	0x3B },
	{ 1152,	0x18 }, { 1280,	0x3C }, { 1536,	0x3D }, { 1792,	0x3E },
	{ 1920,	0x1B }, { 2048,	0x3F }, { 2304,	0x1C }, { 2560,	0x1D },
	{ 3072,	0x1E }, { 3840,	0x1F }
};

/*
 * Calculate and set proper clock divider
 */
static uint8_t i2c_imx_get_clk(unsigned int rate)
{
	unsigned int i2c_clk_rate;
	unsigned int div;
	u8 clk_div;

#if defined(CONFIG_MX31)
	struct clock_control_regs *sc_regs =
		(struct clock_control_regs *)CCM_BASE;

	/* start the required I2C clock */
	writel(readl(&sc_regs->cgr0) | (3 << CONFIG_SYS_I2C_CLK_OFFSET),
		&sc_regs->cgr0);
#endif

	/* Divider value calculation */
	i2c_clk_rate = mxc_get_clock(MXC_IPG_PERCLK);
	div = (i2c_clk_rate + rate - 1) / rate;
	if (div < i2c_clk_div[0][0])
		clk_div = 0;
	else if (div > i2c_clk_div[ARRAY_SIZE(i2c_clk_div) - 1][0])
		clk_div = ARRAY_SIZE(i2c_clk_div) - 1;
	else
		for (clk_div = 0; i2c_clk_div[clk_div][0] < div; clk_div++)
			;

	/* Store divider value */
	return clk_div;
}

/*
 * Reset I2C Controller
 */
void i2c_reset(void)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;

	writeb(0, &i2c_regs->i2cr);	/* Reset module */
	writeb(0, &i2c_regs->i2sr);
}

/*
 * Init I2C Bus
 */
void i2c_init(int speed, int unused)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	u8 clk_idx = i2c_imx_get_clk(speed);
	u8 idx = i2c_clk_div[clk_idx][1];

	/* Store divider value */
	writeb(idx, &i2c_regs->ifdr);

	i2c_reset();
}

/*
 * Set I2C Speed
 */
int i2c_set_bus_speed(unsigned int speed)
{
	i2c_init(speed, 0);
	return 0;
}

/*
 * Get I2C Speed
 */
unsigned int i2c_get_bus_speed(void)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	u8 clk_idx = readb(&i2c_regs->ifdr);
	u8 clk_div;

	for (clk_div = 0; i2c_clk_div[clk_div][1] != clk_idx; clk_div++)
		;

	return mxc_get_clock(MXC_IPG_PERCLK) / i2c_clk_div[clk_div][0];
}

/*
 * Wait for bus to be busy (or free if for_busy = 0)
 *
 * for_busy = 1: Wait for IBB to be asserted
 * for_busy = 0: Wait for IBB to be de-asserted
 */
int i2c_imx_bus_busy(int for_busy)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	unsigned int temp;

	int timeout = I2C_MAX_TIMEOUT;

	while (timeout--) {
		temp = readb(&i2c_regs->i2sr);

		if (for_busy && (temp & I2SR_IBB))
			return 0;
		if (!for_busy && !(temp & I2SR_IBB))
			return 0;

		udelay(1);
	}

	return 1;
}

/*
 * Wait for transaction to complete
 */
int i2c_imx_trx_complete(void)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	int timeout = I2C_MAX_TIMEOUT;

	while (timeout--) {
		if (readb(&i2c_regs->i2sr) & I2SR_IIF) {
			writeb(0, &i2c_regs->i2sr);
			return 0;
		}

		udelay(1);
	}

	return 1;
}

/*
 * Check if the transaction was ACKed
 */
int i2c_imx_acked(void)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;

	return readb(&i2c_regs->i2sr) & I2SR_RX_NO_AK;
}

/*
 * Start the controller
 */
int i2c_imx_start(void)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	unsigned int temp = 0;
	int result;

	/* Enable I2C controller */
	writeb(0, &i2c_regs->i2sr);
	writeb(I2CR_IEN, &i2c_regs->i2cr);

	/* Wait controller to be stable */
	udelay(50);

	/* Start I2C transaction */
	temp = readb(&i2c_regs->i2cr);
	temp |= I2CR_MSTA;
	writeb(temp, &i2c_regs->i2cr);

	result = i2c_imx_bus_busy(1);
	if (result)
		return result;

	temp |= I2CR_MTX | I2CR_TX_NO_AK;
	writeb(temp, &i2c_regs->i2cr);

	return 0;
}

/*
 * Stop the controller
 */
void i2c_imx_stop(void)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	unsigned int temp = 0;

	/* Stop I2C transaction */
	temp = readb(&i2c_regs->i2cr);
	temp |= ~(I2CR_MSTA | I2CR_MTX);
	writeb(temp, &i2c_regs->i2cr);

	i2c_imx_bus_busy(0);

	/* Disable I2C controller */
	writeb(0, &i2c_regs->i2cr);
}

/*
 * Set chip address and access mode
 *
 * read = 1: READ access
 * read = 0: WRITE access
 */
int i2c_imx_set_chip_addr(uchar chip, int read)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	int ret;

	writeb((chip << 1) | read, &i2c_regs->i2dr);

	ret = i2c_imx_trx_complete();
	if (ret)
		return ret;

	ret = i2c_imx_acked();
	if (ret)
		return ret;

	return ret;
}

/*
 * Write register address
 */
int i2c_imx_set_reg_addr(uint addr, int alen)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	int ret = 0;

	while (alen--) {
		writeb((addr >> (alen * 8)) & 0xff, &i2c_regs->i2dr);

		ret = i2c_imx_trx_complete();
		if (ret)
			break;

		ret = i2c_imx_acked();
		if (ret)
			break;
	}

	return ret;
}

/*
 * Try if a chip add given address responds (probe the chip)
 */
int i2c_probe(uchar chip)
{
	int ret;

	ret = i2c_imx_start();
	if (ret)
		return ret;

	ret = i2c_imx_set_chip_addr(chip, 0);
	if (ret)
		return ret;

	i2c_imx_stop();

	return ret;
}

/*
 * Read data from I2C device
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	int ret;
	unsigned int temp;
	int i;

	ret = i2c_imx_start();
	if (ret)
		return ret;

	/* write slave address */
	ret = i2c_imx_set_chip_addr(chip, 0);
	if (ret)
		return ret;

	ret = i2c_imx_set_reg_addr(addr, alen);
	if (ret)
		return ret;

	temp = readb(&i2c_regs->i2cr);
	temp |= I2CR_RSTA;
	writeb(temp, &i2c_regs->i2cr);

	ret = i2c_imx_set_chip_addr(chip, 1);
	if (ret)
		return ret;

	/* setup bus to read data */
	temp = readb(&i2c_regs->i2cr);
	temp &= ~(I2CR_MTX | I2CR_TX_NO_AK);
	if (len == 1)
		temp |= I2CR_TX_NO_AK;
	writeb(temp, &i2c_regs->i2cr);
	readb(&i2c_regs->i2dr);

	/* read data */
	for (i = 0; i < len; i++) {
		ret = i2c_imx_trx_complete();
		if (ret)
			return ret;

		/*
		 * It must generate STOP before read I2DR to prevent
		 * controller from generating another clock cycle
		 */
		if (i == (len - 1)) {
			temp = readb(&i2c_regs->i2cr);
			temp &= ~(I2CR_MSTA | I2CR_MTX);
			writeb(temp, &i2c_regs->i2cr);
			i2c_imx_bus_busy(0);
		} else if (i == (len - 2)) {
			temp = readb(&i2c_regs->i2cr);
			temp |= I2CR_TX_NO_AK;
			writeb(temp, &i2c_regs->i2cr);
		}

		buf[i] = readb(&i2c_regs->i2dr);
	}

	i2c_imx_stop();

	return ret;
}

/*
 * Write data to I2C device
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)I2C_BASE;
	int ret;
	int i;

	ret = i2c_imx_start();
	if (ret)
		return ret;

	/* write slave address */
	ret = i2c_imx_set_chip_addr(chip, 0);
	if (ret)
		return ret;

	ret = i2c_imx_set_reg_addr(addr, alen);
	if (ret)
		return ret;

	for (i = 0; i < len; i++) {
		writeb(buf[i], &i2c_regs->i2dr);

		ret = i2c_imx_trx_complete();
		if (ret)
			return ret;

		ret = i2c_imx_acked();
		if (ret)
			return ret;
	}

	i2c_imx_stop();

	return ret;
}
#endif /* CONFIG_HARD_I2C */
