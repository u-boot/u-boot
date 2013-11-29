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

#define	DWMMC_MAX_CH_NUM		4
#define	DWMMC_MAX_FREQ			52000000
#define	DWMMC_MIN_FREQ			400000
#define	DWMMC_MMC0_CLKSEL_VAL		0x03030001
#define	DWMMC_MMC2_CLKSEL_VAL		0x03020001

/*
 * Function used as callback function to initialise the
 * CLKSEL register for every mmc channel.
 */
static void exynos_dwmci_clksel(struct dwmci_host *host)
{
	dwmci_writel(host, DWMCI_CLKSEL, host->clksel_val);
}

unsigned int exynos_dwmci_get_clk(int dev_index)
{
	return get_mmc_clk(dev_index);
}

static void exynos_dwmci_board_init(struct dwmci_host *host)
{
	if (host->quirks & DWMCI_QUIRK_DISABLE_SMU) {
		dwmci_writel(host, EMMCP_MPSBEGIN0, 0);
		dwmci_writel(host, EMMCP_SEND0, 0);
		dwmci_writel(host, EMMCP_CTRL0,
			     MPSCTRL_SECURE_READ_BIT |
			     MPSCTRL_SECURE_WRITE_BIT |
			     MPSCTRL_NON_SECURE_READ_BIT |
			     MPSCTRL_NON_SECURE_WRITE_BIT | MPSCTRL_VALID);
	}
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
	unsigned int div;
	unsigned long freq, sclk;
	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return 1;
	}
	/* request mmc clock vlaue of 52MHz.  */
	freq = 52000000;
	sclk = get_mmc_clk(index);
	div = DIV_ROUND_UP(sclk, freq);
	/* set the clock divisor for mmc */
	set_mmc_clk(index, div);

	host->name = "EXYNOS DWMMC";
	host->ioaddr = (void *)regbase;
	host->buswidth = bus_width;
#ifdef CONFIG_EXYNOS5420
	host->quirks = DWMCI_QUIRK_DISABLE_SMU;
#endif
	host->board_init = exynos_dwmci_board_init;

	if (clksel) {
		host->clksel_val = clksel;
	} else {
		if (0 == index)
			host->clksel_val = DWMMC_MMC0_CLKSEL_VAL;
		if (2 == index)
			host->clksel_val = DWMMC_MMC2_CLKSEL_VAL;
	}

	host->clksel = exynos_dwmci_clksel;
	host->dev_index = index;
	host->get_mmc_clk = exynos_dwmci_get_clk;
	/* Add the mmc channel to be registered with mmc core */
	if (add_dwmci(host, DWMMC_MAX_FREQ, DWMMC_MIN_FREQ)) {
		debug("dwmmc%d registration failed\n", index);
		return -1;
	}
	return 0;
}

#ifdef CONFIG_OF_CONTROL
int exynos_dwmmc_init(const void *blob)
{
	int index, bus_width;
	int node_list[DWMMC_MAX_CH_NUM];
	int err = 0, dev_id, flag, count, i;
	u32 clksel_val, base, timing[3];

	count = fdtdec_find_aliases_for_id(blob, "mmc",
				COMPAT_SAMSUNG_EXYNOS5_DWMMC, node_list,
				DWMMC_MAX_CH_NUM);

	for (i = 0; i < count; i++) {
		int node = node_list[i];

		if (node <= 0)
			continue;

		/* Extract device id for each mmc channel */
		dev_id = pinmux_decode_periph_id(blob, node);

		/* Get the bus width from the device node */
		bus_width = fdtdec_get_int(blob, node, "samsung,bus-width", 0);
		if (bus_width <= 0) {
			debug("DWMMC: Can't get bus-width\n");
			return -1;
		}
		if (8 == bus_width)
			flag = PINMUX_FLAG_8BIT_MODE;
		else
			flag = PINMUX_FLAG_NONE;

		/* config pinmux for each mmc channel */
		err = exynos_pinmux_config(dev_id, flag);
		if (err) {
			debug("DWMMC not configured\n");
			return err;
		}

		index = dev_id - PERIPH_ID_SDMMC0;

		/* Get the base address from the device node */
		base = fdtdec_get_addr(blob, node, "reg");
		if (!base) {
			debug("DWMMC: Can't get base address\n");
			return -1;
		}
		/* Extract the timing info from the node */
		err = fdtdec_get_int_array(blob, node, "samsung,timing",
					timing, 3);
		if (err) {
			debug("Can't get sdr-timings for divider\n");
			return -1;
		}

		clksel_val = (DWMCI_SET_SAMPLE_CLK(timing[0]) |
				DWMCI_SET_DRV_CLK(timing[1]) |
				DWMCI_SET_DIV_RATIO(timing[2]));
		/* Initialise each mmc channel */
		err = exynos_dwmci_add_port(index, base, bus_width, clksel_val);
		if (err)
			debug("dwmmc Channel-%d init failed\n", index);
	}
	return 0;
}
#endif
