// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for Renesas 9-series PCIe clock generator driver
 *
 * The following series can be supported:
 *   - 9FGV/9DBV/9DMV/9FGL/9DML/9QXL/9SQ
 * Currently supported:
 *   - 9FGV0241
 *   - 9FGV0441
 *   - 9FGV0841
 *
 * Copyright (C) 2022-2026 Marek Vasut
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <i2c.h>
#include <linux/bitops.h>

#define RS9_REG_OE				0x0
#define RS9_REG_SS				0x1
#define RS9_REG_SS_AMP_0V6			0x0
#define RS9_REG_SS_AMP_0V7			0x1
#define RS9_REG_SS_AMP_0V8			0x2
#define RS9_REG_SS_AMP_0V9			0x3
#define RS9_REG_SS_AMP_DEFAULT			RS9_REG_SS_AMP_0V8
#define RS9_REG_SS_AMP_MASK			0x3
#define RS9_REG_SS_SSC_100			0
#define RS9_REG_SS_SSC_M025			(1 << 3)
#define RS9_REG_SS_SSC_M050			(3 << 3)
#define RS9_REG_SS_SSC_DEFAULT			RS9_REG_SS_SSC_100
#define RS9_REG_SS_SSC_MASK			(3 << 3)
#define RS9_REG_SS_SSC_LOCK			BIT(5)
#define RS9_REG_SR				0x2
#define RS9_REG_REF				0x3
#define RS9_REG_REF_OE				BIT(4)
#define RS9_REG_REF_OD				BIT(5)
#define RS9_REG_REF_SR_SLOWEST			0
#define RS9_REG_REF_SR_SLOW			(1 << 6)
#define RS9_REG_REF_SR_FAST			(2 << 6)
#define RS9_REG_REF_SR_FASTER			(3 << 6)
#define RS9_REG_VID				0x5
#define RS9_REG_DID				0x6
#define RS9_REG_BCP				0x7

#define RS9_REG_VID_MASK			GENMASK(3, 0)
#define RS9_REG_VID_IDT				0x01

#define RS9_REG_DID_TYPE_FGV			(0x0 << RS9_REG_DID_TYPE_SHIFT)
#define RS9_REG_DID_TYPE_DBV			(0x1 << RS9_REG_DID_TYPE_SHIFT)
#define RS9_REG_DID_TYPE_DMV			(0x2 << RS9_REG_DID_TYPE_SHIFT)
#define RS9_REG_DID_TYPE_SHIFT			0x6

/* Structure to describe features of a particular 9-series model */
struct rs9_chip_info {
	unsigned int		num_clks;
	u8			outshift;
	u8			did;
};

struct rs9_driver_data {
	struct udevice		*i2c;
	struct rs9_chip_info	*chip_info;
	unsigned long		rate;
	u8			pll_amplitude;
	u8			pll_ssc;
	u8			clk_dif_sr;
};

static int clk_rs9_request(struct clk *clk)
{
	struct rs9_driver_data *rs9 = dev_get_priv(clk->dev);

	if (clk->id > rs9->chip_info->num_clks)
		return -EINVAL;

	return 0;
}

static ulong clk_rs9_get_rate(struct clk *clk)
{
	struct rs9_driver_data *rs9 = dev_get_priv(clk->dev);

	return rs9->rate * 4;
}

static const struct clk_ops clk_rs9_ops = {
	.request = clk_rs9_request,
	.get_rate = clk_rs9_get_rate,
};

static int rs9_write(struct udevice *dev, u8 reg, u8 val)
{
	u8 data[2] = { 1, val };

	return dm_i2c_write(dev, reg, data, 2);
}

static int rs9_read(struct udevice *dev, u8 reg, u8 *val)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct i2c_msg xfer[2];
	u8 txdata = reg;
	u8 rxdata[2];
	int ret;

	xfer[0].addr = chip->chip_addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = (void *)&txdata;

	xfer[1].addr = chip->chip_addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 2;
	xfer[1].buf = (void *)rxdata;

	ret = dm_i2c_xfer(dev, xfer, 2);
	if (ret < 0)
		return ret;

	/*
	 * Byte 0 is transfer length, which is always 1 due
	 * to BCP register programming to 1 in rs9_probe(),
	 * ignore it and use data from Byte 1.
	 */
	*val = rxdata[1];
	return 0;
}

static u8 rs9_calc_dif(const struct rs9_driver_data *rs9, int idx)
{
	/*
	 * On 9FGV0241, the DIF OE0 is BIT(1) and DIF OE(1) is BIT(2),
	 * on 9FGV0441 and 9FGV0841 the DIF OE0 is BIT(0) and so on.
	 * Increment the index in the 9FGV0241 special case here.
	 */
	return BIT(idx + rs9->chip_info->outshift);
}

static int rs9_get_output_config(struct rs9_driver_data *rs9, int idx)
{
	struct udevice *dev = rs9->i2c;
	u8 dif = rs9_calc_dif(rs9, idx);
	unsigned char name[5] = "DIF0";
	ofnode np;
	u32 sr;

	/* Set defaults */
	rs9->clk_dif_sr |= dif;

	snprintf(name, 5, "DIF%d", idx);
	np = dev_read_subnode(dev, name);
	if (!ofnode_valid(np))
		return 0;

	/* Output clock slew rate */
	sr = ofnode_read_u32_default(np, "renesas,slew-rate", 3000000);
	if (sr == 2000000) {		/* 2V/ns */
		rs9->clk_dif_sr &= ~dif;
	} else if (sr == 3000000) {	/* 3V/ns (default) */
		rs9->clk_dif_sr |= dif;
	} else {
		dev_err(dev, "Invalid renesas,slew-rate value\n");
		return -EINVAL;
	}

	return 0;
}

static int rs9_get_common_config(struct rs9_driver_data *rs9)
{
	struct udevice *dev = rs9->i2c;
	unsigned int amp, ssc;

	/* Set defaults */
	rs9->pll_amplitude = RS9_REG_SS_AMP_DEFAULT;
	rs9->pll_ssc = RS9_REG_SS_SSC_DEFAULT;

	/* Output clock amplitude */
	amp = dev_read_u32_default(dev, "renesas,out-amplitude-microvolt", 700000);
	if (amp == 600000) {	/* 0.6V */
		rs9->pll_amplitude = RS9_REG_SS_AMP_0V6;
	} else if (amp == 700000) {	/* 0.7V (default) */
		rs9->pll_amplitude = RS9_REG_SS_AMP_0V7;
	} else if (amp == 800000) {	/* 0.8V */
		rs9->pll_amplitude = RS9_REG_SS_AMP_0V8;
	} else if (amp == 900000) {	/* 0.9V */
		rs9->pll_amplitude = RS9_REG_SS_AMP_0V9;
	} else {
		dev_err(dev, "Invalid renesas,out-amplitude-microvolt value\n");
		return -EINVAL;
	}

	/* Output clock spread spectrum */
	ssc = dev_read_u32_default(dev, "renesas,out-spread-spectrum", 100000);
	if (ssc == 100000) {	/* 100% ... no spread (default) */
		rs9->pll_ssc = RS9_REG_SS_SSC_100;
	} else if (ssc == 99750) {	/* -0.25% ... down spread */
		rs9->pll_ssc = RS9_REG_SS_SSC_M025;
	} else if (ssc == 99500) {	/* -0.50% ... down spread */
		rs9->pll_ssc = RS9_REG_SS_SSC_M050;
	} else {
		dev_err(dev, "Invalid renesas,out-spread-spectrum value\n");
		return -EINVAL;
	}

	return 0;
}

static void rs9_update_config(struct rs9_driver_data *rs9)
{
	struct udevice *dev = rs9->i2c;
	int i;

	/* If amplitude is non-default, update it. */
	if (rs9->pll_amplitude != RS9_REG_SS_AMP_DEFAULT) {
		dm_i2c_reg_clrset(dev, RS9_REG_SS, RS9_REG_SS_AMP_MASK,
				  rs9->pll_amplitude);
	}

	/* If SSC is non-default, update it. */
	if (rs9->pll_ssc != RS9_REG_SS_SSC_DEFAULT) {
		dm_i2c_reg_clrset(dev, RS9_REG_SS, RS9_REG_SS_SSC_MASK,
				  rs9->pll_ssc);
	}

	for (i = 0; i < rs9->chip_info->num_clks; i++) {
		u8 dif = rs9_calc_dif(rs9, i);

		if (rs9->clk_dif_sr & dif)
			continue;

		dm_i2c_reg_clrset(dev, RS9_REG_SR, dif,
				  rs9->clk_dif_sr & dif);
	}
}

static int clk_rs9_probe(struct udevice *dev)
{
	struct rs9_driver_data *rs9 = dev_get_priv(dev);
	struct clk *clk;
	u8 vid, did;
	int i, ret;

	rs9->i2c = dev;
	rs9->chip_info = (struct rs9_chip_info *)dev_get_driver_data(dev);
	if (!rs9->chip_info)
		return -EINVAL;

	clk = devm_clk_get(dev, NULL);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	rs9->rate = clk_get_rate(clk);

	/* Fetch common configuration from DT (if specified) */
	ret = rs9_get_common_config(rs9);
	if (ret)
		return ret;

	/* Fetch DIFx output configuration from DT (if specified) */
	for (i = 0; i < rs9->chip_info->num_clks; i++) {
		ret = rs9_get_output_config(rs9, i);
		if (ret)
			return ret;
	}

	/* Always read back 1 Byte via I2C */
	ret = rs9_write(dev, RS9_REG_BCP, 1);
	if (ret < 0)
		return ret;

	ret = rs9_read(dev, RS9_REG_VID, &vid);
	if (ret < 0)
		return ret;

	ret = rs9_read(dev, RS9_REG_DID, &did);
	if (ret < 0)
		return ret;

	vid &= RS9_REG_VID_MASK;
	if (vid != RS9_REG_VID_IDT || did != rs9->chip_info->did) {
		dev_err(dev, "Incorrect VID/DID: %#02x, %#02x. Expected %#02x, %#02x\n",
			vid, did, RS9_REG_VID_IDT, rs9->chip_info->did);
		return -ENODEV;
	}

	rs9_update_config(rs9);

	return 0;
}

static const struct rs9_chip_info renesas_9fgv0241_info = {
	.num_clks	= 2,
	.outshift	= 1,
	.did		= RS9_REG_DID_TYPE_FGV | 0x02,
};

static const struct rs9_chip_info renesas_9fgv0441_info = {
	.num_clks	= 4,
	.outshift	= 0,
	.did		= RS9_REG_DID_TYPE_FGV | 0x04,
};

static const struct rs9_chip_info renesas_9fgv0841_info = {
	.num_clks	= 8,
	.outshift	= 0,
	.did		= RS9_REG_DID_TYPE_FGV | 0x08,
};

static const struct udevice_id clk_rs9_of_match[] = {
	{ .compatible = "renesas,9fgv0241", .data = (ulong)&renesas_9fgv0241_info },
	{ .compatible = "renesas,9fgv0441", .data = (ulong)&renesas_9fgv0441_info },
	{ .compatible = "renesas,9fgv0841", .data = (ulong)&renesas_9fgv0841_info },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(clk_rs9) = {
	.name = "clk-renesas-pcie-9series",
	.id = UCLASS_CLK,
	.of_match = clk_rs9_of_match,
	.ops = &clk_rs9_ops,
	.probe = clk_rs9_probe,
	.priv_auto = sizeof(struct rs9_driver_data),
};
