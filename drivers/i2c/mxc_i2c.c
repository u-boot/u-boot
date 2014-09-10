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
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <i2c.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef I2C_QUIRK_REG
struct mxc_i2c_regs {
	uint8_t		iadr;
	uint8_t		ifdr;
	uint8_t		i2cr;
	uint8_t		i2sr;
	uint8_t		i2dr;
};
#else
struct mxc_i2c_regs {
	uint32_t	iadr;
	uint32_t	ifdr;
	uint32_t	i2cr;
	uint32_t	i2sr;
	uint32_t	i2dr;
};
#endif

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

#ifdef I2C_QUIRK_REG
#define I2CR_IEN	(0 << 7)
#define I2CR_IDIS	(1 << 7)
#define I2SR_IIF_CLEAR	(1 << 1)
#else
#define I2CR_IEN	(1 << 7)
#define I2CR_IDIS	(0 << 7)
#define I2SR_IIF_CLEAR	(0 << 1)
#endif

#if defined(CONFIG_HARD_I2C) && !defined(CONFIG_SYS_I2C_BASE)
#error "define CONFIG_SYS_I2C_BASE to use the mxc_i2c driver"
#endif

#ifdef I2C_QUIRK_REG
static u16 i2c_clk_div[60][2] = {
	{ 20,	0x00 }, { 22,	0x01 }, { 24,	0x02 }, { 26,	0x03 },
	{ 28,	0x04 },	{ 30,	0x05 },	{ 32,	0x09 }, { 34,	0x06 },
	{ 36,	0x0A }, { 40,	0x07 }, { 44,	0x0C }, { 48,	0x0D },
	{ 52,	0x43 },	{ 56,	0x0E }, { 60,	0x45 }, { 64,	0x12 },
	{ 68,	0x0F },	{ 72,	0x13 },	{ 80,	0x14 },	{ 88,	0x15 },
	{ 96,	0x19 },	{ 104,	0x16 },	{ 112,	0x1A },	{ 128,	0x17 },
	{ 136,	0x4F }, { 144,	0x1C },	{ 160,	0x1D }, { 176,	0x55 },
	{ 192,	0x1E }, { 208,	0x56 },	{ 224,	0x22 }, { 228,	0x24 },
	{ 240,	0x1F },	{ 256,	0x23 }, { 288,	0x5C },	{ 320,	0x25 },
	{ 384,	0x26 }, { 448,	0x2A },	{ 480,	0x27 }, { 512,	0x2B },
	{ 576,	0x2C },	{ 640,	0x2D },	{ 768,	0x31 }, { 896,	0x32 },
	{ 960,	0x2F },	{ 1024,	0x33 },	{ 1152,	0x34 }, { 1280,	0x35 },
	{ 1536,	0x36 }, { 1792,	0x3A },	{ 1920,	0x37 },	{ 2048,	0x3B },
	{ 2304,	0x3C },	{ 2560,	0x3D },	{ 3072,	0x3E }, { 3584,	0x7A },
	{ 3840,	0x3F }, { 4096,	0x7B }, { 5120,	0x7D },	{ 6144,	0x7E },
};
#else
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
#endif


#ifndef CONFIG_SYS_MXC_I2C1_SPEED
#define CONFIG_SYS_MXC_I2C1_SPEED 100000
#endif
#ifndef CONFIG_SYS_MXC_I2C2_SPEED
#define CONFIG_SYS_MXC_I2C2_SPEED 100000
#endif
#ifndef CONFIG_SYS_MXC_I2C3_SPEED
#define CONFIG_SYS_MXC_I2C3_SPEED 100000
#endif

#ifndef CONFIG_SYS_MXC_I2C1_SLAVE
#define CONFIG_SYS_MXC_I2C1_SLAVE 0
#endif
#ifndef CONFIG_SYS_MXC_I2C2_SLAVE
#define CONFIG_SYS_MXC_I2C2_SLAVE 0
#endif
#ifndef CONFIG_SYS_MXC_I2C3_SLAVE
#define CONFIG_SYS_MXC_I2C3_SLAVE 0
#endif


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
	writeb(I2CR_IDIS, &i2c_regs->i2cr);
	writeb(0, &i2c_regs->i2sr);
	return 0;
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
#ifdef I2C_QUIRK_REG
			writeb(sr | I2SR_IAL, &i2c_regs->i2sr);
#else
			writeb(sr & ~I2SR_IAL, &i2c_regs->i2sr);
#endif
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

	writeb(I2SR_IIF_CLEAR, &i2c_regs->i2sr);
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
#ifdef I2C_QUIRK_REG
	if (readb(&i2c_regs->i2cr) & I2CR_IDIS) {
#else
	if (!(readb(&i2c_regs->i2cr) & I2CR_IEN)) {
#endif
		writeb(I2CR_IEN, &i2c_regs->i2cr);
		/* Wait for controller to be stable */
		udelay(50);
	}
	if (readb(&i2c_regs->iadr) == (chip << 1))
		writeb((chip << 1) ^ 2, &i2c_regs->iadr);
	writeb(I2SR_IIF_CLEAR, &i2c_regs->i2sr);
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
			/* Disable controller */
			writeb(I2CR_IDIS, &i2c_regs->i2cr);
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
	writeb(I2SR_IIF_CLEAR, &i2c_regs->i2sr);
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
		writeb(I2SR_IIF_CLEAR, &i2c_regs->i2sr);
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

static void * const i2c_bases[] = {
#if defined(CONFIG_MX25)
	(void *)IMX_I2C_BASE,
	(void *)IMX_I2C2_BASE,
	(void *)IMX_I2C3_BASE
#elif defined(CONFIG_MX27)
	(void *)IMX_I2C1_BASE,
	(void *)IMX_I2C2_BASE
#elif defined(CONFIG_MX31) || defined(CONFIG_MX35) || \
	defined(CONFIG_MX51) || defined(CONFIG_MX53) ||	\
	defined(CONFIG_MX6) || defined(CONFIG_LS102XA)
	(void *)I2C1_BASE_ADDR,
	(void *)I2C2_BASE_ADDR,
	(void *)I2C3_BASE_ADDR
#elif defined(CONFIG_VF610)
	(void *)I2C0_BASE_ADDR
#elif defined(CONFIG_FSL_LSCH3)
	(void *)I2C1_BASE_ADDR,
	(void *)I2C2_BASE_ADDR,
	(void *)I2C3_BASE_ADDR,
	(void *)I2C4_BASE_ADDR
#else
#error "architecture not supported"
#endif
};

void *i2c_get_base(struct i2c_adapter *adap)
{
	return i2c_bases[adap->hwadapnr];
}

static struct i2c_parms *i2c_get_parms(void *base)
{
	struct sram_data *srdata = (void *)gd->srdata;
	int i = 0;
	struct i2c_parms *p = srdata->i2c_data;
	while (i < ARRAY_SIZE(srdata->i2c_data)) {
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

static int mxc_i2c_read(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len)
{
	return bus_i2c_read(i2c_get_base(adap), chip, addr, alen, buffer, len);
}

static int mxc_i2c_write(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len)
{
	return bus_i2c_write(i2c_get_base(adap), chip, addr, alen, buffer, len);
}

/*
 * Test if a chip at a given address responds (probe the chip)
 */
static int mxc_i2c_probe(struct i2c_adapter *adap, uint8_t chip)
{
	return bus_i2c_write(i2c_get_base(adap), chip, 0, 0, NULL, 0);
}

void bus_i2c_init(void *base, int speed, int unused,
		int (*idle_bus_fn)(void *p), void *idle_bus_data)
{
	struct sram_data *srdata = (void *)gd->srdata;
	int i = 0;
	struct i2c_parms *p = srdata->i2c_data;
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
		if (i >= ARRAY_SIZE(srdata->i2c_data))
			return;
	}
	bus_i2c_set_bus_speed(base, speed);
}

/*
 * Init I2C Bus
 */
static void mxc_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
	bus_i2c_init(i2c_get_base(adap), speed, slaveaddr, NULL, NULL);
}

/*
 * Set I2C Speed
 */
static uint mxc_i2c_set_bus_speed(struct i2c_adapter *adap, uint speed)
{
	return bus_i2c_set_bus_speed(i2c_get_base(adap), speed);
}

/*
 * Register mxc i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(mxc0, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C1_SPEED,
			 CONFIG_SYS_MXC_I2C1_SLAVE, 0)
U_BOOT_I2C_ADAP_COMPLETE(mxc1, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C2_SPEED,
			 CONFIG_SYS_MXC_I2C2_SLAVE, 1)
#if defined(CONFIG_MX31) || defined(CONFIG_MX35) ||\
	defined(CONFIG_MX51) || defined(CONFIG_MX53) ||\
	defined(CONFIG_MX6) || defined(CONFIG_LS102XA)
U_BOOT_I2C_ADAP_COMPLETE(mxc2, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C3_SPEED,
			 CONFIG_SYS_MXC_I2C3_SLAVE, 2)
#endif
