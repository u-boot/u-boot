// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023-2026 Spacemit, Inc
 * Copyright (C) 2025-2026 RISCStar Ltd.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <i2c.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <reset.h>
#include "k1_i2c.h"

#define ICR_OFFSET		0x00
#define ISR_OFFSET		0x04
#define ISAR_OFFSET		0x08
#define IDBR_OFFSET		0x0c
#define ILCR_OFFSET		0x10
#define IWCR_OFFSET		0x14
#define IRCR_OFFSET		0x18
#define IBMR_OFFSET		0x1c
#define WFIFO_OFFSET		0x20
#define WFIFO_WPTR_OFFSET	0x24
#define WFIFO_RPTR_OFFSET	0x28
#define RFIFO_OFFSET		0x2c
#define RFIFO_WPTR_OFFSET	0x30
#define RFIFO_RPTR_OFFSET	0x34

/* All transfers are described by this data structure */
struct k1_i2c_msg {
	u8 condition;
	u8 acknack;
	u8 direction;
	u8 data;
};

struct k1_i2c {
	u32 icr;
	u32 isr;
	u32 isar;
	u32 idbr;
	u32 ilcr;
	u32 iwcr;
	u32 irst_cyc;
	u32 ibmr;
};

struct k1_i2c_priv {
	int id;
	void __iomem *base;
	struct reset_ctl_bulk resets;
	struct clk clk;
	u32 clk_rate;
};

/*
 * i2c_reset: - reset the host controller
 *
 */
static void i2c_reset(void __iomem *base)
{
	u32 icr_mode;
	u32 val;

	/* Save bus mode (standard or fast speed) for later use */
	icr_mode = readl(base + ICR_OFFSET) & ICR_MODE_MASK;
	/* disable unit */
	val = readl(base + ICR_OFFSET);
	writel(val & ~ICR_IUE, base + ICR_OFFSET);
	udelay(10);
	/* reset the unit */
	val = readl(base + ICR_OFFSET);
	val |= ICR_UR;
	writel(val, base + ICR_OFFSET);
	udelay(100);
	/* disable unit */
	val = readl(base + ICR_OFFSET);
	writel(val & ~ICR_IUE, base + ICR_OFFSET);

	/* set slave address */
	writel(0x00, base + ISR_OFFSET);
	/* set control reg values */
	writel(I2C_ICR_INIT | icr_mode, base + ICR_OFFSET);
	writel(I2C_ISR_INIT, base + ISR_OFFSET); /* set clear interrupt bits */
	val = readl(base + ICR_OFFSET);
	val |= ICR_IUE;
	writel(val, base + ICR_OFFSET); /* enable unit */
	udelay(100);
}

static inline bool is_isr_set_or_clr(unsigned long isr, unsigned long set_mask,
				     unsigned long clr_mask)
{
	return ((isr & set_mask) == set_mask) && ((isr & clr_mask) == 0);
}

/*
 * i2c_isr_set_cleared: - wait until certain bits of the I2C status register
 *	                  are set and cleared
 *
 * @return: 0 on success or -ETIMEDOUT.
 */
static int i2c_isr_set_cleared(void __iomem *base, unsigned long set_mask,
			       unsigned long clr_mask)
{
	int cnt = 1000, delay = 10, isr, ret;

	ret = read_poll_timeout(readl, isr,
				is_isr_set_or_clr(isr, set_mask, clr_mask),
				delay, delay * cnt, base + ISR_OFFSET);
	return ret;
}

/*
 * i2c_transfer: - Transfer one byte over the i2c bus
 *
 * This function can transfer a byte over the i2c bus in both directions.
 * It is used by the public API functions.
 *
 * @return:  0: transfer successful or error code
 */
static int i2c_transfer(void __iomem *base, struct k1_i2c_msg *msg)
{
	int ret;
	u32 val;

	if (!msg)
		goto transfer_error_msg_empty;

	switch (msg->direction) {
	case I2C_WRITE:
		/* check if bus is not busy */
		if (i2c_isr_set_cleared(base, 0, ISR_IBB))
			goto transfer_error_bus_busy;

		/* start transmission */
		val = readl(base + ICR_OFFSET);
		val &= ~ICR_START;
		writel(val, base + ICR_OFFSET);
		val = readl(base + ICR_OFFSET);
		val &= ~ICR_STOP;
		writel(val, base + ICR_OFFSET);
		writel(msg->data, base + IDBR_OFFSET);
		if (msg->condition == I2C_COND_START) {
			val = readl(base + ICR_OFFSET);
			val |= ICR_START;
			writel(val, base + ICR_OFFSET);
		}
		if (msg->condition == I2C_COND_STOP) {
			val = readl(base + ICR_OFFSET);
			val |= ICR_STOP;
			writel(val, base + ICR_OFFSET);
		}
		if (msg->acknack == I2C_ACKNAK_SENDNAK) {
			val = readl(base + ICR_OFFSET);
			val |= ICR_ACKNAK;
			writel(val, base + ICR_OFFSET);
		}
		if (msg->acknack == I2C_ACKNAK_SENDACK) {
			val = readl(base + ICR_OFFSET);
			val &= ~ICR_ACKNAK;
			writel(val, base + ICR_OFFSET);
		}
		val = readl(base + ICR_OFFSET);
		val &= ~ICR_ALDIE;
		writel(val, base + ICR_OFFSET);
		val = readl(base + ICR_OFFSET);
		val |= ICR_TB;
		writel(val, base + ICR_OFFSET);

		/* transmit register empty? */
		if (i2c_isr_set_cleared(base, ISR_ITE, 0))
			goto transfer_error_transmit_timeout;

		/* clear 'transmit empty' state */
		val = readl(base + ISR_OFFSET);
		val |= ISR_ITE;
		writel(val, base + ISR_OFFSET);

		/* wait for ACK from slave */
		if (msg->acknack == I2C_ACKNAK_WAITACK)
			if (i2c_isr_set_cleared(base, 0, ISR_ACKNAK))
				goto transfer_error_ack_missing;
		break;

	case I2C_READ:
		/* check if bus is not busy */
		if (i2c_isr_set_cleared(base, 0, ISR_IBB))
			goto transfer_error_bus_busy;

		/* start receive */
		val = readl(base + ICR_OFFSET);
		val &= ~ICR_START;
		writel(val, base + ICR_OFFSET);
		val = readl(base + ICR_OFFSET);
		val &= ~ICR_STOP;
		writel(val, base + ICR_OFFSET);
		if (msg->condition == I2C_COND_START) {
			val = readl(base + ICR_OFFSET);
			val |= ICR_START;
			writel(val, base + ICR_OFFSET);
		}
		if (msg->condition == I2C_COND_STOP) {
			val = readl(base + ICR_OFFSET);
			val |= ICR_STOP;
			writel(val, base + ICR_OFFSET);
		}
		if (msg->acknack == I2C_ACKNAK_SENDNAK) {
			val = readl(base + ICR_OFFSET);
			val |= ICR_ACKNAK;
			writel(val, base + ICR_OFFSET);
		}
		if (msg->acknack == I2C_ACKNAK_SENDACK) {
			val = readl(base + ICR_OFFSET);
			val &= ~ICR_ACKNAK;
			writel(val, base + ICR_OFFSET);
		}
		val = readl(base + ICR_OFFSET);
		val &= ~ICR_ALDIE;
		writel(val, base + ICR_OFFSET);
		val = readl(base + ICR_OFFSET);
		val |= ICR_TB;
		writel(val, base + ICR_OFFSET);

		/* receive register full? */
		if (i2c_isr_set_cleared(base, ISR_IRF, 0))
			goto transfer_error_receive_timeout;

		msg->data = readl(base + IDBR_OFFSET);

		/* clear 'receive empty' state */
		val = readl(base + ISR_OFFSET);
		val |= ISR_IRF;
		writel(val, base + ISR_OFFSET);
		break;
	default:
		goto transfer_error_illegal_param;
	}

	return 0;

transfer_error_msg_empty:
	debug("%s: error: 'msg' is empty\n", __func__);
	ret = -EINVAL;
	goto i2c_transfer_finish;

transfer_error_transmit_timeout:
	debug("%s: error: transmit timeout\n", __func__);
	ret = -ETIMEDOUT;
	goto i2c_transfer_finish;

transfer_error_ack_missing:
	debug("%s: error: ACK missing\n", __func__);
	ret = -EREMOTEIO;
	goto i2c_transfer_finish;

transfer_error_receive_timeout:
	debug("%s: error: receive timeout\n", __func__);
	ret = -ETIMEDOUT;
	goto i2c_transfer_finish;

transfer_error_illegal_param:
	debug("%s: error: illegal parameters\n", __func__);
	ret = -EINVAL;
	goto i2c_transfer_finish;

transfer_error_bus_busy:
	debug("%s: error: bus is busy\n", __func__);
	ret = -EIO;
	goto i2c_transfer_finish;

i2c_transfer_finish:
	debug("%s: ISR: 0x%04x\n", __func__, readl(base + ISR_OFFSET));
	i2c_reset(base);
	return ret;
}

static int __i2c_read(void __iomem *base, uchar chip, u8 *addr, int alen,
		      uchar *buffer, int len)
{
	struct k1_i2c_msg msg;
	int ret;

	debug("%s(chip=0x%02x, addr=0x%02x, alen=0x%02x, len=0x%02x)\n",
	      __func__, chip, *addr, alen, len);

	if (len == 0) {
		pr_err("reading zero byte is invalid\n");
		return -EINVAL;
	}

	i2c_reset(base);

	/* dummy chip address write */
	debug("%s: dummy chip address write\n", __func__);
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data = (chip << 1);
	msg.data &= 0xFE;
	ret = i2c_transfer(base, &msg);
	if (ret)
		return ret;

	/*
	 * send memory address bytes;
	 * alen defines how much bytes we have to send.
	 */
	while (--alen >= 0) {
		debug("%s: send address byte %02x (alen=%d)\n",
		      __func__, *addr, alen);
		msg.condition = I2C_COND_NORMAL;
		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = addr[alen];
		ret = i2c_transfer(base, &msg);
		if (ret)
			return ret;
	}

	/* start read sequence */
	debug("%s: start read sequence\n", __func__);
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1);
	msg.data     |= 0x01;
	ret = i2c_transfer(base, &msg);
	if (ret)
		return ret;

	/* read bytes; send NACK at last byte */
	while (len--) {
		if (len == 0) {
			msg.condition = I2C_COND_STOP;
			msg.acknack   = I2C_ACKNAK_SENDNAK;
		} else {
			msg.condition = I2C_COND_NORMAL;
			msg.acknack   = I2C_ACKNAK_SENDACK;
		}

		msg.direction = I2C_READ;
		msg.data      = 0x00;
		ret = i2c_transfer(base, &msg);
		if (ret)
			return ret;

		*buffer = msg.data;
		debug("%s: reading byte (%p)=0x%02x\n",
		      __func__, buffer, *buffer);
		buffer++;
	}

	i2c_reset(base);

	return 0;
}

static int __i2c_write(struct k1_i2c *base, uchar chip, u8 *addr, int alen,
		       uchar *buffer, int len)
{
	struct k1_i2c_msg msg;
	int ret;

	debug("%s(chip=0x%02x, addr=0x%02x, alen=0x%02x, len=0x%02x)\n",
	      __func__, chip, *addr, alen, len);

	i2c_reset(base);

	/* chip address write */
	debug("%s: chip address write\n", __func__);
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data = (chip << 1);
	msg.data &= 0xFE;
	ret = i2c_transfer(base, &msg);
	if (ret)
		return ret;

	/*
	 * send memory address bytes;
	 * alen defines how much bytes we have to send.
	 */
	while (--alen >= 0) {
		debug("%s: send address byte %02x (alen=%d)\n",
		      __func__, *addr, alen);
		msg.condition = I2C_COND_NORMAL;
		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = addr[alen];
		ret = i2c_transfer(base, &msg);
		if (ret)
			return ret;
	}

	/* write bytes; send NACK at last byte */
	while (len--) {
		debug("%s: writing byte (%p)=0x%02x\n",
		      __func__, buffer, *buffer);

		if (len == 0)
			msg.condition = I2C_COND_STOP;
		else
			msg.condition = I2C_COND_NORMAL;

		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = *(buffer++);

		ret = i2c_transfer(base, &msg);
		if (ret)
			return ret;
	}

	i2c_reset(base);

	return 0;
}

static int k1_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct k1_i2c_priv *i2c = dev_get_priv(bus);
	struct i2c_msg *dmsg, *omsg, dummy;

	memset(&dummy, 0, sizeof(struct i2c_msg));

	/*
	 * We expect either two messages (one with an offset and one with the
	 * actual data) or one message (just data or offset/data combined)
	 */
	if (nmsgs > 2 || nmsgs == 0) {
		debug("%s: Only one or two messages are supported.", __func__);
		return -EINVAL;
	}

	omsg = nmsgs == 1 ? &dummy : msg;
	dmsg = nmsgs == 1 ? msg : msg + 1;

	if (dmsg->flags & I2C_M_RD)
		return __i2c_read(i2c->base, dmsg->addr, omsg->buf,
				  omsg->len, dmsg->buf, dmsg->len);
	else
		return __i2c_write(i2c->base, dmsg->addr, omsg->buf,
				   omsg->len, dmsg->buf, dmsg->len);
}

static int k1_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct k1_i2c_priv *priv = dev_get_priv(bus);
	void __iomem *base = priv->base;
	u32 val;

	if (speed > I2C_SPEED_STANDARD_RATE)
		val = ICR_FM;
	else
		val = ICR_SM;
	clrsetbits_le32(base + ICR_OFFSET, ICR_MODE_MASK, val);

	return 0;
}

static int k1_i2c_probe(struct udevice *bus)
{
	struct k1_i2c_priv *priv = dev_get_priv(bus);
	struct reset_ctl reset;
	int ret;

	priv->id = dev_seq(bus);
	ret = reset_get_by_index(bus, 0, &reset);
	if (ret) {
		dev_err(bus, "%s: can not get reset\n", __func__);
		return ret;
	}
	reset_assert(&reset);
	udelay(10);
	reset_deassert(&reset);
	udelay(10);

	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret && ret != -ENOSYS && ret != -EOPNOTSUPP) {
		debug("%s: failed to enable clock\n", __func__);
		return ret;
	}
	priv->clk_rate = clk_get_rate(&priv->clk);

	priv->base = (void *)devfdt_get_addr_ptr(bus);
	k1_i2c_set_bus_speed(bus, priv->clk_rate);
	return 0;
}

static const struct dm_i2c_ops k1_i2c_ops = {
	.xfer		= k1_i2c_xfer,
	.set_bus_speed	= k1_i2c_set_bus_speed,
};

static const struct udevice_id k1_i2c_ids[] = {
	{ .compatible = "spacemit,k1-i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_spacemit) = {
	.name	= "i2c_spacemit",
	.id	= UCLASS_I2C,
	.of_match = k1_i2c_ids,
	.probe	= k1_i2c_probe,
	.priv_auto = sizeof(struct k1_i2c_priv),
	.ops	= &k1_i2c_ops,
};
