/*
 * (C) Copyright 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <dwmmc.h>
#include <errno.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/system_manager.h>

static const struct socfpga_clock_manager *clock_manager_base =
		(void *)SOCFPGA_CLKMGR_ADDRESS;
static const struct socfpga_system_manager *system_manager_base =
		(void *)SOCFPGA_SYSMGR_ADDRESS;

static void socfpga_dwmci_clksel(struct dwmci_host *host)
{
	unsigned int drvsel;
	unsigned int smplsel;

	/* Disable SDMMC clock. */
	clrbits_le32(&clock_manager_base->per_pll.en,
		CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK);

	/* Configures drv_sel and smpl_sel */
	drvsel = CONFIG_SOCFPGA_DWMMC_DRVSEL;
	smplsel = CONFIG_SOCFPGA_DWMMC_SMPSEL;

	debug("%s: drvsel %d smplsel %d\n", __func__, drvsel, smplsel);
	writel(SYSMGR_SDMMC_CTRL_SET(smplsel, drvsel),
		&system_manager_base->sdmmcgrp_ctrl);

	debug("%s: SYSMGR_SDMMCGRP_CTRL_REG = 0x%x\n", __func__,
		readl(&system_manager_base->sdmmcgrp_ctrl));

	/* Enable SDMMC clock */
	setbits_le32(&clock_manager_base->per_pll.en,
		CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK);
}

int socfpga_dwmmc_init(u32 regbase, int bus_width, int index)
{
	struct dwmci_host *host;
	unsigned long clk = cm_get_mmc_controller_clk_hz();

	if (clk == 0) {
		printf("%s: MMC clock is zero!", __func__);
		return -EINVAL;
	}

	/* calloc for zero init */
	host = calloc(1, sizeof(struct dwmci_host));
	if (!host) {
		printf("%s: calloc() failed!\n", __func__);
		return -ENOMEM;
	}

	host->name = "SOCFPGA DWMMC";
	host->ioaddr = (void *)regbase;
	host->buswidth = bus_width;
	host->clksel = socfpga_dwmci_clksel;
	host->dev_index = index;
	/* fixed clock divide by 4 which due to the SDMMC wrapper */
	host->bus_hz = clk;
	host->fifoth_val = MSIZE(0x2) |
		RX_WMARK(CONFIG_SOCFPGA_DWMMC_FIFO_DEPTH / 2 - 1) |
		TX_WMARK(CONFIG_SOCFPGA_DWMMC_FIFO_DEPTH / 2);

	return add_dwmci(host, host->bus_hz, 400000);
}

