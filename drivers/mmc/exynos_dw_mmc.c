// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#include <clk.h>
#include <dwmmc.h>
#include <asm/global_data.h>
#include <malloc.h>
#include <errno.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/gpio.h>
#include <linux/err.h>
#include <linux/printk.h>

#define	DWMMC_MAX_CH_NUM		4
#define	DWMMC_MAX_FREQ			52000000
#define	DWMMC_MIN_FREQ			400000
#define	DWMMC_MMC0_SDR_TIMING_VAL	0x03030001
#define	DWMMC_MMC2_SDR_TIMING_VAL	0x03020001

#define EXYNOS4412_FIXED_CIU_CLK_DIV	4

/* Quirks */
#define DWMCI_QUIRK_DISABLE_SMU		BIT(0)

#ifdef CONFIG_DM_MMC
#include <dm.h>
DECLARE_GLOBAL_DATA_PTR;

struct exynos_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};
#endif

/* Chip specific data */
struct exynos_dwmmc_variant {
	u32 clksel;		/* CLKSEL register offset */
	u8 div;			/* (optional) fixed clock divider value: 0..7 */
	u32 quirks;		/* quirk flags - see DWMCI_QUIRK_... */
};

/* Exynos implementation specific driver private data */
struct dwmci_exynos_priv_data {
#ifdef CONFIG_DM_MMC
	struct dwmci_host host;
#endif
	struct clk clk;
	u32 sdr_timing;
	u32 ddr_timing;
	const struct exynos_dwmmc_variant *chip;
};

static struct dwmci_exynos_priv_data *exynos_dwmmc_get_priv(
		struct dwmci_host *host)
{
#ifdef CONFIG_DM_MMC
	return container_of(host, struct dwmci_exynos_priv_data, host);
#else
	return host->priv;
#endif
}

/**
 * exynos_dwmmc_get_sclk - Get source clock (SDCLKIN) rate
 * @host: MMC controller object
 * @rate: Will contain clock rate, Hz
 *
 * Return: 0 on success or negative value on error
 */
static int exynos_dwmmc_get_sclk(struct dwmci_host *host, unsigned long *rate)
{
#ifdef CONFIG_CPU_V7A
	*rate = get_mmc_clk(host->dev_index);
#else
	struct dwmci_exynos_priv_data *priv = exynos_dwmmc_get_priv(host);

	*rate = clk_get_rate(&priv->clk);
#endif

	if (IS_ERR_VALUE(*rate))
		return *rate;

	return 0;
}

/**
 * exynos_dwmmc_set_sclk - Set source clock (SDCLKIN) rate
 * @host: MMC controller object
 * @rate: Desired clock rate, Hz
 *
 * Return: 0 on success or negative value on error
 */
static int exynos_dwmmc_set_sclk(struct dwmci_host *host, unsigned long rate)
{
	int err;

#ifdef CONFIG_CPU_V7A
	unsigned long sclk;
	unsigned int div;

	err = exynos_dwmmc_get_sclk(host, &sclk);
	if (err)
		return err;

	div = DIV_ROUND_UP(sclk, rate);
	set_mmc_clk(host->dev_index, div);
#else
	struct dwmci_exynos_priv_data *priv = exynos_dwmmc_get_priv(host);

	err = clk_set_rate(&priv->clk, rate);
	if (err < 0)
		return err;
#endif

	return 0;
}

/* Configure CLKSEL register with chosen timing values */
static int exynos_dwmci_clksel(struct dwmci_host *host)
{
	struct dwmci_exynos_priv_data *priv = exynos_dwmmc_get_priv(host);
	u32 timing;

	if (host->mmc->selected_mode == MMC_DDR_52)
		timing = priv->ddr_timing;
	else
		timing = priv->sdr_timing;

	dwmci_writel(host, priv->chip->clksel, timing);

	return 0;
}

/**
 * exynos_dwmmc_get_ciu_div - Get internal clock divider value
 * @host: MMC controller object
 *
 * Returns: Divider value, in range of 1..8
 */
static u8 exynos_dwmmc_get_ciu_div(struct dwmci_host *host)
{
	struct dwmci_exynos_priv_data *priv = exynos_dwmmc_get_priv(host);

	if (priv->chip->div)
		return priv->chip->div + 1;

	/*
	 * Since SDCLKIN is divided inside controller by the DIVRATIO
	 * value set in the CLKSEL register, we need to use the same output
	 * clock value to calculate the CLKDIV value.
	 * as per user manual:cclk_in = SDCLKIN / (DIVRATIO + 1)
	 */
	return ((dwmci_readl(host, priv->chip->clksel) >> DWMCI_DIVRATIO_BIT)
				& DWMCI_DIVRATIO_MASK) + 1;
}

static unsigned int exynos_dwmci_get_clk(struct dwmci_host *host, uint freq)
{
	unsigned long sclk;
	u8 clk_div;
	int err;

	/* Should be double rate for DDR mode */
	if (host->mmc->selected_mode == MMC_DDR_52 && host->mmc->bus_width == 8)
		freq *= 2;

	clk_div = exynos_dwmmc_get_ciu_div(host);
	err = exynos_dwmmc_set_sclk(host, freq * clk_div);
	if (err) {
		printf("DWMMC%d: failed to set clock rate (%d); "
		       "continue anyway\n", host->dev_index, err);
	}

	err = exynos_dwmmc_get_sclk(host, &sclk);
	if (err) {
		printf("DWMMC%d: failed to get clock rate (%d)\n",
		       host->dev_index, err);
		return 0;
	}

	return sclk / clk_div;
}

static void exynos_dwmci_board_init(struct dwmci_host *host)
{
	struct dwmci_exynos_priv_data *priv = exynos_dwmmc_get_priv(host);

	if (priv->chip->quirks & DWMCI_QUIRK_DISABLE_SMU) {
		dwmci_writel(host, EMMCP_MPSBEGIN0, 0);
		dwmci_writel(host, EMMCP_SEND0, 0);
		dwmci_writel(host, EMMCP_CTRL0,
			     MPSCTRL_SECURE_READ_BIT |
			     MPSCTRL_SECURE_WRITE_BIT |
			     MPSCTRL_NON_SECURE_READ_BIT |
			     MPSCTRL_NON_SECURE_WRITE_BIT | MPSCTRL_VALID);
	}

	if (priv->sdr_timing)
		exynos_dwmci_clksel(host);
}

#ifdef CONFIG_DM_MMC
static int exynos_dwmmc_of_to_plat(struct udevice *dev)
{
	struct dwmci_exynos_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	u32 div, timing[2];
	int err;

	priv->chip = (struct exynos_dwmmc_variant *)dev_get_driver_data(dev);

#ifdef CONFIG_CPU_V7A
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);

	/* Obtain device ID for current MMC channel */
	host->dev_id = pinmux_decode_periph_id(blob, node);
	host->dev_index = dev_read_u32_default(dev, "index", host->dev_id);
	if (host->dev_index == host->dev_id)
		host->dev_index = host->dev_id - PERIPH_ID_SDMMC0;

	if (host->dev_index > 4) {
		printf("DWMMC%d: Can't get the dev index\n", host->dev_index);
		return -EINVAL;
	}
#else
	if (dev_read_bool(dev, "non-removable"))
		host->dev_index = 0; /* eMMC */
	else
		host->dev_index = 2; /* SD card */
#endif

	host->ioaddr = dev_read_addr_ptr(dev);
	if (!host->ioaddr) {
		printf("DWMMC%d: Can't get base address\n", host->dev_index);
		return -EINVAL;
	}

	if (priv->chip->div)
		div = priv->chip->div;
	else
		div = dev_read_u32_default(dev, "samsung,dw-mshc-ciu-div", 0);

	err = dev_read_u32_array(dev, "samsung,dw-mshc-sdr-timing", timing, 2);
	if (err) {
		printf("DWMMC%d: Can't get sdr-timings\n", host->dev_index);
		return -EINVAL;
	}
	priv->sdr_timing = DWMCI_SET_SAMPLE_CLK(timing[0]) |
			   DWMCI_SET_DRV_CLK(timing[1]) |
			   DWMCI_SET_DIV_RATIO(div);

	/* sdr_timing wasn't set, use the default value */
	if (!priv->sdr_timing) {
		if (host->dev_index == 0)
			priv->sdr_timing = DWMMC_MMC0_SDR_TIMING_VAL;
		else if (host->dev_index == 2)
			priv->sdr_timing = DWMMC_MMC2_SDR_TIMING_VAL;
	}

	err = dev_read_u32_array(dev, "samsung,dw-mshc-ddr-timing", timing, 2);
	if (err) {
		debug("DWMMC%d: Can't get ddr-timings, using sdr-timings\n",
		      host->dev_index);
		priv->ddr_timing = priv->sdr_timing;
	} else {
		priv->ddr_timing = DWMCI_SET_SAMPLE_CLK(timing[0]) |
				   DWMCI_SET_DRV_CLK(timing[1]) |
				   DWMCI_SET_DIV_RATIO(div);
	}

	host->buswidth = dev_read_u32_default(dev, "bus-width", 4);
	host->fifo_depth = dev_read_u32_default(dev, "fifo-depth", 0);
	host->bus_hz = dev_read_u32_default(dev, "clock-frequency", 0);

	return 0;
}

static int exynos_dwmmc_probe(struct udevice *dev)
{
	struct exynos_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct dwmci_exynos_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	unsigned long freq;
	int err;

#ifndef CONFIG_CPU_V7A
	err = clk_get_by_index(dev, 1, &priv->clk); /* ciu */
	if (err)
		return err;
#endif

#ifdef CONFIG_CPU_V7A
	int flag;

	flag = host->buswidth == 8 ? PINMUX_FLAG_8BIT_MODE : PINMUX_FLAG_NONE;
	err = exynos_pinmux_config(host->dev_id, flag);
	if (err) {
		printf("DWMMC%d not configure\n", host->dev_index);
		return err;
	}
#endif

	if (host->bus_hz)
		freq = host->bus_hz;
	else
		freq = DWMMC_MAX_FREQ;

	err = exynos_dwmmc_set_sclk(host, freq);
	if (err) {
		printf("DWMMC%d: failed to set clock rate on probe (%d); "
		       "continue anyway\n", host->dev_index, err);
	}

	host->name = dev->name;
	host->board_init = exynos_dwmci_board_init;
	host->caps = MMC_MODE_DDR_52MHz;
	host->clksel = exynos_dwmci_clksel;
	host->get_mmc_clk = exynos_dwmci_get_clk;

#ifdef CONFIG_BLK
	dwmci_setup_cfg(&plat->cfg, host, DWMMC_MAX_FREQ, DWMMC_MIN_FREQ);
	host->mmc = &plat->mmc;
#else
	err = add_dwmci(host, DWMMC_MAX_FREQ, DWMMC_MIN_FREQ);
	if (err) {
		printf("DWMMC%d registration failed\n", host->dev_index);
		return err;
	}
#endif

	host->mmc->priv = &priv->host;
	upriv->mmc = host->mmc;
	host->mmc->dev = dev;
	host->priv = dev;

	return dwmci_probe(dev);
}

static int exynos_dwmmc_bind(struct udevice *dev)
{
	struct exynos_mmc_plat *plat = dev_get_plat(dev);

	return dwmci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct exynos_dwmmc_variant exynos4_drv_data = {
	.clksel	= DWMCI_CLKSEL,
	.div	= EXYNOS4412_FIXED_CIU_CLK_DIV - 1,
};

static const struct exynos_dwmmc_variant exynos5_drv_data = {
	.clksel	= DWMCI_CLKSEL,
#ifdef CONFIG_EXYNOS5420
	.quirks	= DWMCI_QUIRK_DISABLE_SMU,
#endif
};

static const struct exynos_dwmmc_variant exynos7_smu_drv_data = {
	.clksel	= DWMCI_CLKSEL64,
	.quirks	= DWMCI_QUIRK_DISABLE_SMU,
};

static const struct udevice_id exynos_dwmmc_ids[] = {
	{
		.compatible	= "samsung,exynos4412-dw-mshc",
		.data		= (ulong)&exynos4_drv_data,
	}, {
		.compatible	= "samsung,exynos5420-dw-mshc-smu",
		.data		= (ulong)&exynos5_drv_data,
	}, {
		.compatible	= "samsung,exynos5420-dw-mshc",
		.data		= (ulong)&exynos5_drv_data,
	}, {
		.compatible	= "samsung,exynos-dwmmc",
		.data		= (ulong)&exynos5_drv_data,
	}, {
		.compatible	= "samsung,exynos7-dw-mshc-smu",
		.data		= (ulong)&exynos7_smu_drv_data,
	},
	{ }
};

U_BOOT_DRIVER(exynos_dwmmc_drv) = {
	.name		= "exynos_dwmmc",
	.id		= UCLASS_MMC,
	.of_match	= exynos_dwmmc_ids,
	.of_to_plat	= exynos_dwmmc_of_to_plat,
	.bind		= exynos_dwmmc_bind,
	.probe		= exynos_dwmmc_probe,
	.ops		= &dm_dwmci_ops,
	.priv_auto	= sizeof(struct dwmci_exynos_priv_data),
	.plat_auto	= sizeof(struct exynos_mmc_plat),
};
#endif
