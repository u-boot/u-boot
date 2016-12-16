/*
 * LPC32xx I2C interface driver
 *
 * (C) Copyright 2014-2015  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD - 3ADEV <albert.aribaud@3adev.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * NOTE: This driver should be converted to driver model before June 2017.
 * Please see doc/driver-model/i2c-howto.txt for instructions.
 */

#include <common.h>
#include <asm/io.h>
#include <i2c.h>
#include <linux/errno.h>
#include <asm/arch/clk.h>

/*
 * Provide default speed and slave if target did not
 */

#if !defined(CONFIG_SYS_I2C_LPC32XX_SPEED)
#define CONFIG_SYS_I2C_LPC32XX_SPEED 350000
#endif

#if !defined(CONFIG_SYS_I2C_LPC32XX_SLAVE)
#define CONFIG_SYS_I2C_LPC32XX_SLAVE 0
#endif

/* i2c register set */
struct lpc32xx_i2c_registers {
	union {
		u32 rx;
		u32 tx;
	};
	u32 stat;
	u32 ctrl;
	u32 clk_hi;
	u32 clk_lo;
	u32 adr;
	u32 rxfl;
	u32 txfl;
	u32 rxb;
	u32 txb;
	u32 stx;
	u32 stxfl;
};

/* TX register fields */
#define LPC32XX_I2C_TX_START		0x00000100
#define LPC32XX_I2C_TX_STOP		0x00000200

/* Control register values */
#define LPC32XX_I2C_SOFT_RESET		0x00000100

/* Status register values */
#define LPC32XX_I2C_STAT_TFF		0x00000400
#define LPC32XX_I2C_STAT_RFE		0x00000200
#define LPC32XX_I2C_STAT_DRMI		0x00000008
#define LPC32XX_I2C_STAT_NAI		0x00000004
#define LPC32XX_I2C_STAT_TDI		0x00000001

static struct lpc32xx_i2c_registers *lpc32xx_i2c[] = {
	(struct lpc32xx_i2c_registers *)I2C1_BASE,
	(struct lpc32xx_i2c_registers *)I2C2_BASE,
	(struct lpc32xx_i2c_registers *)(USB_BASE + 0x300)
};

/* Set I2C bus speed */
static unsigned int lpc32xx_i2c_set_bus_speed(struct i2c_adapter *adap,
			unsigned int speed)
{
	int half_period;

	if (speed == 0)
		return -EINVAL;

	/* OTG I2C clock source and CLK registers are different */
	if (adap->hwadapnr == 2) {
		half_period = (get_periph_clk_rate() / speed) / 2;
		if (half_period > 0xFF)
			return -EINVAL;
	} else {
		half_period = (get_hclk_clk_rate() / speed) / 2;
		if (half_period > 0x3FF)
			return -EINVAL;
	}

	writel(half_period, &lpc32xx_i2c[adap->hwadapnr]->clk_hi);
	writel(half_period, &lpc32xx_i2c[adap->hwadapnr]->clk_lo);
	return 0;
}

/* I2C init called by cmd_i2c when doing 'i2c reset'. */
static void _i2c_init(struct i2c_adapter *adap,
	int requested_speed, int slaveadd)
{
	struct lpc32xx_i2c_registers *i2c = lpc32xx_i2c[adap->hwadapnr];

	/* soft reset (auto-clears) */
	writel(LPC32XX_I2C_SOFT_RESET, &i2c->ctrl);
	/* set HI and LO periods for half of the default speed */
	lpc32xx_i2c_set_bus_speed(adap, requested_speed);
}

/* I2C probe called by cmd_i2c when doing 'i2c probe'. */
static int lpc32xx_i2c_probe(struct i2c_adapter *adap, u8 dev)
{
	struct lpc32xx_i2c_registers *i2c = lpc32xx_i2c[adap->hwadapnr];
	int stat;

	/* Soft-reset the controller */
	writel(LPC32XX_I2C_SOFT_RESET, &i2c->ctrl);
	while (readl(&i2c->ctrl) & LPC32XX_I2C_SOFT_RESET)
		;
	/* Addre slave for write with start before and stop after */
	writel((dev<<1) | LPC32XX_I2C_TX_START | LPC32XX_I2C_TX_STOP,
	       &i2c->tx);
	/* wait for end of transation */
	while (!((stat = readl(&i2c->stat)) & LPC32XX_I2C_STAT_TDI))
		;
	/* was there no acknowledge? */
	return (stat & LPC32XX_I2C_STAT_NAI) ? -1 : 0;
}

/*
 * I2C read called by cmd_i2c when doing 'i2c read' and by cmd_eeprom.c
 * Begin write, send address byte(s), begin read, receive data bytes, end.
 */
static int lpc32xx_i2c_read(struct i2c_adapter *adap, u8 dev, uint addr,
			 int alen, u8 *data, int length)
{
	struct lpc32xx_i2c_registers *i2c = lpc32xx_i2c[adap->hwadapnr];
	int stat, wlen;

	/* Soft-reset the controller */
	writel(LPC32XX_I2C_SOFT_RESET, &i2c->ctrl);
	while (readl(&i2c->ctrl) & LPC32XX_I2C_SOFT_RESET)
		;
	/* do we need to write an address at all? */
	if (alen) {
		/* Address slave in write mode */
		writel((dev<<1) | LPC32XX_I2C_TX_START, &i2c->tx);
		/* write address bytes */
		while (alen--) {
			/* compute address byte + stop for the last one */
			int a = (addr >> (8 * alen)) & 0xff;
			if (!alen)
				a |= LPC32XX_I2C_TX_STOP;
			/* Send address byte */
			writel(a, &i2c->tx);
		}
		/* wait for end of transation */
		while (!((stat = readl(&i2c->stat)) & LPC32XX_I2C_STAT_TDI))
			;
		/* clear end-of-transaction flag */
		writel(1, &i2c->stat);
	}
	/* do we have to read data at all? */
	if (length) {
		/* Address slave in read mode */
		writel(1 | (dev<<1) | LPC32XX_I2C_TX_START, &i2c->tx);
		wlen = length;
		/* get data */
		while (length | wlen) {
			/* read status for TFF and RFE */
			stat = readl(&i2c->stat);
			/* must we, can we write a trigger byte? */
			if ((wlen > 0)
			   & (!(stat & LPC32XX_I2C_STAT_TFF))) {
				wlen--;
				/* write trigger byte + stop if last */
				writel(wlen ? 0 :
				LPC32XX_I2C_TX_STOP, &i2c->tx);
			}
			/* must we, can we read a data byte? */
			if ((length > 0)
			   & (!(stat & LPC32XX_I2C_STAT_RFE))) {
				length--;
				/* read byte */
				*(data++) = readl(&i2c->rx);
			}
		}
		/* wait for end of transation */
		while (!((stat = readl(&i2c->stat)) & LPC32XX_I2C_STAT_TDI))
			;
		/* clear end-of-transaction flag */
		writel(1, &i2c->stat);
	}
	/* success */
	return 0;
}

/*
 * I2C write called by cmd_i2c when doing 'i2c write' and by cmd_eeprom.c
 * Begin write, send address byte(s), send data bytes, end.
 */
static int lpc32xx_i2c_write(struct i2c_adapter *adap, u8 dev, uint addr,
			  int alen, u8 *data, int length)
{
	struct lpc32xx_i2c_registers *i2c = lpc32xx_i2c[adap->hwadapnr];
	int stat;

	/* Soft-reset the controller */
	writel(LPC32XX_I2C_SOFT_RESET, &i2c->ctrl);
	while (readl(&i2c->ctrl) & LPC32XX_I2C_SOFT_RESET)
		;
	/* do we need to write anything at all? */
	if (alen | length)
		/* Address slave in write mode */
		writel((dev<<1) | LPC32XX_I2C_TX_START, &i2c->tx);
	else
		return 0;
	/* write address bytes */
	while (alen) {
		/* wait for transmit fifo not full */
		stat = readl(&i2c->stat);
		if (!(stat & LPC32XX_I2C_STAT_TFF)) {
			alen--;
			int a = (addr >> (8 * alen)) & 0xff;
			if (!(alen | length))
				a |= LPC32XX_I2C_TX_STOP;
			/* Send address byte */
			writel(a, &i2c->tx);
		}
	}
	while (length) {
		/* wait for transmit fifo not full */
		stat = readl(&i2c->stat);
		if (!(stat & LPC32XX_I2C_STAT_TFF)) {
			/* compute data byte, add stop if length==0 */
			length--;
			int d = *(data++);
			if (!length)
				d |= LPC32XX_I2C_TX_STOP;
			/* Send data byte */
			writel(d, &i2c->tx);
		}
	}
	/* wait for end of transation */
	while (!((stat = readl(&i2c->stat)) & LPC32XX_I2C_STAT_TDI))
		;
	/* clear end-of-transaction flag */
	writel(1, &i2c->stat);
	return 0;
}

U_BOOT_I2C_ADAP_COMPLETE(lpc32xx_0, _i2c_init, lpc32xx_i2c_probe,
			 lpc32xx_i2c_read, lpc32xx_i2c_write,
			 lpc32xx_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_LPC32XX_SPEED,
			 CONFIG_SYS_I2C_LPC32XX_SLAVE,
			 0)

U_BOOT_I2C_ADAP_COMPLETE(lpc32xx_1, _i2c_init, lpc32xx_i2c_probe,
			 lpc32xx_i2c_read, lpc32xx_i2c_write,
			 lpc32xx_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_LPC32XX_SPEED,
			 CONFIG_SYS_I2C_LPC32XX_SLAVE,
			 1)

U_BOOT_I2C_ADAP_COMPLETE(lpc32xx_2, _i2c_init, NULL,
			 lpc32xx_i2c_read, lpc32xx_i2c_write,
			 lpc32xx_i2c_set_bus_speed,
			 100000,
			 0,
			 2)
