/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <asm/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/clk.h>
#include <errno.h>
#include <asm/arch/pinmux.h>

static char *S5P_NAME = "SAMSUNG SDHCI";
static void s5p_sdhci_set_control_reg(struct sdhci_host *host)
{
	unsigned long val, ctrl;
	/*
	 * SELCLKPADDS[17:16]
	 * 00 = 2mA
	 * 01 = 4mA
	 * 10 = 7mA
	 * 11 = 9mA
	 */
	sdhci_writel(host, SDHCI_CTRL4_DRIVE_MASK(0x3), SDHCI_CONTROL4);

	val = sdhci_readl(host, SDHCI_CONTROL2);
	val &= SDHCI_CTRL2_SELBASECLK_SHIFT;

	val |=	SDHCI_CTRL2_ENSTAASYNCCLR |
		SDHCI_CTRL2_ENCMDCNFMSK |
		SDHCI_CTRL2_ENFBCLKRX |
		SDHCI_CTRL2_ENCLKOUTHOLD;

	sdhci_writel(host, val, SDHCI_CONTROL2);

	/*
	 * FCSEL3[31] FCSEL2[23] FCSEL1[15] FCSEL0[7]
	 * FCSel[1:0] : Rx Feedback Clock Delay Control
	 *	Inverter delay means10ns delay if SDCLK 50MHz setting
	 *	01 = Delay1 (basic delay)
	 *	11 = Delay2 (basic delay + 2ns)
	 *	00 = Delay3 (inverter delay)
	 *	10 = Delay4 (inverter delay + 2ns)
	 */
	val = SDHCI_CTRL3_FCSEL0 | SDHCI_CTRL3_FCSEL1;
	sdhci_writel(host, val, SDHCI_CONTROL3);

	/*
	 * SELBASECLK[5:4]
	 * 00/01 = HCLK
	 * 10 = EPLL
	 * 11 = XTI or XEXTCLK
	 */
	ctrl = sdhci_readl(host, SDHCI_CONTROL2);
	ctrl &= ~SDHCI_CTRL2_SELBASECLK_MASK(0x3);
	ctrl |= SDHCI_CTRL2_SELBASECLK_MASK(0x2);
	sdhci_writel(host, ctrl, SDHCI_CONTROL2);
}

static int s5p_sdhci_core_init(struct sdhci_host *host)
{
	host->name = S5P_NAME;

	host->quirks = SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE |
		SDHCI_QUIRK_BROKEN_R1B | SDHCI_QUIRK_32BIT_DMA_ADDR |
		SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_USE_WIDE8;
	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	host->set_control_reg = &s5p_sdhci_set_control_reg;
	host->set_clock = set_mmc_clk;

	host->host_caps = MMC_MODE_HC;
	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;

	return add_sdhci(host, 52000000, 400000);
}

int s5p_sdhci_init(u32 regbase, int index, int bus_width)
{
	struct sdhci_host *host = malloc(sizeof(struct sdhci_host));
	if (!host) {
		printf("sdhci__host malloc fail!\n");
		return 1;
	}
	host->ioaddr = (void *)regbase;
	host->index = index;
	host->bus_width = bus_width;

	return s5p_sdhci_core_init(host);
}

#ifdef CONFIG_OF_CONTROL
struct sdhci_host sdhci_host[SDHCI_MAX_HOSTS];

static int do_sdhci_init(struct sdhci_host *host)
{
	char str[20];
	int dev_id, flag;
	int err = 0;

	flag = host->bus_width == 8 ? PINMUX_FLAG_8BIT_MODE : PINMUX_FLAG_NONE;
	dev_id = host->index + PERIPH_ID_SDMMC0;

	if (fdt_gpio_isvalid(&host->pwr_gpio)) {
		sprintf(str, "sdhci%d_power", host->index & 0xf);
		gpio_request(host->pwr_gpio.gpio, str);
		gpio_direction_output(host->pwr_gpio.gpio, 1);
		err = exynos_pinmux_config(dev_id, flag);
		if (err) {
			debug("MMC not configured\n");
			return err;
		}
	}

	if (fdt_gpio_isvalid(&host->cd_gpio)) {
		sprintf(str, "sdhci%d_cd", host->index & 0xf);
		gpio_request(host->cd_gpio.gpio, str);
		gpio_direction_input(host->cd_gpio.gpio);
		if (gpio_get_value(host->cd_gpio.gpio))
			return -ENODEV;

		err = exynos_pinmux_config(dev_id, flag);
		if (err) {
			printf("external SD not configured\n");
			return err;
		}
	}

	return s5p_sdhci_core_init(host);
}

static int sdhci_get_config(const void *blob, int node, struct sdhci_host *host)
{
	int bus_width, dev_id;
	unsigned int base;

	/* Get device id */
	dev_id = pinmux_decode_periph_id(blob, node);
	if (dev_id < PERIPH_ID_SDMMC0 && dev_id > PERIPH_ID_SDMMC3) {
		debug("MMC: Can't get device id\n");
		return -1;
	}
	host->index = dev_id - PERIPH_ID_SDMMC0;

	/* Get bus width */
	bus_width = fdtdec_get_int(blob, node, "samsung,bus-width", 0);
	if (bus_width <= 0) {
		debug("MMC: Can't get bus-width\n");
		return -1;
	}
	host->bus_width = bus_width;

	/* Get the base address from the device node */
	base = fdtdec_get_addr(blob, node, "reg");
	if (!base) {
		debug("MMC: Can't get base address\n");
		return -1;
	}
	host->ioaddr = (void *)base;

	fdtdec_decode_gpio(blob, node, "pwr-gpios", &host->pwr_gpio);
	fdtdec_decode_gpio(blob, node, "cd-gpios", &host->cd_gpio);

	return 0;
}

static int process_nodes(const void *blob, int node_list[], int count)
{
	struct sdhci_host *host;
	int i, node;

	debug("%s: count = %d\n", __func__, count);

	/* build sdhci_host[] for each controller */
	for (i = 0; i < count; i++) {
		node = node_list[i];
		if (node <= 0)
			continue;

		host = &sdhci_host[i];

		if (sdhci_get_config(blob, node, host)) {
			printf("%s: failed to decode dev %d\n",	__func__, i);
			return -1;
		}
		do_sdhci_init(host);
	}
	return 0;
}

int exynos_mmc_init(const void *blob)
{
	int count;
	int node_list[SDHCI_MAX_HOSTS];

	count = fdtdec_find_aliases_for_id(blob, "mmc",
			COMPAT_SAMSUNG_EXYNOS_MMC, node_list,
			SDHCI_MAX_HOSTS);

	process_nodes(blob, node_list, count);

	return 1;
}
#endif
