/*
 * Copyright (C) 2014 Panasonic Corporation
 * Copyright (C) 2015 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <dm/device.h>
#include <dm/root.h>
#include <i2c.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

struct uniphier_fi2c_regs {
	u32 cr;				/* control register */
#define I2C_CR_MST	(1 << 3)	/* master mode */
#define I2C_CR_STA	(1 << 2)	/* start condition */
#define I2C_CR_STO	(1 << 1)	/* stop condition */
#define I2C_CR_NACK	(1 << 0)	/* not ACK */
	u32 dttx;			/* send FIFO (write-only) */
#define dtrx		dttx		/* receive FIFO (read-only) */
#define I2C_DTTX_CMD	(1 << 8)	/* send command (slave addr) */
#define I2C_DTTX_RD	(1 << 0)	/* read */
	u32 __reserved;			/* no register at offset 0x08 */
	u32 slad;			/* slave address */
	u32 cyc;			/* clock cycle control */
	u32 lctl;			/* clock low period control */
	u32 ssut;			/* restart/stop setup time control */
	u32 dsut;			/* data setup time control */
	u32 intr;			/* interrupt status */
	u32 ie;				/* interrupt enable */
	u32 ic;				/* interrupt clear */
#define I2C_INT_TE	(1 << 9)	/* TX FIFO empty */
#define I2C_INT_RB	(1 << 4)	/* received specified bytes */
#define I2C_INT_NA	(1 << 2)	/* no answer */
#define I2C_INT_AL	(1 << 1)	/* arbitration lost */
	u32 sr;				/* status register */
#define I2C_SR_DB	(1 << 12)	/* device busy */
#define I2C_SR_BB	(1 << 8)	/* bus busy */
#define I2C_SR_RFF	(1 << 3)	/* Rx FIFO full */
#define I2C_SR_RNE	(1 << 2)	/* Rx FIFO not empty */
#define I2C_SR_TNF	(1 << 1)	/* Tx FIFO not full */
#define I2C_SR_TFE	(1 << 0)	/* Tx FIFO empty */
	u32 __reserved2;		/* no register at offset 0x30 */
	u32 rst;			/* reset control */
#define I2C_RST_TBRST	(1 << 2)	/* clear Tx FIFO */
#define I2C_RST_RBRST	(1 << 1)	/* clear Rx FIFO */
#define I2C_RST_RST	(1 << 0)	/* forcible bus reset */
	u32 bm;				/* bus monitor */
	u32 noise;			/* noise filter control */
	u32 tbc;			/* Tx byte count setting */
	u32 rbc;			/* Rx byte count setting */
	u32 tbcm;			/* Tx byte count monitor */
	u32 rbcm;			/* Rx byte count monitor */
	u32 brst;			/* bus reset */
#define I2C_BRST_FOEN	(1 << 1)	/* normal operation */
#define I2C_BRST_RSCLO	(1 << 0)	/* release SCL low fixing */
};

#define FIOCLK	50000000

struct uniphier_fi2c_dev {
	struct uniphier_fi2c_regs __iomem *regs;	/* register base */
	unsigned long fioclk;			/* internal operation clock */
	unsigned long timeout;			/* time out (us) */
};

static int poll_status(u32 __iomem *reg, u32 flag)
{
	int wait = 1000000; /* 1 sec is long enough */

	while (readl(reg) & flag) {
		if (wait-- < 0)
			return -EREMOTEIO;
		udelay(1);
	}

	return 0;
}

static int reset_bus(struct uniphier_fi2c_regs __iomem *regs)
{
	int ret;

	/* bus forcible reset */
	writel(I2C_RST_RST, &regs->rst);
	ret = poll_status(&regs->rst, I2C_RST_RST);
	if (ret < 0)
		debug("error: fail to reset I2C controller\n");

	return ret;
}

static int check_device_busy(struct uniphier_fi2c_regs __iomem *regs)
{
	int ret;

	ret = poll_status(&regs->sr, I2C_SR_DB);
	if (ret < 0) {
		debug("error: device busy too long. reset...\n");
		ret = reset_bus(regs);
	}

	return ret;
}

static int uniphier_fi2c_probe(struct udevice *dev)
{
	fdt_addr_t addr;
	fdt_size_t size;
	struct uniphier_fi2c_dev *priv = dev_get_priv(dev);
	int ret;

	addr = fdtdec_get_addr_size(gd->fdt_blob, dev->of_offset, "reg",
				    &size);

	priv->regs = map_sysmem(addr, size);

	if (!priv->regs)
		return -ENOMEM;

	priv->fioclk = FIOCLK;

	/* bus forcible reset */
	ret = reset_bus(priv->regs);
	if (ret < 0)
		return ret;

	writel(I2C_BRST_FOEN | I2C_BRST_RSCLO, &priv->regs->brst);

	return 0;
}

static int uniphier_fi2c_remove(struct udevice *dev)
{
	struct uniphier_fi2c_dev *priv = dev_get_priv(dev);

	unmap_sysmem(priv->regs);

	return 0;
}

static int wait_for_irq(struct uniphier_fi2c_dev *dev, u32 flags,
			bool *stop)
{
	u32 irq;
	unsigned long wait = dev->timeout;
	int ret = -EREMOTEIO;

	do {
		udelay(1);
		irq = readl(&dev->regs->intr);
	} while (!(irq & flags) && wait--);

	if (wait < 0) {
		debug("error: time out\n");
		return ret;
	}

	if (irq & I2C_INT_AL) {
		debug("error: arbitration lost\n");
		*stop = false;
		return ret;
	}

	if (irq & I2C_INT_NA) {
		debug("error: no answer\n");
		return ret;
	}

	return 0;
}

static int issue_stop(struct uniphier_fi2c_dev *dev, int old_ret)
{
	int ret;

	debug("stop condition\n");
	writel(I2C_CR_MST | I2C_CR_STO, &dev->regs->cr);

	ret = poll_status(&dev->regs->sr, I2C_SR_DB);
	if (ret < 0)
		debug("error: device busy after operation\n");

	return old_ret ? old_ret : ret;
}

static int uniphier_fi2c_transmit(struct uniphier_fi2c_dev *dev, uint addr,
				  uint len, const u8 *buf, bool *stop)
{
	int ret;
	const u32 irq_flags = I2C_INT_TE | I2C_INT_NA | I2C_INT_AL;
	struct uniphier_fi2c_regs __iomem *regs = dev->regs;

	debug("%s: addr = %x, len = %d\n", __func__, addr, len);

	writel(I2C_DTTX_CMD | addr << 1, &regs->dttx);

	writel(irq_flags, &regs->ie);
	writel(irq_flags, &regs->ic);

	debug("start condition\n");
	writel(I2C_CR_MST | I2C_CR_STA, &regs->cr);

	ret = wait_for_irq(dev, irq_flags, stop);
	if (ret < 0)
		goto error;

	while (len--) {
		debug("sending %x\n", *buf);
		writel(*buf++, &regs->dttx);

		writel(irq_flags, &regs->ic);

		ret = wait_for_irq(dev, irq_flags, stop);
		if (ret < 0)
			goto error;
	}

error:
	writel(irq_flags, &regs->ic);

	if (*stop)
		ret = issue_stop(dev, ret);

	return ret;
}

static int uniphier_fi2c_receive(struct uniphier_fi2c_dev *dev, uint addr,
				 uint len, u8 *buf, bool *stop)
{
	int ret = 0;
	const u32 irq_flags = I2C_INT_RB | I2C_INT_NA | I2C_INT_AL;
	struct uniphier_fi2c_regs __iomem *regs = dev->regs;

	debug("%s: addr = %x, len = %d\n", __func__, addr, len);

	/*
	 * In case 'len == 0', only the slave address should be sent
	 * for probing, which is covered by the transmit function.
	 */
	if (len == 0)
		return uniphier_fi2c_transmit(dev, addr, len, buf, stop);

	writel(I2C_DTTX_CMD | I2C_DTTX_RD | addr << 1, &regs->dttx);

	writel(0, &regs->rbc);
	writel(irq_flags, &regs->ie);
	writel(irq_flags, &regs->ic);

	debug("start condition\n");
	writel(I2C_CR_MST | I2C_CR_STA | (len == 1 ? I2C_CR_NACK : 0),
	       &regs->cr);

	while (len--) {
		ret = wait_for_irq(dev, irq_flags, stop);
		if (ret < 0)
			goto error;

		*buf++ = readl(&regs->dtrx);
		debug("received %x\n", *(buf - 1));

		if (len == 1)
			writel(I2C_CR_MST | I2C_CR_NACK, &regs->cr);

		writel(irq_flags, &regs->ic);
	}

error:
	writel(irq_flags, &regs->ic);

	if (*stop)
		ret = issue_stop(dev, ret);

	return ret;
}

static int uniphier_fi2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			     int nmsgs)
{
	int ret;
	struct uniphier_fi2c_dev *dev = dev_get_priv(bus);
	bool stop;

	ret = check_device_busy(dev->regs);
	if (ret < 0)
		return ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		/* If next message is read, skip the stop condition */
		stop = nmsgs > 1 && msg[1].flags & I2C_M_RD ? false : true;

		if (msg->flags & I2C_M_RD)
			ret = uniphier_fi2c_receive(dev, msg->addr, msg->len,
						    msg->buf, &stop);
		else
			ret = uniphier_fi2c_transmit(dev, msg->addr, msg->len,
						     msg->buf, &stop);

		if (ret < 0)
			break;
	}

	return ret;
}

static int uniphier_fi2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	int ret;
	unsigned int clk_count;
	struct uniphier_fi2c_dev *dev = dev_get_priv(bus);
	struct uniphier_fi2c_regs __iomem *regs = dev->regs;

	/* max supported frequency is 400 kHz */
	if (speed > 400000)
		return -EINVAL;

	ret = check_device_busy(dev->regs);
	if (ret < 0)
		return ret;

	/* make sure the bus is idle when changing the frequency */
	writel(I2C_BRST_RSCLO, &regs->brst);

	clk_count = dev->fioclk / speed;

	writel(clk_count, &regs->cyc);
	writel(clk_count / 2, &regs->lctl);
	writel(clk_count / 2, &regs->ssut);
	writel(clk_count / 16, &regs->dsut);

	writel(I2C_BRST_FOEN | I2C_BRST_RSCLO, &regs->brst);

	/*
	 * Theoretically, each byte can be transferred in
	 * 1000000 * 9 / speed usec.
	 * This time out value is long enough.
	 */
	dev->timeout = 100000000L / speed;

	return 0;
}

static const struct dm_i2c_ops uniphier_fi2c_ops = {
	.xfer = uniphier_fi2c_xfer,
	.set_bus_speed = uniphier_fi2c_set_bus_speed,
};

static const struct udevice_id uniphier_fi2c_of_match[] = {
	{ .compatible = "socionext,uniphier-fi2c" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_fi2c) = {
	.name = "uniphier-fi2c",
	.id = UCLASS_I2C,
	.of_match = uniphier_fi2c_of_match,
	.probe = uniphier_fi2c_probe,
	.remove = uniphier_fi2c_remove,
	.priv_auto_alloc_size = sizeof(struct uniphier_fi2c_dev),
	.ops = &uniphier_fi2c_ops,
};
