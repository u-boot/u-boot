// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017, Fuzhou Rockchip Electronics Co., Ltd
 *
 * Rockchip SARADC driver for U-Boot
 */

#include <adc.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <power/regulator.h>

#define usleep_range(a, b) udelay((b))

#define SARADC_CTRL_CHN_MASK		GENMASK(2, 0)
#define SARADC_CTRL_POWER_CTRL		BIT(3)
#define SARADC_CTRL_IRQ_ENABLE		BIT(5)
#define SARADC_CTRL_IRQ_STATUS		BIT(6)

#define SARADC_TIMEOUT			(100 * 1000)

struct rockchip_saradc_regs_v1 {
	unsigned int data;
	unsigned int stas;
	unsigned int ctrl;
	unsigned int dly_pu_soc;
};

struct rockchip_saradc_regs_v2 {
	unsigned int conv_con;
#define SARADC2_SINGLE_MODE	BIT(5)
#define SARADC2_START		BIT(4)
#define SARADC2_CONV_CHANNELS	GENMASK(3, 0)
	unsigned int t_pd_soc;
	unsigned int t_as_soc;
	unsigned int t_das_soc;
	unsigned int t_sel_soc;
	unsigned int high_comp[16];
	unsigned int low_comp[16];
	unsigned int debounce;
	unsigned int ht_int_en;
	unsigned int lt_int_en;
	unsigned int reserved[24];
	unsigned int mt_int_en;
	unsigned int end_int_en;
#define SARADC2_EN_END_INT	BIT(0)
	unsigned int st_con;
	unsigned int status;
	unsigned int end_int_st;
	unsigned int ht_int_st;
	unsigned int lt_int_st;
	unsigned int mt_int_st;
	unsigned int data[16];
	unsigned int auto_ch_en;
};

union rockchip_saradc_regs {
	struct rockchip_saradc_regs_v1	*v1;
	struct rockchip_saradc_regs_v2	*v2;
};
struct rockchip_saradc_data {
	int				num_bits;
	int				num_channels;
	unsigned long			clk_rate;
	int (*channel_data)(struct udevice *dev, int channel, unsigned int *data);
	int (*start_channel)(struct udevice *dev, int channel);
	int (*stop)(struct udevice *dev);
};

struct rockchip_saradc_priv {
	union rockchip_saradc_regs		regs;
	int					active_channel;
	const struct rockchip_saradc_data	*data;
	struct reset_ctl			*reset;
};

int rockchip_saradc_channel_data_v1(struct udevice *dev, int channel,
				    unsigned int *data)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);

	if ((readl(&priv->regs.v1->ctrl) & SARADC_CTRL_IRQ_STATUS) !=
	    SARADC_CTRL_IRQ_STATUS)
		return -EBUSY;

	/* Read value */
	*data = readl(&priv->regs.v1->data);

	/* Power down adc */
	writel(0, &priv->regs.v1->ctrl);

	return 0;
}

int rockchip_saradc_channel_data_v2(struct udevice *dev, int channel,
				    unsigned int *data)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);

	if (!(readl(&priv->regs.v2->end_int_st) & SARADC2_EN_END_INT))
		return -EBUSY;

	/* Read value */
	*data = readl(&priv->regs.v2->data[channel]);

	/* Acknowledge the interrupt */
	writel(SARADC2_EN_END_INT, &priv->regs.v2->end_int_st);

	return 0;
}
int rockchip_saradc_channel_data(struct udevice *dev, int channel,
				 unsigned int *data)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	int ret;

	if (channel != priv->active_channel) {
		pr_err("Requested channel is not active!");
		return -EINVAL;
	}

	ret = priv->data->channel_data(dev, channel, data);
	if (ret) {
		if (ret != -EBUSY)
			pr_err("Error reading channel data, %d!", ret);
		return ret;
	}

	*data &= uc_pdata->data_mask;

	return 0;
}

int rockchip_saradc_start_channel_v1(struct udevice *dev, int channel)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);

	/* 8 clock periods as delay between power up and start cmd */
	writel(8, &priv->regs.v1->dly_pu_soc);

	/* Select the channel to be used and trigger conversion */
	writel(SARADC_CTRL_POWER_CTRL | (channel & SARADC_CTRL_CHN_MASK) |
	       SARADC_CTRL_IRQ_ENABLE, &priv->regs.v1->ctrl);

	return 0;
}

static void rockchip_saradc_reset_controller(struct reset_ctl *reset)
{
	reset_assert(reset);
	usleep_range(10, 20);
	reset_deassert(reset);
}

int rockchip_saradc_start_channel_v2(struct udevice *dev, int channel)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);

	/*
	 * Downstream says
	 * """If read other chn at anytime, then chn1 will error, assert
	 * controller as a workaround."""
	 */
	if (priv->reset)
		rockchip_saradc_reset_controller(priv->reset);

	writel(0xc, &priv->regs.v2->t_das_soc);
	writel(0x20, &priv->regs.v2->t_pd_soc);

	/* Acknowledge any previous interrupt */
	writel(SARADC2_EN_END_INT, &priv->regs.v2->end_int_st);

	rk_clrsetreg(&priv->regs.v2->conv_con,
		     SARADC2_CONV_CHANNELS | SARADC2_START | SARADC2_SINGLE_MODE,
		     FIELD_PREP(SARADC2_CONV_CHANNELS, channel) |
		     FIELD_PREP(SARADC2_START, 1) |
		     FIELD_PREP(SARADC2_SINGLE_MODE, 1));

	return 0;
}

int rockchip_saradc_start_channel(struct udevice *dev, int channel)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);
	int ret;

	if (channel < 0 || channel >= priv->data->num_channels) {
		pr_err("Requested channel is invalid!");
		return -EINVAL;
	}

	ret = priv->data->start_channel(dev, channel);
	if (ret) {
		pr_err("Error starting channel, %d!", ret);
		return ret;
	}

	priv->active_channel = channel;

	return 0;
}

int rockchip_saradc_stop_v1(struct udevice *dev)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);

	/* Power down adc */
	writel(0, &priv->regs.v1->ctrl);

	return 0;
}

int rockchip_saradc_stop(struct udevice *dev)
{
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);

	if (priv->data->stop) {
		int ret = priv->data->stop(dev);

		if (ret) {
			pr_err("Error stopping channel, %d!", ret);
			return ret;
		}
	}

	priv->active_channel = -1;

	return 0;
}

int rockchip_saradc_probe(struct udevice *dev)
{
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);
	struct udevice *vref = NULL;
	struct clk clk;
	int vref_uv;
	int ret;

	priv->reset = devm_reset_control_get_optional(dev, "saradc-apb");

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_set_rate(&clk, priv->data->clk_rate);
	if (IS_ERR_VALUE(ret))
		return ret;

	priv->active_channel = -1;

	ret = device_get_supply_regulator(dev, "vref-supply", &vref);
	if (ret && uc_pdata->vdd_microvolts <= 0) {
		printf("can't get vref-supply: %d\n", ret);
		return ret;
	}

	if (priv->reset)
		rockchip_saradc_reset_controller(priv->reset);

	if (vref)
		vref_uv = regulator_get_value(vref);
	else
		vref_uv = uc_pdata->vdd_microvolts;
	if (vref_uv < 0) {
		printf("can't get vref-supply value: %d\n", vref_uv);
		return vref_uv;
	}

	/* VDD supplied by common vref pin */
	uc_pdata->vdd_supply = vref;
	uc_pdata->vdd_microvolts = vref_uv;
	uc_pdata->vss_microvolts = 0;

	return 0;
}

int rockchip_saradc_of_to_plat(struct udevice *dev)
{
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	struct rockchip_saradc_priv *priv = dev_get_priv(dev);
	struct rockchip_saradc_data *data;

	data = (struct rockchip_saradc_data *)dev_get_driver_data(dev);
	priv->regs.v1 = dev_read_addr_ptr(dev);
	if (!priv->regs.v1) {
		pr_err("Dev: %s - can't get address!", dev->name);
		return -EINVAL;
	}

	priv->data = data;
	uc_pdata->data_mask = (1 << priv->data->num_bits) - 1;
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = SARADC_TIMEOUT / 5;
	uc_pdata->channel_mask = (1 << priv->data->num_channels) - 1;

	return 0;
}

static const struct adc_ops rockchip_saradc_ops = {
	.start_channel = rockchip_saradc_start_channel,
	.channel_data = rockchip_saradc_channel_data,
	.stop = rockchip_saradc_stop,
};

static const struct rockchip_saradc_data saradc_data = {
	.num_bits = 10,
	.num_channels = 3,
	.clk_rate = 1000000,
	.channel_data = rockchip_saradc_channel_data_v1,
	.start_channel = rockchip_saradc_start_channel_v1,
	.stop = rockchip_saradc_stop_v1,
};

static const struct rockchip_saradc_data rk3066_tsadc_data = {
	.num_bits = 12,
	.num_channels = 2,
	.clk_rate = 50000,
	.channel_data = rockchip_saradc_channel_data_v1,
	.start_channel = rockchip_saradc_start_channel_v1,
	.stop = rockchip_saradc_stop_v1,
};

static const struct rockchip_saradc_data rk3399_saradc_data = {
	.num_bits = 10,
	.num_channels = 6,
	.clk_rate = 1000000,
	.channel_data = rockchip_saradc_channel_data_v1,
	.start_channel = rockchip_saradc_start_channel_v1,
	.stop = rockchip_saradc_stop_v1,
};

static const struct rockchip_saradc_data rk3528_saradc_data = {
	.num_bits = 10,
	.num_channels = 4,
	.clk_rate = 1000000,
	.channel_data = rockchip_saradc_channel_data_v2,
	.start_channel = rockchip_saradc_start_channel_v2,
};

static const struct rockchip_saradc_data rk3588_saradc_data = {
	.num_bits = 12,
	.num_channels = 8,
	.clk_rate = 1000000,
	.channel_data = rockchip_saradc_channel_data_v2,
	.start_channel = rockchip_saradc_start_channel_v2,
};

static const struct udevice_id rockchip_saradc_ids[] = {
	{ .compatible = "rockchip,saradc",
	  .data = (ulong)&saradc_data },
	{ .compatible = "rockchip,rk3066-tsadc",
	  .data = (ulong)&rk3066_tsadc_data },
	{ .compatible = "rockchip,rk3399-saradc",
	  .data = (ulong)&rk3399_saradc_data },
	{ .compatible = "rockchip,rk3528-saradc",
	  .data = (ulong)&rk3528_saradc_data },
	{ .compatible = "rockchip,rk3588-saradc",
	  .data = (ulong)&rk3588_saradc_data },
	{ }
};

U_BOOT_DRIVER(rockchip_saradc) = {
	.name		= "rockchip_saradc",
	.id		= UCLASS_ADC,
	.of_match	= rockchip_saradc_ids,
	.ops		= &rockchip_saradc_ops,
	.probe		= rockchip_saradc_probe,
	.of_to_plat = rockchip_saradc_of_to_plat,
	.priv_auto	= sizeof(struct rockchip_saradc_priv),
};
