/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dwmmc.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/gpio.h>
#include <asm-generic/errno.h>

#define	DWMMC_MAX_CH_NUM		4
#define	DWMMC_MAX_FREQ			52000000
#define	DWMMC_MIN_FREQ			400000
#define	DWMMC_MMC0_SDR_TIMING_VAL	0x03030001
#define	DWMMC_MMC2_SDR_TIMING_VAL	0x03020001

/* Exynos implmentation specific drver private data */
struct dwmci_exynos_priv_data {
	u32 sdr_timing;
};

/*
 * Function used as callback function to initialise the
 * CLKSEL register for every mmc channel.
 */
static void exynos_dwmci_clksel(struct dwmci_host *host)
{
	struct dwmci_exynos_priv_data *priv = host->priv;

	dwmci_writel(host, DWMCI_CLKSEL, priv->sdr_timing);
}

unsigned int exynos_dwmci_get_clk(struct dwmci_host *host)
{
	unsigned long sclk;
	int8_t clk_div;

	/*
	 * Since SDCLKIN is divided inside controller by the DIVRATIO
	 * value set in the CLKSEL register, we need to use the same output
	 * clock value to calculate the CLKDIV value.
	 * as per user manual:cclk_in = SDCLKIN / (DIVRATIO + 1)
	 */
	clk_div = ((dwmci_readl(host, DWMCI_CLKSEL) >> DWMCI_DIVRATIO_BIT)
			& DWMCI_DIVRATIO_MASK) + 1;
	sclk = get_mmc_clk(host->dev_index);

	/*
	 * Assume to know divider value.
	 * When clock unit is broken, need to set "host->div"
	 */
	return sclk / clk_div / (host->div + 1);
}

static void exynos_dwmci_board_init(struct dwmci_host *host)
{
	struct dwmci_exynos_priv_data *priv = host->priv;

	if (host->quirks & DWMCI_QUIRK_DISABLE_SMU) {
		dwmci_writel(host, EMMCP_MPSBEGIN0, 0);
		dwmci_writel(host, EMMCP_SEND0, 0);
		dwmci_writel(host, EMMCP_CTRL0,
			     MPSCTRL_SECURE_READ_BIT |
			     MPSCTRL_SECURE_WRITE_BIT |
			     MPSCTRL_NON_SECURE_READ_BIT |
			     MPSCTRL_NON_SECURE_WRITE_BIT | MPSCTRL_VALID);
	}

	/* Set to timing value at initial time */
	if (priv->sdr_timing)
		exynos_dwmci_clksel(host);
}

static int exynos_dwmci_core_init(struct dwmci_host *host, int index)
{
	unsigned int div;
	unsigned long freq, sclk;
	struct dwmci_exynos_priv_data *priv = host->priv;

	if (host->bus_hz)
		freq = host->bus_hz;
	else
		freq = DWMMC_MAX_FREQ;

	/* request mmc clock vlaue of 52MHz.  */
	sclk = get_mmc_clk(index);
	div = DIV_ROUND_UP(sclk, freq);
	/* set the clock divisor for mmc */
	set_mmc_clk(index, div);

	host->name = "EXYNOS DWMMC";
#ifdef CONFIG_EXYNOS5420
	host->quirks = DWMCI_QUIRK_DISABLE_SMU;
#endif
	host->board_init = exynos_dwmci_board_init;

	if (!priv->sdr_timing) {
		if (index == 0)
			priv->sdr_timing = DWMMC_MMC0_SDR_TIMING_VAL;
		else if (index == 2)
			priv->sdr_timing = DWMMC_MMC2_SDR_TIMING_VAL;
	}

	host->caps = MMC_MODE_DDR_52MHz;
	host->clksel = exynos_dwmci_clksel;
	host->dev_index = index;
	host->get_mmc_clk = exynos_dwmci_get_clk;
	/* Add the mmc channel to be registered with mmc core */
	if (add_dwmci(host, DWMMC_MAX_FREQ, DWMMC_MIN_FREQ)) {
		printf("DWMMC%d registration failed\n", index);
		return -1;
	}
	return 0;
}

/*
 * This function adds the mmc channel to be registered with mmc core.
 * index -	mmc channel number.
 * regbase -	register base address of mmc channel specified in 'index'.
 * bus_width -	operating bus width of mmc channel specified in 'index'.
 * clksel -	value to be written into CLKSEL register in case of FDT.
 *		NULL in case od non-FDT.
 */
int exynos_dwmci_add_port(int index, u32 regbase, int bus_width, u32 clksel)
{
	struct dwmci_host *host = NULL;
	struct dwmci_exynos_priv_data *priv;

	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		error("dwmci_host malloc fail!\n");
		return -ENOMEM;
	}

	priv = malloc(sizeof(struct dwmci_exynos_priv_data));
	if (!priv) {
		error("dwmci_exynos_priv_data malloc fail!\n");
		return -ENOMEM;
	}

	host->ioaddr = (void *)regbase;
	host->buswidth = bus_width;

	if (clksel)
		priv->sdr_timing = clksel;

	host->priv = priv;

	return exynos_dwmci_core_init(host, index);
}

#ifdef CONFIG_OF_CONTROL
static struct dwmci_host dwmci_host[DWMMC_MAX_CH_NUM];

static int do_dwmci_init(struct dwmci_host *host)
{
	int index, flag, err;

	index = host->dev_index;

	flag = host->buswidth == 8 ? PINMUX_FLAG_8BIT_MODE : PINMUX_FLAG_NONE;
	err = exynos_pinmux_config(host->dev_id, flag);
	if (err) {
		printf("DWMMC%d not configure\n", index);
		return err;
	}

	return exynos_dwmci_core_init(host, index);
}

static int exynos_dwmci_get_config(const void *blob, int node,
					struct dwmci_host *host)
{
	int err = 0;
	u32 base, timing[3];
	struct dwmci_exynos_priv_data *priv;

	priv = malloc(sizeof(struct dwmci_exynos_priv_data));
	if (!priv) {
		error("dwmci_exynos_priv_data malloc fail!\n");
		return -ENOMEM;
	}

	/* Extract device id for each mmc channel */
	host->dev_id = pinmux_decode_periph_id(blob, node);

	host->dev_index = fdtdec_get_int(blob, node, "index", host->dev_id);
	if (host->dev_index == host->dev_id)
		host->dev_index = host->dev_id - PERIPH_ID_SDMMC0;

	/* Get the bus width from the device node */
	host->buswidth = fdtdec_get_int(blob, node, "samsung,bus-width", 0);
	if (host->buswidth <= 0) {
		printf("DWMMC%d: Can't get bus-width\n", host->dev_index);
		return -EINVAL;
	}

	/* Set the base address from the device node */
	base = fdtdec_get_addr(blob, node, "reg");
	if (!base) {
		printf("DWMMC%d: Can't get base address\n", host->dev_index);
		return -EINVAL;
	}
	host->ioaddr = (void *)base;

	/* Extract the timing info from the node */
	err =  fdtdec_get_int_array(blob, node, "samsung,timing", timing, 3);
	if (err) {
		printf("DWMMC%d: Can't get sdr-timings for devider\n",
				host->dev_index);
		return -EINVAL;
	}

	priv->sdr_timing = (DWMCI_SET_SAMPLE_CLK(timing[0]) |
			DWMCI_SET_DRV_CLK(timing[1]) |
			DWMCI_SET_DIV_RATIO(timing[2]));

	/* sdr_timing didn't assigned anything, use the default value */
	if (!priv->sdr_timing) {
		if (host->dev_index == 0)
			priv->sdr_timing = DWMMC_MMC0_SDR_TIMING_VAL;
		else if (host->dev_index == 2)
			priv->sdr_timing = DWMMC_MMC2_SDR_TIMING_VAL;
	}

	host->fifoth_val = fdtdec_get_int(blob, node, "fifoth_val", 0);
	host->bus_hz = fdtdec_get_int(blob, node, "bus_hz", 0);
	host->div = fdtdec_get_int(blob, node, "div", 0);

	host->priv = priv;

	return 0;
}

static int exynos_dwmci_process_node(const void *blob,
					int node_list[], int count)
{
	struct dwmci_host *host;
	int i, node, err;

	for (i = 0; i < count; i++) {
		node = node_list[i];
		if (node <= 0)
			continue;
		host = &dwmci_host[i];
		err = exynos_dwmci_get_config(blob, node, host);
		if (err) {
			printf("%s: failed to decode dev %d\n", __func__, i);
			return err;
		}

		do_dwmci_init(host);
	}
	return 0;
}

int exynos_dwmmc_init(const void *blob)
{
	int compat_id;
	int node_list[DWMMC_MAX_CH_NUM];
	int boot_dev_node;
	int err = 0, count;

	compat_id = COMPAT_SAMSUNG_EXYNOS_DWMMC;

	count = fdtdec_find_aliases_for_id(blob, "mmc",
				compat_id, node_list, DWMMC_MAX_CH_NUM);

	/* For DWMMC always set boot device as mmc 0 */
	if (count >= 3 && get_boot_mode() == BOOT_MODE_SD) {
		boot_dev_node = node_list[2];
		node_list[2] = node_list[0];
		node_list[0] = boot_dev_node;
	}

	err = exynos_dwmci_process_node(blob, node_list, count);

	return err;
}
#endif
