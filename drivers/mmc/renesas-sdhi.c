// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <fdtdec.h>
#include <mmc.h>
#include <dm.h>
#include <linux/compat.h>
#include <linux/dma-direction.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <power/regulator.h>
#include <asm/unaligned.h>

#include "tmio-common.h"

#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)

/* SCC registers */
#define RENESAS_SDHI_SCC_DTCNTL			0x800
#define   RENESAS_SDHI_SCC_DTCNTL_TAPEN		BIT(0)
#define   RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT	16
#define   RENESAS_SDHI_SCC_DTCNTL_TAPNUM_MASK		0xff
#define RENESAS_SDHI_SCC_TAPSET			0x804
#define RENESAS_SDHI_SCC_DT2FF			0x808
#define RENESAS_SDHI_SCC_CKSEL			0x80c
#define   RENESAS_SDHI_SCC_CKSEL_DTSEL		BIT(0)
#define RENESAS_SDHI_SCC_RVSCNTL			0x810
#define   RENESAS_SDHI_SCC_RVSCNTL_RVSEN		BIT(0)
#define RENESAS_SDHI_SCC_RVSREQ			0x814
#define   RENESAS_SDHI_SCC_RVSREQ_RVSERR		BIT(2)
#define RENESAS_SDHI_SCC_SMPCMP			0x818
#define RENESAS_SDHI_SCC_TMPPORT2			0x81c

#define RENESAS_SDHI_MAX_TAP 3

static unsigned int renesas_sdhi_init_tuning(struct tmio_sd_priv *priv)
{
	u32 reg;

	/* Initialize SCC */
	tmio_sd_writel(priv, 0, TMIO_SD_INFO1);

	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg &= ~TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	/* Set sampling clock selection range */
	tmio_sd_writel(priv, 0x8 << RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT,
			   RENESAS_SDHI_SCC_DTCNTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_DTCNTL);
	reg |= RENESAS_SDHI_SCC_DTCNTL_TAPEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_DTCNTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_CKSEL);
	reg |= RENESAS_SDHI_SCC_CKSEL_DTSEL;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_CKSEL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	tmio_sd_writel(priv, 0x300 /* scc_tappos */,
			   RENESAS_SDHI_SCC_DT2FF);

	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg |= TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	/* Read TAPNUM */
	return (tmio_sd_readl(priv, RENESAS_SDHI_SCC_DTCNTL) >>
		RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT) &
		RENESAS_SDHI_SCC_DTCNTL_TAPNUM_MASK;
}

static void renesas_sdhi_reset_tuning(struct tmio_sd_priv *priv)
{
	u32 reg;

	/* Reset SCC */
	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg &= ~TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_CKSEL);
	reg &= ~RENESAS_SDHI_SCC_CKSEL_DTSEL;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_CKSEL);

	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg |= TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);
}

static void renesas_sdhi_prepare_tuning(struct tmio_sd_priv *priv,
				       unsigned long tap)
{
	/* Set sampling clock position */
	tmio_sd_writel(priv, tap, RENESAS_SDHI_SCC_TAPSET);
}

static unsigned int renesas_sdhi_compare_scc_data(struct tmio_sd_priv *priv)
{
	/* Get comparison of sampling data */
	return tmio_sd_readl(priv, RENESAS_SDHI_SCC_SMPCMP);
}

static int renesas_sdhi_select_tuning(struct tmio_sd_priv *priv,
				     unsigned int tap_num, unsigned int taps,
				     unsigned int smpcmp)
{
	unsigned long tap_cnt;  /* counter of tuning success */
	unsigned long tap_set;  /* tap position */
	unsigned long tap_start;/* start position of tuning success */
	unsigned long tap_end;  /* end position of tuning success */
	unsigned long ntap;     /* temporary counter of tuning success */
	unsigned long match_cnt;/* counter of matching data */
	unsigned long i;
	bool select = false;
	u32 reg;

	/* Clear SCC_RVSREQ */
	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_RVSREQ);

	/* Merge the results */
	for (i = 0; i < tap_num * 2; i++) {
		if (!(taps & BIT(i))) {
			taps &= ~BIT(i % tap_num);
			taps &= ~BIT((i % tap_num) + tap_num);
		}
		if (!(smpcmp & BIT(i))) {
			smpcmp &= ~BIT(i % tap_num);
			smpcmp &= ~BIT((i % tap_num) + tap_num);
		}
	}

	/*
	 * Find the longest consecutive run of successful probes.  If that
	 * is more than RENESAS_SDHI_MAX_TAP probes long then use the
	 * center index as the tap.
	 */
	tap_cnt = 0;
	ntap = 0;
	tap_start = 0;
	tap_end = 0;
	for (i = 0; i < tap_num * 2; i++) {
		if (taps & BIT(i))
			ntap++;
		else {
			if (ntap > tap_cnt) {
				tap_start = i - ntap;
				tap_end = i - 1;
				tap_cnt = ntap;
			}
			ntap = 0;
		}
	}

	if (ntap > tap_cnt) {
		tap_start = i - ntap;
		tap_end = i - 1;
		tap_cnt = ntap;
	}

	/*
	 * If all of the TAP is OK, the sampling clock position is selected by
	 * identifying the change point of data.
	 */
	if (tap_cnt == tap_num * 2) {
		match_cnt = 0;
		ntap = 0;
		tap_start = 0;
		tap_end = 0;
		for (i = 0; i < tap_num * 2; i++) {
			if (smpcmp & BIT(i))
				ntap++;
			else {
				if (ntap > match_cnt) {
					tap_start = i - ntap;
					tap_end = i - 1;
					match_cnt = ntap;
				}
				ntap = 0;
			}
		}
		if (ntap > match_cnt) {
			tap_start = i - ntap;
			tap_end = i - 1;
			match_cnt = ntap;
		}
		if (match_cnt)
			select = true;
	} else if (tap_cnt >= RENESAS_SDHI_MAX_TAP)
		select = true;

	if (select)
		tap_set = ((tap_start + tap_end) / 2) % tap_num;
	else
		return -EIO;

	/* Set SCC */
	tmio_sd_writel(priv, tap_set, RENESAS_SDHI_SCC_TAPSET);

	/* Enable auto re-tuning */
	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg |= RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	return 0;
}

int renesas_sdhi_execute_tuning(struct udevice *dev, uint opcode)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;
	unsigned int tap_num;
	unsigned int taps = 0, smpcmp = 0;
	int i, ret = 0;
	u32 caps;

	/* Only supported on Renesas RCar */
	if (!(priv->caps & TMIO_SD_CAP_RCAR_UHS))
		return -EINVAL;

	/* clock tuning is not needed for upto 52MHz */
	if (!((mmc->selected_mode == MMC_HS_200) ||
	      (mmc->selected_mode == UHS_SDR104) ||
	      (mmc->selected_mode == UHS_SDR50)))
		return 0;

	tap_num = renesas_sdhi_init_tuning(priv);
	if (!tap_num)
		/* Tuning is not supported */
		goto out;

	if (tap_num * 2 >= sizeof(taps) * 8) {
		dev_err(dev,
			"Too many taps, skipping tuning. Please consider updating size of taps field of tmio_mmc_host\n");
		goto out;
	}

	/* Issue CMD19 twice for each tap */
	for (i = 0; i < 2 * tap_num; i++) {
		renesas_sdhi_prepare_tuning(priv, i % tap_num);

		/* Force PIO for the tuning */
		caps = priv->caps;
		priv->caps &= ~TMIO_SD_CAP_DMA_INTERNAL;

		ret = mmc_send_tuning(mmc, opcode, NULL);

		priv->caps = caps;

		if (ret == 0)
			taps |= BIT(i);

		ret = renesas_sdhi_compare_scc_data(priv);
		if (ret == 0)
			smpcmp |= BIT(i);

		mdelay(1);
	}

	ret = renesas_sdhi_select_tuning(priv, tap_num, taps, smpcmp);

out:
	if (ret < 0) {
		dev_warn(dev, "Tuning procedure failed\n");
		renesas_sdhi_reset_tuning(priv);
	}

	return ret;
}
#endif

static int renesas_sdhi_set_ios(struct udevice *dev)
{
	int ret = tmio_sd_set_ios(dev);

	mdelay(10);

#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	renesas_sdhi_reset_tuning(priv);
#endif

	return ret;
}

static const struct dm_mmc_ops renesas_sdhi_ops = {
	.send_cmd = tmio_sd_send_cmd,
	.set_ios = renesas_sdhi_set_ios,
	.get_cd = tmio_sd_get_cd,
#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)
	.execute_tuning = renesas_sdhi_execute_tuning,
#endif
};

#define RENESAS_GEN2_QUIRKS	TMIO_SD_CAP_RCAR_GEN2
#define RENESAS_GEN3_QUIRKS				\
	TMIO_SD_CAP_64BIT | TMIO_SD_CAP_RCAR_GEN3 | TMIO_SD_CAP_RCAR_UHS

static const struct udevice_id renesas_sdhi_match[] = {
	{ .compatible = "renesas,sdhi-r8a7790", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7791", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7792", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7793", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7794", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7795", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7796", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77965", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77970", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77990", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77995", .data = RENESAS_GEN3_QUIRKS },
	{ /* sentinel */ }
};

static int renesas_sdhi_probe(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	u32 quirks = dev_get_driver_data(dev);
	struct fdt_resource reg_res;
	struct clk clk;
	DECLARE_GLOBAL_DATA_PTR;
	int ret;

	if (quirks == RENESAS_GEN2_QUIRKS) {
		ret = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev),
				       "reg", 0, &reg_res);
		if (ret < 0) {
			dev_err(dev, "\"reg\" resource not found, ret=%i\n",
				ret);
			return ret;
		}

		if (fdt_resource_size(&reg_res) == 0x100)
			quirks |= TMIO_SD_CAP_16BIT;
	}

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get host clock\n");
		return ret;
	}

	/* set to max rate */
	priv->mclk = clk_set_rate(&clk, ULONG_MAX);
	if (IS_ERR_VALUE(priv->mclk)) {
		dev_err(dev, "failed to set rate for host clock\n");
		clk_free(&clk);
		return priv->mclk;
	}

	ret = clk_enable(&clk);
	clk_free(&clk);
	if (ret) {
		dev_err(dev, "failed to enable host clock\n");
		return ret;
	}

	ret = tmio_sd_probe(dev, quirks);
#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)
	if (!ret)
		renesas_sdhi_reset_tuning(dev_get_priv(dev));
#endif
	return ret;
}

U_BOOT_DRIVER(renesas_sdhi) = {
	.name = "renesas-sdhi",
	.id = UCLASS_MMC,
	.of_match = renesas_sdhi_match,
	.bind = tmio_sd_bind,
	.probe = renesas_sdhi_probe,
	.priv_auto_alloc_size = sizeof(struct tmio_sd_priv),
	.platdata_auto_alloc_size = sizeof(struct tmio_sd_plat),
	.ops = &renesas_sdhi_ops,
};
