/*
 * Copyright (C) 2015 Moritz Fischer <moritz.fischer@ettus.com>
 * IP from Cadence (ID T-CS-PE-0007-100, Version R1p10f2)
 *
 * This file is based on: drivers/i2c/zynq_i2c.c,
 * with added driver-model support and code cleanup.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/types.h>
#include <linux/io.h>
#include <asm/errno.h>
#include <dm/device.h>
#include <dm/root.h>
#include <i2c.h>
#include <fdtdec.h>
#include <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

/* i2c register set */
struct cdns_i2c_regs {
	u32 control;
	u32 status;
	u32 address;
	u32 data;
	u32 interrupt_status;
	u32 transfer_size;
	u32 slave_mon_pause;
	u32 time_out;
	u32 interrupt_mask;
	u32 interrupt_enable;
	u32 interrupt_disable;
};

/* Control register fields */
#define CDNS_I2C_CONTROL_RW		0x00000001
#define CDNS_I2C_CONTROL_MS		0x00000002
#define CDNS_I2C_CONTROL_NEA		0x00000004
#define CDNS_I2C_CONTROL_ACKEN		0x00000008
#define CDNS_I2C_CONTROL_HOLD		0x00000010
#define CDNS_I2C_CONTROL_SLVMON		0x00000020
#define CDNS_I2C_CONTROL_CLR_FIFO	0x00000040
#define CDNS_I2C_CONTROL_DIV_B_SHIFT	8
#define CDNS_I2C_CONTROL_DIV_B_MASK	0x00003F00
#define CDNS_I2C_CONTROL_DIV_A_SHIFT	14
#define CDNS_I2C_CONTROL_DIV_A_MASK	0x0000C000

/* Status register values */
#define CDNS_I2C_STATUS_RXDV	0x00000020
#define CDNS_I2C_STATUS_TXDV	0x00000040
#define CDNS_I2C_STATUS_RXOVF	0x00000080
#define CDNS_I2C_STATUS_BA	0x00000100

/* Interrupt register fields */
#define CDNS_I2C_INTERRUPT_COMP		0x00000001
#define CDNS_I2C_INTERRUPT_DATA		0x00000002
#define CDNS_I2C_INTERRUPT_NACK		0x00000004
#define CDNS_I2C_INTERRUPT_TO		0x00000008
#define CDNS_I2C_INTERRUPT_SLVRDY	0x00000010
#define CDNS_I2C_INTERRUPT_RXOVF	0x00000020
#define CDNS_I2C_INTERRUPT_TXOVF	0x00000040
#define CDNS_I2C_INTERRUPT_RXUNF	0x00000080
#define CDNS_I2C_INTERRUPT_ARBLOST	0x00000200

#define CDNS_I2C_FIFO_DEPTH		16
#define CDNS_I2C_TRANSFER_SIZE_MAX	255 /* Controller transfer limit */

#ifdef DEBUG
static void cdns_i2c_debug_status(struct cdns_i2c_regs *cdns_i2c)
{
	int int_status;
	int status;
	int_status = readl(&cdns_i2c->interrupt_status);

	status = readl(&cdns_i2c->status);
	if (int_status || status) {
		debug("Status: ");
		if (int_status & CDNS_I2C_INTERRUPT_COMP)
			debug("COMP ");
		if (int_status & CDNS_I2C_INTERRUPT_DATA)
			debug("DATA ");
		if (int_status & CDNS_I2C_INTERRUPT_NACK)
			debug("NACK ");
		if (int_status & CDNS_I2C_INTERRUPT_TO)
			debug("TO ");
		if (int_status & CDNS_I2C_INTERRUPT_SLVRDY)
			debug("SLVRDY ");
		if (int_status & CDNS_I2C_INTERRUPT_RXOVF)
			debug("RXOVF ");
		if (int_status & CDNS_I2C_INTERRUPT_TXOVF)
			debug("TXOVF ");
		if (int_status & CDNS_I2C_INTERRUPT_RXUNF)
			debug("RXUNF ");
		if (int_status & CDNS_I2C_INTERRUPT_ARBLOST)
			debug("ARBLOST ");
		if (status & CDNS_I2C_STATUS_RXDV)
			debug("RXDV ");
		if (status & CDNS_I2C_STATUS_TXDV)
			debug("TXDV ");
		if (status & CDNS_I2C_STATUS_RXOVF)
			debug("RXOVF ");
		if (status & CDNS_I2C_STATUS_BA)
			debug("BA ");
		debug("TS%d ", readl(&cdns_i2c->transfer_size));
		debug("\n");
	}
}
#endif

struct i2c_cdns_bus {
	int id;
	struct cdns_i2c_regs __iomem *regs;	/* register base */
};


/** cdns_i2c_probe() - Probe method
 * @dev: udevice pointer
 *
 * DM callback called when device is probed
 */
static int cdns_i2c_probe(struct udevice *dev)
{
	struct i2c_cdns_bus *bus = dev_get_priv(dev);

	bus->regs = (struct cdns_i2c_regs *)dev_get_addr(dev);
	if (!bus->regs)
		return -ENOMEM;

	/* TODO: Calculate dividers based on CPU_CLK_1X */
	/* 111MHz / ( (3 * 17) * 22 ) = ~100KHz */
	writel((16 << CDNS_I2C_CONTROL_DIV_B_SHIFT) |
		(2 << CDNS_I2C_CONTROL_DIV_A_SHIFT), &bus->regs->control);

	/* Enable master mode, ack, and 7-bit addressing */
	setbits_le32(&bus->regs->control, CDNS_I2C_CONTROL_MS |
		CDNS_I2C_CONTROL_ACKEN | CDNS_I2C_CONTROL_NEA);

	debug("%s bus %d at %p\n", __func__, dev->seq, bus->regs);

	return 0;
}

static int cdns_i2c_remove(struct udevice *dev)
{
	struct i2c_cdns_bus *bus = dev_get_priv(dev);

	debug("%s bus %d at %p\n", __func__, dev->seq, bus->regs);

	unmap_sysmem(bus->regs);

	return 0;
}

/* Wait for an interrupt */
static u32 cdns_i2c_wait(struct cdns_i2c_regs *cdns_i2c, u32 mask)
{
	int timeout, int_status;

	for (timeout = 0; timeout < 100; timeout++) {
		udelay(100);
		int_status = readl(&cdns_i2c->interrupt_status);
		if (int_status & mask)
			break;
	}

	/* Clear interrupt status flags */
	writel(int_status & mask, &cdns_i2c->interrupt_status);

	return int_status & mask;
}

static int cdns_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	if (speed != 100000) {
		printf("%s, failed to set clock speed to %u\n", __func__,
		       speed);
		return -EINVAL;
	}

	return 0;
}

/* Probe to see if a chip is present. */
static int cdns_i2c_probe_chip(struct udevice *bus, uint chip_addr,
				uint chip_flags)
{
	struct i2c_cdns_bus *i2c_bus = dev_get_priv(bus);
	struct cdns_i2c_regs *regs = i2c_bus->regs;

	/* Attempt to read a byte */
	setbits_le32(&regs->control, CDNS_I2C_CONTROL_CLR_FIFO |
		CDNS_I2C_CONTROL_RW);
	clrbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);
	writel(0xFF, &regs->interrupt_status);
	writel(chip_addr, &regs->address);
	writel(1, &regs->transfer_size);

	return (cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP |
		CDNS_I2C_INTERRUPT_NACK) &
		CDNS_I2C_INTERRUPT_COMP) ? 0 : -ETIMEDOUT;
}

static int cdns_i2c_write_data(struct i2c_cdns_bus *i2c_bus, u32 addr, u8 *data,
			       u32 len, bool next_is_read)
{
	u8 *cur_data = data;

	struct cdns_i2c_regs *regs = i2c_bus->regs;

	setbits_le32(&regs->control, CDNS_I2C_CONTROL_CLR_FIFO |
		CDNS_I2C_CONTROL_HOLD);

	/* if next is a read, we need to clear HOLD, doesn't work */
	if (next_is_read)
		clrbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);

	clrbits_le32(&regs->control, CDNS_I2C_CONTROL_RW);

	writel(0xFF, &regs->interrupt_status);
	writel(addr, &regs->address);

	while (len--) {
		writel(*(cur_data++), &regs->data);
		if (readl(&regs->transfer_size) == CDNS_I2C_FIFO_DEPTH) {
			if (!cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP)) {
				/* Release the bus */
				clrbits_le32(&regs->control,
					     CDNS_I2C_CONTROL_HOLD);
				return -ETIMEDOUT;
			}
		}
	}

	/* All done... release the bus */
	clrbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);
	/* Wait for the address and data to be sent */
	if (!cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP))
		return -ETIMEDOUT;
	return 0;
}

static int cdns_i2c_read_data(struct i2c_cdns_bus *i2c_bus, u32 addr, u8 *data,
			      u32 len)
{
	u32 status;
	u32 i = 0;
	u8 *cur_data = data;

	/* TODO: Fix this */
	struct cdns_i2c_regs *regs = i2c_bus->regs;

	/* Check the hardware can handle the requested bytes */
	if ((len < 0) || (len > CDNS_I2C_TRANSFER_SIZE_MAX))
		return -EINVAL;

	setbits_le32(&regs->control, CDNS_I2C_CONTROL_CLR_FIFO |
		CDNS_I2C_CONTROL_RW);

	/* Start reading data */
	writel(addr, &regs->address);
	writel(len, &regs->transfer_size);

	/* Wait for data */
	do {
		status = cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP |
			CDNS_I2C_INTERRUPT_DATA);
		if (!status) {
			/* Release the bus */
			clrbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);
			return -ETIMEDOUT;
		}
		debug("Read %d bytes\n",
		      len - readl(&regs->transfer_size));
		for (; i < len - readl(&regs->transfer_size); i++)
			*(cur_data++) = readl(&regs->data);
	} while (readl(&regs->transfer_size) != 0);
	/* All done... release the bus */
	clrbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);

#ifdef DEBUG
	cdns_i2c_debug_status(regs);
#endif
	return 0;
}

static int cdns_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			 int nmsgs)
{
	struct i2c_cdns_bus *i2c_bus = dev_get_priv(dev);
	int ret;

	debug("i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		bool next_is_read = nmsgs > 1 && (msg[1].flags & I2C_M_RD);

		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			ret = cdns_i2c_read_data(i2c_bus, msg->addr, msg->buf,
						 msg->len);
		} else {
			ret = cdns_i2c_write_data(i2c_bus, msg->addr, msg->buf,
						  msg->len, next_is_read);
		}
		if (ret) {
			debug("i2c_write: error sending\n");
			return -EREMOTEIO;
		}
	}

	return 0;
}

static const struct dm_i2c_ops cdns_i2c_ops = {
	.xfer = cdns_i2c_xfer,
	.probe_chip = cdns_i2c_probe_chip,
	.set_bus_speed = cdns_i2c_set_bus_speed,
};

static const struct udevice_id cdns_i2c_of_match[] = {
	{ .compatible = "cdns,i2c-r1p10" },
	{ /* end of table */ }
};

U_BOOT_DRIVER(cdns_i2c) = {
	.name = "i2c-cdns",
	.id = UCLASS_I2C,
	.of_match = cdns_i2c_of_match,
	.probe = cdns_i2c_probe,
	.remove = cdns_i2c_remove,
	.priv_auto_alloc_size = sizeof(struct i2c_cdns_bus),
	.ops = &cdns_i2c_ops,
};
