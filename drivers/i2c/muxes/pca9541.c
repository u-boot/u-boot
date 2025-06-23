// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2008-2009 Rodolfo Giometti <giometti@linux.it>
 * Copyright (c) 2008-2009 Eurotech S.p.A. <info@eurotech.it>
 * Copyright (c) 2010 Ericsson AB.
 * Copyright (c) 2025 Advanced Micro Devices, Inc.
 */

#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <linux/delay.h>

/*
 * The PCA9541 is a bus master selector. It supports two I2C masters connected
 * to a single slave bus.
 *
 * Before each bus transaction, a master has to acquire bus ownership. After the
 * transaction is complete, bus ownership has to be released. This fits well
 * into the I2C multiplexer framework, which provides select and release
 * functions for this purpose. For this reason, this driver is modeled as
 * single-channel I2C bus multiplexer.
 *
 * This driver assumes that the two bus masters are controlled by two different
 * hosts. If a single host controls both masters, platform code has to ensure
 * that only one of the masters is instantiated at any given time.
 */

#define PCA9541_CONTROL		0x01
#define PCA9541_ISTAT		0x02

#define PCA9541_CTL_MYBUS	BIT(0)
#define PCA9541_CTL_NMYBUS	BIT(1)
#define PCA9541_CTL_BUSON	BIT(2)
#define PCA9541_CTL_NBUSON	BIT(3)
#define PCA9541_CTL_BUSINIT	BIT(4)
#define PCA9541_CTL_TESTON	BIT(6)
#define PCA9541_CTL_NTESTON	BIT(7)

#define PCA9541_ISTAT_INTIN	BIT(0)
#define PCA9541_ISTAT_BUSINIT	BIT(1)
#define PCA9541_ISTAT_BUSOK	BIT(2)
#define PCA9541_ISTAT_BUSLOST	BIT(3)
#define PCA9541_ISTAT_MYTEST	BIT(6)
#define PCA9541_ISTAT_NMYTEST	BIT(7)

#define BUSON			(PCA9541_CTL_BUSON | PCA9541_CTL_NBUSON)
#define MYBUS			(PCA9541_CTL_MYBUS | PCA9541_CTL_NMYBUS)

/* arbitration timeouts, in jiffies */
#define ARB_TIMEOUT_US		125000	/* 125 ms until forcing bus ownership */
#define ARB2_TIMEOUT_US		250000	/* 250 ms until acquisition failure */

/* arbitration retry delays, in us */
#define SELECT_DELAY_SHORT	50
#define SELECT_DELAY_LONG	1000

struct pca9541_plat {
	u32 addr;
};

struct pca9541_priv {
	u32 addr;
	unsigned long select_timeout;
	long arb_timeout;
};

static inline int mybus(int x)
{
	return !(x & MYBUS) || ((x & MYBUS) == MYBUS);
}

static inline int busoff(int x)
{
	return !(x & BUSON) || ((x & BUSON) == BUSON);
}

static int pca9541_reg_write(struct udevice *mux, struct pca9541_priv *client,
			     u8 command, u8 val)
{
	return dm_i2c_write(mux, command, &val, 1);
}

static int pca9541_reg_read(struct udevice *mux, struct pca9541_priv *client,
			    u8 command)
{
	int ret;
	uchar byte;

	ret = dm_i2c_read(mux, command, &byte, 1);

	return ret ?: byte;
}

/*
 * Arbitration management functions
 */

/* Release bus. Also reset NTESTON and BUSINIT if it was set. */
static void pca9541_release_bus(struct udevice *mux, struct pca9541_priv *client)
{
	int reg;

	reg = pca9541_reg_read(mux, client, PCA9541_CONTROL);
	if (reg >= 0 && !busoff(reg) && mybus(reg))
		pca9541_reg_write(mux, client, PCA9541_CONTROL,
				  (reg & PCA9541_CTL_NBUSON) >> 1);
}

/*
 * Arbitration is defined as a two-step process. A bus master can only activate
 * the slave bus if it owns it; otherwise it has to request ownership first.
 * This multi-step process ensures that access contention is resolved
 * gracefully.
 *
 * Bus	Ownership	Other master	Action
 * state		requested access
 * ----------------------------------------------------
 * off	-		yes		wait for arbitration timeout or
 *					for other master to drop request
 * off	no		no		take ownership
 * off	yes		no		turn on bus
 * on	yes		-		done
 * on	no		-		wait for arbitration timeout or
 *					for other master to release bus
 *
 * The main contention point occurs if the slave bus is off and both masters
 * request ownership at the same time. In this case, one master will turn on
 * the slave bus, believing that it owns it. The other master will request
 * bus ownership. Result is that the bus is turned on, and master which did
 * _not_ own the slave bus before ends up owning it.
 */

/* Control commands per PCA9541 datasheet */
static const u8 pca9541_control[16] = {
	4, 0, 1, 5, 4, 4, 5, 5, 0, 0, 1, 1, 0, 4, 5, 1
};

/*
 * Channel arbitration
 *
 * Return values:
 *  <0: error
 *  0 : bus not acquired
 *  1 : bus acquired
 */
static int pca9541_arbitrate(struct udevice *mux, struct pca9541_priv *client)
{
	int reg, ret = 0;

	reg = pca9541_reg_read(mux, client, PCA9541_CONTROL);
	if (reg < 0)
		return reg;

	if (busoff(reg)) {
		int istat;

		/*
		 * Bus is off. Request ownership or turn it on unless
		 * other master requested ownership.
		 */
		istat = pca9541_reg_read(mux, client, PCA9541_ISTAT);
		if (!(istat & PCA9541_ISTAT_NMYTEST) ||
		    client->arb_timeout <= 0) {
			/*
			 * Other master did not request ownership,
			 * or arbitration timeout expired. Take the bus.
			 */
			pca9541_reg_write(mux, client, PCA9541_CONTROL,
					  pca9541_control[reg & 0x0f]
					  | PCA9541_CTL_NTESTON);
			client->select_timeout = SELECT_DELAY_SHORT;
		} else {
			/*
			 * Other master requested ownership.
			 * Set extra long timeout to give it time to acquire it.
			 */
			client->select_timeout = SELECT_DELAY_LONG * 2;
		}
	} else if (mybus(reg)) {
		/*
		 * Bus is on, and we own it. We are done with acquisition.
		 * Reset NTESTON and BUSINIT, then return success.
		 */
		if (reg & (PCA9541_CTL_NTESTON | PCA9541_CTL_BUSINIT))
			pca9541_reg_write(mux, client, PCA9541_CONTROL,
					  reg & ~(PCA9541_CTL_NTESTON
					  | PCA9541_CTL_BUSINIT));
		ret = 1;
	} else {
		/*
		 * Other master owns the bus.
		 * If arbitration timeout has expired, force ownership.
		 * Otherwise request it.
		 */
		client->select_timeout = SELECT_DELAY_LONG;
		if (client->arb_timeout <= 0) {
			/* Time is up, take the bus and reset it. */
			pca9541_reg_write(mux, client, PCA9541_CONTROL,
					  pca9541_control[reg & 0x0f]
					  | PCA9541_CTL_BUSINIT
					  | PCA9541_CTL_NTESTON);
		} else {
			/* Request bus ownership if needed */
			if (!(reg & PCA9541_CTL_NTESTON))
				pca9541_reg_write(mux, client, PCA9541_CONTROL,
						  reg | PCA9541_CTL_NTESTON);
		}
	}

	return ret;
}

static int pca9541_select_chan(struct udevice *mux, struct udevice *bus,
			       uint channel)
{
	struct pca9541_priv *priv = dev_get_priv(mux);
	int ret;
	long timeout = ARB2_TIMEOUT_US; /* Give up after this time */

	/* Force bus ownership after this time */
	priv->arb_timeout = ARB_TIMEOUT_US;
	do {
		ret = pca9541_arbitrate(mux, priv);
		if (ret)
			return ret < 0 ? ret : 0;

		udelay(priv->select_timeout);
		timeout -= priv->select_timeout;
		priv->arb_timeout -= priv->select_timeout;
	} while (timeout > 0);

	debug("I2C Arbitration select timeout\n");

	return -ETIMEDOUT;
}

static int pca9541_release_chan(struct udevice *mux, struct udevice *bus,
				uint channel)
{
	struct pca9541_priv *priv = dev_get_priv(mux);

	pca9541_release_bus(mux, priv);

	return 0;
}

/*
 * I2C init/probing/exit functions
 */
static int pca9541_of_to_plat(struct udevice *dev)
{
	struct pca9541_plat *plat = dev_get_plat(dev);

	plat->addr = dev_read_u32_default(dev, "reg", 0);
	if (!plat->addr) {
		debug("Reg property is not found\n");
		return -ENODEV;
	}

	debug("Device %s at 0x%x\n", dev->name, plat->addr);

	return 0;
}

static int pca9541_probe(struct udevice *dev)
{
	struct pca9541_plat *plat = dev_get_plat(dev);
	struct pca9541_priv *priv = dev_get_priv(dev);

	priv->addr = plat->addr;

	return 0;
}

static const struct i2c_mux_ops pca9541_ops = {
	.select = pca9541_select_chan,
	.deselect = pca9541_release_chan,
};

static const struct udevice_id pca9541_ids[] = {
	{ .compatible = "nxp,pca9541", },
	{ }
};

U_BOOT_DRIVER(pca9541) = {
	.name = "pca9541",
	.id = UCLASS_I2C_MUX,
	.of_match = pca9541_ids,
	.probe = pca9541_probe,
	.ops = &pca9541_ops,
	.of_to_plat = pca9541_of_to_plat,
	.plat_auto = sizeof(struct pca9541_plat),
	.priv_auto = sizeof(struct pca9541_priv),
};
