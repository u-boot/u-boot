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
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <i2c.h>
#include <watchdog.h>

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
#define I2SR_IAL	(1 << 4)
#define I2SR_IIF	(1 << 1)
#define I2SR_RX_NO_AK	(1 << 0)

#if defined(CONFIG_HARD_I2C) && !defined(CONFIG_SYS_I2C_BASE)
#error "define CONFIG_SYS_I2C_BASE to use the mxc_i2c driver"
#endif

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
	i2c_clk_rate = mxc_get_clock(MXC_I2C_CLK);
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
 * Set I2C Bus speed
 */
static int bus_i2c_set_bus_speed(void *base, int speed)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)base;
	u8 clk_idx = i2c_imx_get_clk(speed);
	u8 idx = i2c_clk_div[clk_idx][1];

	/* Store divider value */
	writeb(idx, &i2c_regs->ifdr);

	/* Reset module */
	writeb(0, &i2c_regs->i2cr);
	writeb(0, &i2c_regs->i2sr);
	return 0;
}

/*
 * Get I2C Speed
 */
static unsigned int bus_i2c_get_bus_speed(void *base)
{
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)base;
	u8 clk_idx = readb(&i2c_regs->ifdr);
	u8 clk_div;

	for (clk_div = 0; i2c_clk_div[clk_div][1] != clk_idx; clk_div++)
		;

	return mxc_get_clock(MXC_I2C_CLK) / i2c_clk_div[clk_div][0];
}

#define ST_BUS_IDLE (0 | (I2SR_IBB << 8))
#define ST_BUS_BUSY (I2SR_IBB | (I2SR_IBB << 8))
#define ST_IIF (I2SR_IIF | (I2SR_IIF << 8))

static int wait_for_sr_state(struct mxc_i2c_regs *i2c_regs, unsigned state)
{
	unsigned sr;
	ulong elapsed;
	ulong start_time = get_timer(0);
	for (;;) {
		sr = readb(&i2c_regs->i2sr);
		if (sr & I2SR_IAL) {
			writeb(sr & ~I2SR_IAL, &i2c_regs->i2sr);
			printf("%s: Arbitration lost sr=%x cr=%x state=%x\n",
				__func__, sr, readb(&i2c_regs->i2cr), state);
			return -ERESTART;
		}
		if ((sr & (state >> 8)) == (unsigned char)state)
			return sr;
		WATCHDOG_RESET();
		elapsed = get_timer(start_time);
		if (elapsed > (CONFIG_SYS_HZ / 10))	/* .1 seconds */
			break;
	}
	printf("%s: failed sr=%x cr=%x state=%x\n", __func__,
			sr, readb(&i2c_regs->i2cr), state);
	return -ETIMEDOUT;
}

static int tx_byte(struct mxc_i2c_regs *i2c_regs, u8 byte)
{
	int ret;

	writeb(0, &i2c_regs->i2sr);
	writeb(byte, &i2c_regs->i2dr);
	ret = wait_for_sr_state(i2c_regs, ST_IIF);
	if (ret < 0)
		return ret;
	if (ret & I2SR_RX_NO_AK)
		return -ENODEV;
	return 0;
}

/*
 * Stop I2C transaction
 */
static void i2c_imx_stop(struct mxc_i2c_regs *i2c_regs)
{
	int ret;
	unsigned int temp = readb(&i2c_regs->i2cr);

	temp &= ~(I2CR_MSTA | I2CR_MTX);
	writeb(temp, &i2c_regs->i2cr);
	ret = wait_for_sr_state(i2c_regs, ST_BUS_IDLE);
	if (ret < 0)
		printf("%s:trigger stop failed\n", __func__);
}

/*
 * Send start signal, chip address and
 * write register address
 */
static int i2c_init_transfer_(struct mxc_i2c_regs *i2c_regs,
		uchar chip, uint addr, int alen)
{
	unsigned int temp;
	int ret;

	/* Enable I2C controller */
	if (!(readb(&i2c_regs->i2cr) & I2CR_IEN)) {
		writeb(I2CR_IEN, &i2c_regs->i2cr);
		/* Wait for controller to be stable */
		udelay(50);
	}
	if (readb(&i2c_regs->iadr) == (chip << 1))
		writeb((chip << 1) ^ 2, &i2c_regs->iadr);
	writeb(0, &i2c_regs->i2sr);
	ret = wait_for_sr_state(i2c_regs, ST_BUS_IDLE);
	if (ret < 0)
		return ret;

	/* Start I2C transaction */
	temp = readb(&i2c_regs->i2cr);
	temp |= I2CR_MSTA;
	writeb(temp, &i2c_regs->i2cr);

	ret = wait_for_sr_state(i2c_regs, ST_BUS_BUSY);
	if (ret < 0)
		return ret;

	temp |= I2CR_MTX | I2CR_TX_NO_AK;
	writeb(temp, &i2c_regs->i2cr);

	/* write slave address */
	ret = tx_byte(i2c_regs, chip << 1);
	if (ret < 0)
		return ret;

	while (alen--) {
		ret = tx_byte(i2c_regs, (addr >> (alen * 8)) & 0xff);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int i2c_idle_bus(void *base);

static int i2c_init_transfer(struct mxc_i2c_regs *i2c_regs,
		uchar chip, uint addr, int alen)
{
	int retry;
	int ret;
	for (retry = 0; retry < 3; retry++) {
		ret = i2c_init_transfer_(i2c_regs, chip, addr, alen);
		if (ret >= 0)
			return 0;
		i2c_imx_stop(i2c_regs);
		if (ret == -ENODEV)
			return ret;

		printf("%s: failed for chip 0x%x retry=%d\n", __func__, chip,
				retry);
		if (ret != -ERESTART)
			writeb(0, &i2c_regs->i2cr);	/* Disable controller */
		udelay(100);
		if (i2c_idle_bus(i2c_regs) < 0)
			break;
	}
	printf("%s: give up i2c_regs=%p\n", __func__, i2c_regs);
	return ret;
}

/*
 * Read data from I2C device
 */
int bus_i2c_read(void *base, uchar chip, uint addr, int alen, uchar *buf,
		int len)
{
	int ret;
	unsigned int temp;
	int i;
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)base;

	ret = i2c_init_transfer(i2c_regs, chip, addr, alen);
	if (ret < 0)
		return ret;

	temp = readb(&i2c_regs->i2cr);
	temp |= I2CR_RSTA;
	writeb(temp, &i2c_regs->i2cr);

	ret = tx_byte(i2c_regs, (chip << 1) | 1);
	if (ret < 0) {
		i2c_imx_stop(i2c_regs);
		return ret;
	}

	/* setup bus to read data */
	temp = readb(&i2c_regs->i2cr);
	temp &= ~(I2CR_MTX | I2CR_TX_NO_AK);
	if (len == 1)
		temp |= I2CR_TX_NO_AK;
	writeb(temp, &i2c_regs->i2cr);
	writeb(0, &i2c_regs->i2sr);
	readb(&i2c_regs->i2dr);		/* dummy read to clear ICF */

	/* read data */
	for (i = 0; i < len; i++) {
		ret = wait_for_sr_state(i2c_regs, ST_IIF);
		if (ret < 0) {
			i2c_imx_stop(i2c_regs);
			return ret;
		}

		/*
		 * It must generate STOP before read I2DR to prevent
		 * controller from generating another clock cycle
		 */
		if (i == (len - 1)) {
			i2c_imx_stop(i2c_regs);
		} else if (i == (len - 2)) {
			temp = readb(&i2c_regs->i2cr);
			temp |= I2CR_TX_NO_AK;
			writeb(temp, &i2c_regs->i2cr);
		}
		writeb(0, &i2c_regs->i2sr);
		buf[i] = readb(&i2c_regs->i2dr);
	}
	i2c_imx_stop(i2c_regs);
	return 0;
}

/*
 * Write data to I2C device
 */
int bus_i2c_write(void *base, uchar chip, uint addr, int alen,
		const uchar *buf, int len)
{
	int ret;
	int i;
	struct mxc_i2c_regs *i2c_regs = (struct mxc_i2c_regs *)base;

	ret = i2c_init_transfer(i2c_regs, chip, addr, alen);
	if (ret < 0)
		return ret;

	for (i = 0; i < len; i++) {
		ret = tx_byte(i2c_regs, buf[i]);
		if (ret < 0)
			break;
	}
	i2c_imx_stop(i2c_regs);
	return ret;
}

struct i2c_parms {
	void *base;
	void *idle_bus_data;
	int (*idle_bus_fn)(void *p);
};

struct sram_data {
	unsigned curr_i2c_bus;
	struct i2c_parms i2c_data[3];
};

/*
 * For SPL boot some boards need i2c before SDRAM is initialized so force
 * variables to live in SRAM
 */
static struct sram_data __attribute__((section(".data"))) srdata;

void *get_base(void)
{
#ifdef CONFIG_SYS_I2C_BASE
#ifdef CONFIG_I2C_MULTI_BUS
	void *ret = srdata.i2c_data[srdata.curr_i2c_bus].base;
	if (ret)
		return ret;
#endif
	return (void *)CONFIG_SYS_I2C_BASE;
#elif defined(CONFIG_I2C_MULTI_BUS)
	return srdata.i2c_data[srdata.curr_i2c_bus].base;
#else
	return srdata.i2c_data[0].base;
#endif
}

static struct i2c_parms *i2c_get_parms(void *base)
{
	int i = 0;
	struct i2c_parms *p = srdata.i2c_data;
	while (i < ARRAY_SIZE(srdata.i2c_data)) {
		if (p->base == base)
			return p;
		p++;
		i++;
	}
	printf("Invalid I2C base: %p\n", base);
	return NULL;
}

static int i2c_idle_bus(void *base)
{
	struct i2c_parms *p = i2c_get_parms(base);
	if (p && p->idle_bus_fn)
		return p->idle_bus_fn(p->idle_bus_data);
	return 0;
}

#ifdef CONFIG_I2C_MULTI_BUS
unsigned int i2c_get_bus_num(void)
{
	return srdata.curr_i2c_bus;
}

int i2c_set_bus_num(unsigned bus_idx)
{
	if (bus_idx >= ARRAY_SIZE(srdata.i2c_data))
		return -1;
	if (!srdata.i2c_data[bus_idx].base)
		return -1;
	srdata.curr_i2c_bus = bus_idx;
	return 0;
}
#endif

int i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	return bus_i2c_read(get_base(), chip, addr, alen, buf, len);
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	return bus_i2c_write(get_base(), chip, addr, alen, buf, len);
}

/*
 * Test if a chip at a given address responds (probe the chip)
 */
int i2c_probe(uchar chip)
{
	return bus_i2c_write(get_base(), chip, 0, 0, NULL, 0);
}

void bus_i2c_init(void *base, int speed, int unused,
		int (*idle_bus_fn)(void *p), void *idle_bus_data)
{
	int i = 0;
	struct i2c_parms *p = srdata.i2c_data;
	if (!base)
		return;
	for (;;) {
		if (!p->base || (p->base == base)) {
			p->base = base;
			if (idle_bus_fn) {
				p->idle_bus_fn = idle_bus_fn;
				p->idle_bus_data = idle_bus_data;
			}
			break;
		}
		p++;
		i++;
		if (i >= ARRAY_SIZE(srdata.i2c_data))
			return;
	}
	bus_i2c_set_bus_speed(base, speed);
}

/*
 * Init I2C Bus
 */
void i2c_init(int speed, int unused)
{
	bus_i2c_init(get_base(), speed, unused, NULL, NULL);
}

/*
 * Set I2C Speed
 */
int i2c_set_bus_speed(unsigned int speed)
{
	return bus_i2c_set_bus_speed(get_base(), speed);
}

/*
 * Get I2C Speed
 */
unsigned int i2c_get_bus_speed(void)
{
	return bus_i2c_get_bus_speed(get_base());
}
