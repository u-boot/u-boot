/*
 * drivers/i2c/rcar_i2c.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct rcar_i2c {
	u32 icscr;
	u32 icmcr;
	u32 icssr;
	u32 icmsr;
	u32 icsier;
	u32 icmier;
	u32 icccr;
	u32 icsar;
	u32 icmar;
	u32 icrxdtxd;
	u32 icccr2;
	u32 icmpr;
	u32 ichpr;
	u32 iclpr;
};

#define MCR_MDBS	0x80	/* non-fifo mode switch	*/
#define MCR_FSCL	0x40	/* override SCL pin	*/
#define MCR_FSDA	0x20	/* override SDA pin	*/
#define MCR_OBPC	0x10	/* override pins	*/
#define MCR_MIE		0x08	/* master if enable	*/
#define MCR_TSBE	0x04
#define MCR_FSB		0x02	/* force stop bit	*/
#define MCR_ESG		0x01	/* en startbit gen.	*/

#define MSR_MASK	0x7f
#define MSR_MNR		0x40	/* nack received	*/
#define MSR_MAL		0x20	/* arbitration lost	*/
#define MSR_MST		0x10	/* sent a stop		*/
#define MSR_MDE		0x08
#define MSR_MDT		0x04
#define MSR_MDR		0x02
#define MSR_MAT		0x01	/* slave addr xfer done	*/

static const struct rcar_i2c *i2c_dev[CONFIF_SYS_RCAR_I2C_NUM_CONTROLLERS] = {
	(struct rcar_i2c *)CONFIG_SYS_RCAR_I2C0_BASE,
	(struct rcar_i2c *)CONFIG_SYS_RCAR_I2C1_BASE,
	(struct rcar_i2c *)CONFIG_SYS_RCAR_I2C2_BASE,
	(struct rcar_i2c *)CONFIG_SYS_RCAR_I2C3_BASE,
};

static void rcar_i2c_raw_rw_common(struct rcar_i2c *dev, u8 chip, uint addr)
{
	/* set slave address */
	writel(chip << 1, &dev->icmar);
	/* set register address */
	writel(addr, &dev->icrxdtxd);
	/* clear status */
	writel(0, &dev->icmsr);
	/* start master send */
	writel(MCR_MDBS | MCR_MIE | MCR_ESG, &dev->icmcr);

	while ((readl(&dev->icmsr) & (MSR_MAT | MSR_MDE))
		!= (MSR_MAT | MSR_MDE))
		udelay(10);

	/* clear ESG */
	writel(MCR_MDBS | MCR_MIE, &dev->icmcr);
	/* start SCLclk */
	writel(~(MSR_MAT | MSR_MDE), &dev->icmsr);

	while (!(readl(&dev->icmsr) & MSR_MDE))
		udelay(10);
}

static void rcar_i2c_raw_rw_finish(struct rcar_i2c *dev)
{
	while (!(readl(&dev->icmsr) & MSR_MST))
		udelay(10);

	writel(0, &dev->icmcr);
}

static int
rcar_i2c_raw_write(struct rcar_i2c *dev, u8 chip, uint addr, u8 *val, int size)
{
	rcar_i2c_raw_rw_common(dev, chip, addr);

	/* set send date */
	writel(*val, &dev->icrxdtxd);
	/* start SCLclk */
	writel(~MSR_MDE, &dev->icmsr);

	while (!(readl(&dev->icmsr) & MSR_MDE))
		udelay(10);

	/* set stop condition */
	writel(MCR_MDBS | MCR_MIE | MCR_FSB, &dev->icmcr);
	/* start SCLclk */
	writel(~MSR_MDE, &dev->icmsr);

	rcar_i2c_raw_rw_finish(dev);

	return 0;
}

static u8
rcar_i2c_raw_read(struct rcar_i2c *dev, u8 chip, uint addr)
{
	u8 ret;

	rcar_i2c_raw_rw_common(dev, chip, addr);

	/* set slave address, receive */
	writel((chip << 1) | 1, &dev->icmar);
	/* start master receive */
	writel(MCR_MDBS | MCR_MIE | MCR_ESG, &dev->icmcr);
	/* clear status */
	writel(0, &dev->icmsr);

	while ((readl(&dev->icmsr) & (MSR_MAT | MSR_MDR))
		!= (MSR_MAT | MSR_MDR))
		udelay(10);

	/* clear ESG */
	writel(MCR_MDBS | MCR_MIE, &dev->icmcr);
	/* prepare stop condition */
	writel(MCR_MDBS | MCR_MIE | MCR_FSB, &dev->icmcr);
	/* start SCLclk */
	writel(~(MSR_MAT | MSR_MDR), &dev->icmsr);

	while (!(readl(&dev->icmsr) & MSR_MDR))
		udelay(10);

	/* get receive data */
	ret = (u8)readl(&dev->icrxdtxd);
	/* start SCLclk */
	writel(~MSR_MDR, &dev->icmsr);

	rcar_i2c_raw_rw_finish(dev);

	return ret;
}

/*
 * SCL  = iicck / (20 + SCGD * 8 + F[(ticf + tr + intd) * iicck])
 * iicck  : I2C internal clock < 20 MHz
 * ticf : I2C SCL falling time: 35 ns
 * tr   : I2C SCL rising time:  200 ns
 * intd : LSI internal delay:   I2C0: 50 ns I2C1-3: 5
 * F[n] : n rounded up to an integer
 */
static u32 rcar_clock_gen(int i2c_no, u32 bus_speed)
{
	u32 iicck, f, scl, scgd;
	u32 intd = 5;

	int bit = 0, cdf_width = 3;
	for (bit = 0; bit < (1 << cdf_width); bit++) {
		iicck = CONFIG_HP_CLK_FREQ / (1 + bit);
		if (iicck < 20000000)
			break;
	}

	if (bit > (1 << cdf_width)) {
		puts("rcar-i2c: Can not get CDF\n");
		return 0;
	}

	if (i2c_no == 0)
		intd = 50;

	f = (35 + 200 + intd) * (iicck / 1000000000);

	for (scgd = 0; scgd < 0x40; scgd++) {
		scl = iicck / (20 + (scgd * 8) + f);
		if (scl <= bus_speed)
			break;
	}

	if (scgd > 0x40) {
		puts("rcar-i2c: Can not get SDGB\n");
		return 0;
	}

	debug("%s: scl: %d\n", __func__, scl);
	debug("%s: bit %x\n", __func__, bit);
	debug("%s: scgd %x\n", __func__, scgd);
	debug("%s: iccr %x\n", __func__, (scgd << (cdf_width) | bit));

	return scgd << (cdf_width) | bit;
}

static void
rcar_i2c_init(struct i2c_adapter *adap, int speed, int slaveadd)
{
	struct rcar_i2c *dev = (struct rcar_i2c *)i2c_dev[adap->hwadapnr];
	u32 icccr = 0;

	/* No i2c support prior to relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return;

	/*
	 * reset slave mode.
	 * slave mode is not used on this driver
	 */
	writel(0, &dev->icsier);
	writel(0, &dev->icsar);
	writel(0, &dev->icscr);
	writel(0, &dev->icssr);

	/* reset master mode */
	writel(0, &dev->icmier);
	writel(0, &dev->icmcr);
	writel(0, &dev->icmsr);
	writel(0, &dev->icmar);

	icccr = rcar_clock_gen(adap->hwadapnr, adap->speed);
	if (icccr == 0)
		puts("I2C: Init failed\n");
	else
		writel(icccr, &dev->icccr);
}

static int rcar_i2c_read(struct i2c_adapter *adap, uint8_t chip,
			uint addr, int alen, u8 *data, int len)
{
	struct rcar_i2c *dev = (struct rcar_i2c *)i2c_dev[adap->hwadapnr];
	int i;

	for (i = 0; i < len; i++)
		data[i] = rcar_i2c_raw_read(dev, chip, addr + i);

	return 0;
}

static int rcar_i2c_write(struct i2c_adapter *adap, uint8_t chip, uint addr,
			int alen, u8 *data, int len)
{
	struct rcar_i2c *dev = (struct rcar_i2c *)i2c_dev[adap->hwadapnr];
	return rcar_i2c_raw_write(dev, chip, addr, data, len);
}

static int
rcar_i2c_probe(struct i2c_adapter *adap, u8 dev)
{
	return rcar_i2c_read(adap, dev, 0, 0, NULL, 0);
}

static unsigned int rcar_i2c_set_bus_speed(struct i2c_adapter *adap,
			unsigned int speed)
{
	struct rcar_i2c *dev = (struct rcar_i2c *)i2c_dev[adap->hwadapnr];
	u32 icccr;
	int ret = 0;

	rcar_i2c_raw_rw_finish(dev);

	icccr = rcar_clock_gen(adap->hwadapnr, speed);
	if (icccr == 0) {
		puts("I2C: Init failed\n");
		ret = -1;
	} else {
		writel(icccr, &dev->icccr);
	}
	return ret;
}

/*
 * Register RCAR i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(rcar_0, rcar_i2c_init, rcar_i2c_probe, rcar_i2c_read,
			 rcar_i2c_write, rcar_i2c_set_bus_speed,
			 CONFIG_SYS_RCAR_I2C0_SPEED, 0, 0)
U_BOOT_I2C_ADAP_COMPLETE(rcar_1, rcar_i2c_init, rcar_i2c_probe, rcar_i2c_read,
			 rcar_i2c_write, rcar_i2c_set_bus_speed,
			 CONFIG_SYS_RCAR_I2C1_SPEED, 0, 1)
U_BOOT_I2C_ADAP_COMPLETE(rcar_2, rcar_i2c_init, rcar_i2c_probe, rcar_i2c_read,
			 rcar_i2c_write, rcar_i2c_set_bus_speed,
			 CONFIG_SYS_RCAR_I2C2_SPEED, 0, 2)
U_BOOT_I2C_ADAP_COMPLETE(rcar_3, rcar_i2c_init, rcar_i2c_probe, rcar_i2c_read,
			 rcar_i2c_write, rcar_i2c_set_bus_speed,
			 CONFIG_SYS_RCAR_I2C3_SPEED, 0, 3)
