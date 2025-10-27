// SPDX-License-Identifier: GPL-2.0+
/*
 * U-Boot driver for the MediaTek MT7621 I2C controller.
 *
 * Derived from the Linux kernel driver:
 *   drivers/i2c/busses/i2c-mt7621.c
 *
 * Copyright (C) 2013 Steven Liu <steven_liu@mediatek.com>
 * Copyright (C) 2014 Sittisak <sittisaks@hotmail.com>
 * Copyright (C) 2016 Michael Lee <igvtee@gmail.com>
 * Copyright (C) 2018 Jan Breuer <jan.breuer@jaybee.cz>
 * Copyright (C) 2025 Justin Swartz <justin.swartz@risingedge.co.za>
 */

#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <dm.h>
#include <clk.h>
#include <i2c.h>
#include <log.h>
#include <reset.h>
#include <time.h>

#define REG_SM0CFG2                   0x28
#define REG_SM0CTL0                   0x40
#define REG_SM0CTL1                   0x44
#define REG_SM0D0                     0x50
#define REG_SM0D1                     0x54

#define SM0CFG2_MODE_MANUAL           0

#define SM0CTL0_ODRAIN                BIT(31)
#define SM0CTL0_CLK_DIV_MASK          (0x7ff << 16)
#define SM0CTL0_CLK_DIV_MAX           0x7ff
#define SM0CTL0_EN                    BIT(1)
#define SM0CTL0_SCL_STRETCH           BIT(0)

#define SM0CTL1_TRI                   BIT(0)
#define SM0CTL1_TRI_IDLE              0
#define SM0CTL1_START                 (1 << 4)
#define SM0CTL1_WRITE                 (2 << 4)
#define SM0CTL1_STOP                  (3 << 4)
#define SM0CTL1_READ_LAST             (4 << 4)
#define SM0CTL1_READ                  (5 << 4)
#define SM0CTL1_PGLEN(x)              ((((x) - 1) << 8) & SM0CTL1_PGLEN_MASK)
#define SM0CTL1_PGLEN_MASK            (0x7 << 8)
#define SM0CTL1_ACK_MASK              (0xff << 16)

#define TIMEOUT_1SEC                  1000
#define I2C_MAX_STD_MODE_FREQ         100000

struct mt7621_i2c_priv {
	void __iomem *base;
	uint speed;
	u32 clk_div;
	struct clk clk;
	struct reset_ctl reset_ctl;
};

static int mt7621_i2c_wait_idle(struct udevice *dev)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	ulong start_time = get_timer(0);
	u32 value;

	while (get_timer(start_time) < TIMEOUT_1SEC) {
		value = readl(priv->base + REG_SM0CTL1);
		if ((value & SM0CTL1_TRI) == SM0CTL1_TRI_IDLE)
			return 0;

		udelay(10);
	}

	return -ETIMEDOUT;
}

static int mt7621_i2c_reset(struct udevice *dev)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	u32 value;

	reset_assert(&priv->reset_ctl);
	udelay(100);
	reset_deassert(&priv->reset_ctl);

	value = readl(priv->base + REG_SM0CTL0);
	value &= ~SM0CTL0_CLK_DIV_MASK;
	value |= (priv->clk_div << 16) & SM0CTL0_CLK_DIV_MASK;
	value |= SM0CTL0_EN | SM0CTL0_SCL_STRETCH;
	writel(value, priv->base + REG_SM0CTL0);

	writel(SM0CFG2_MODE_MANUAL, priv->base + REG_SM0CFG2);
	return 0;
}

static int mt7621_i2c_master_start(struct udevice *dev)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);

	writel(SM0CTL1_START | SM0CTL1_TRI, priv->base + REG_SM0CTL1);
	return mt7621_i2c_wait_idle(dev);
}

static int mt7621_i2c_master_stop(struct udevice *dev)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);

	writel(SM0CTL1_STOP | SM0CTL1_TRI, priv->base + REG_SM0CTL1);
	return mt7621_i2c_wait_idle(dev);
}

static int mt7621_i2c_master_cmd(struct udevice *dev, u32 cmd, int len)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);

	writel(cmd | SM0CTL1_TRI | SM0CTL1_PGLEN(len),
	       priv->base + REG_SM0CTL1);

	return mt7621_i2c_wait_idle(dev);
}

static int mt7621_i2c_7bit_address(struct udevice *dev, struct i2c_msg *msg)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	u32 addr = msg->addr << 1;

	if (msg->flags & I2C_M_RD)
		addr |= 1;

	writel(addr, priv->base + REG_SM0D0);
	return mt7621_i2c_master_cmd(dev, SM0CTL1_WRITE, 1);
}

static int mt7621_i2c_10bit_address(struct udevice *dev, struct i2c_msg *msg)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	u16 addr = 0xf0 | ((msg->addr >> 7) & 0x06) | (msg->addr & 0xff) << 8;

	if (msg->flags & I2C_M_RD)
		addr |= 1;

	writel(addr, priv->base + REG_SM0D0);
	return mt7621_i2c_master_cmd(dev, SM0CTL1_WRITE, 2);
}

static int mt7621_i2c_address(struct udevice *dev, struct i2c_msg *msg)
{
	int ret;

	if (msg->flags & I2C_M_TEN) {
		ret = mt7621_i2c_10bit_address(dev, msg);
		if (ret)
			return ret;
	} else {
		ret = mt7621_i2c_7bit_address(dev, msg);
		if (ret)
			return ret;
	}

	return 0;
}

static int mt7621_i2c_check_ack(struct udevice *dev, struct i2c_msg *msg,
				u32 length)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	u32 status = readl(priv->base + REG_SM0CTL1);
	u32 expected = GENMASK(length - 1, 0);
	u32 mask = (expected << 16) & SM0CTL1_ACK_MASK;

	if (msg->flags & I2C_M_IGNORE_NAK)
		return 0;

	if ((status & mask) != mask)
		return -ENXIO;

	return 0;
}

static int mt7621_i2c_master_read(struct udevice *dev, struct i2c_msg *msg)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	int offset, length, last, ret;
	u32 cmd;
	u32 data[2];

	for (offset = 0; offset < msg->len; offset += 8) {
		if (msg->len - offset >= 8)
			length = 8;
		else
			length = msg->len - offset;

		last = msg->len - offset <= 8;
		cmd = last ? SM0CTL1_READ_LAST : SM0CTL1_READ;
		ret = mt7621_i2c_master_cmd(dev, cmd, length);
		if (ret)
			return ret;

		data[0] = readl(priv->base + REG_SM0D0);
		data[1] = readl(priv->base + REG_SM0D1);
		memcpy(&msg->buf[offset], data, length);
	}

	return 0;
}

static int mt7621_i2c_master_write(struct udevice *dev, struct i2c_msg *msg)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	int offset, length, ret;
	u32 data[2];

	for (offset = 0; offset < msg->len; offset += 8) {
		if (msg->len - offset >= 8)
			length = 8;
		else
			length = msg->len - offset;

		memcpy(data, &msg->buf[offset], length);
		writel(data[0], priv->base + REG_SM0D0);
		writel(data[1], priv->base + REG_SM0D1);

		ret = mt7621_i2c_master_cmd(dev, SM0CTL1_WRITE, length);
		if (ret)
			return ret;

		ret = mt7621_i2c_check_ack(dev, msg, length);
		if (ret)
			return ret;
	}

	return 0;
}

static int mt7621_i2c_xfer(struct udevice *dev, struct i2c_msg *msgs, int count)
{
	struct i2c_msg *msg;
	int index, ret;

	for (index = 0; index < count; index++) {
		msg = &msgs[index];

		ret = mt7621_i2c_wait_idle(dev);
		if (ret)
			goto reset;

		ret = mt7621_i2c_master_start(dev);
		if (ret)
			goto reset;

		ret = mt7621_i2c_address(dev, msg);
		if (ret)
			goto reset;

		ret = mt7621_i2c_check_ack(dev, msg, 1);
		if (ret)
			goto stop;

		if (msg->flags & I2C_M_RD) {
			ret = mt7621_i2c_master_read(dev, msg);
			if (ret)
				goto reset;
		} else {
			ret = mt7621_i2c_master_write(dev, msg);
			if (ret)
				goto reset;
		}
	}

	ret = mt7621_i2c_wait_idle(dev);
	if (ret)
		goto reset;

	ret = mt7621_i2c_master_stop(dev);
	if (ret)
		goto reset;

	return 0;

stop:
	ret = mt7621_i2c_master_stop(dev);
	if (ret)
		goto reset;
	return -ENXIO;

reset:
	mt7621_i2c_reset(dev);
	return ret;
}

static int mt7621_i2c_set_speed(struct udevice *dev, uint speed)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	ulong clk_rate = clk_get_rate(&priv->clk);

	priv->speed = speed;
	priv->clk_div = clk_rate / priv->speed - 1;

	if (priv->clk_div < 99)
		priv->clk_div = 99;

	if (priv->clk_div > SM0CTL0_CLK_DIV_MAX)
		priv->clk_div = SM0CTL0_CLK_DIV_MAX;

	return 0;
}

static const struct dm_i2c_ops mt7621_i2c_ops = {
	.xfer          = mt7621_i2c_xfer,
	.set_bus_speed = mt7621_i2c_set_speed,
	.deblock       = mt7621_i2c_reset,
};

static int mt7621_i2c_of_to_plat(struct udevice *dev)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	return 0;
}

int mt7621_i2c_probe(struct udevice *dev)
{
	struct mt7621_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base) {
		dev_err(dev, "failed to get base address\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "sys_clock", &priv->clk);
	if (ret) {
		dev_err(dev, "failed to get clock source\n");
		return ret;
	}

	ret = reset_get_by_name(dev, "i2c_reset", &priv->reset_ctl);
	if (ret) {
		dev_err(dev, "failed to get reset control\n");
		return ret;
	}

	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	mt7621_i2c_set_speed(dev, I2C_MAX_STD_MODE_FREQ);
	mt7621_i2c_reset(dev);

	return 0;
}

static const struct udevice_id mt7621_i2c_ids[] = {
	{ .compatible = "mediatek,mt7621-i2c" },
	{ }
};

U_BOOT_DRIVER(mt7621_i2c) = {
	.name       = "mt7621_i2c",
	.id         = UCLASS_I2C,
	.of_match   = mt7621_i2c_ids,
	.of_to_plat = mt7621_i2c_of_to_plat,
	.probe      = mt7621_i2c_probe,
	.priv_auto  = sizeof(struct mt7621_i2c_priv),
	.ops        = &mt7621_i2c_ops,
};
