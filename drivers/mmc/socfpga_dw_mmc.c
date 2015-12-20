/*
 * (C) Copyright 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/system_manager.h>
#include <dm.h>
#include <dwmmc.h>
#include <errno.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <linux/err.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_clock_manager *clock_manager_base =
		(void *)SOCFPGA_CLKMGR_ADDRESS;
static const struct socfpga_system_manager *system_manager_base =
		(void *)SOCFPGA_SYSMGR_ADDRESS;

/* socfpga implmentation specific driver private data */
struct dwmci_socfpga_priv_data {
	struct dwmci_host	host;
	unsigned int		drvsel;
	unsigned int		smplsel;
};

static void socfpga_dwmci_clksel(struct dwmci_host *host)
{
	struct dwmci_socfpga_priv_data *priv = host->priv;
	u32 sdmmc_mask = ((priv->smplsel & 0x7) << SYSMGR_SDMMC_SMPLSEL_SHIFT) |
			 ((priv->drvsel & 0x7) << SYSMGR_SDMMC_DRVSEL_SHIFT);

	/* Disable SDMMC clock. */
	clrbits_le32(&clock_manager_base->per_pll.en,
		CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK);

	debug("%s: drvsel %d smplsel %d\n", __func__,
	      priv->drvsel, priv->smplsel);
	writel(sdmmc_mask, &system_manager_base->sdmmcgrp_ctrl);

	debug("%s: SYSMGR_SDMMCGRP_CTRL_REG = 0x%x\n", __func__,
		readl(&system_manager_base->sdmmcgrp_ctrl));

	/* Enable SDMMC clock */
	setbits_le32(&clock_manager_base->per_pll.en,
		CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK);
}

static int socfpga_dwmmc_ofdata_to_platdata(struct udevice *dev)
{
	/* FIXME: probe from DT eventually too/ */
	const unsigned long clk = cm_get_mmc_controller_clk_hz();

	struct dwmci_socfpga_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	int fifo_depth;

	if (clk == 0) {
		printf("DWMMC: MMC clock is zero!");
		return -EINVAL;
	}

	fifo_depth = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				    "fifo-depth", 0);
	if (fifo_depth < 0) {
		printf("DWMMC: Can't get FIFO depth\n");
		return -EINVAL;
	}

	host->name = dev->name;
	host->ioaddr = (void *)dev_get_addr(dev);
	host->buswidth = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					"bus-width", 4);
	host->clksel = socfpga_dwmci_clksel;

	/*
	 * TODO(sjg@chromium.org): Remove the need for this hack.
	 * We only have one dwmmc block on gen5 SoCFPGA.
	 */
	host->dev_index = 0;
	/* Fixed clock divide by 4 which due to the SDMMC wrapper */
	host->bus_hz = clk;
	host->fifoth_val = MSIZE(0x2) |
		RX_WMARK(fifo_depth / 2 - 1) | TX_WMARK(fifo_depth / 2);
	priv->drvsel = fdtdec_get_uint(gd->fdt_blob, dev->of_offset,
				       "drvsel", 3);
	priv->smplsel = fdtdec_get_uint(gd->fdt_blob, dev->of_offset,
					"smplsel", 0);
	host->priv = priv;

	return 0;
}

static int socfpga_dwmmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct dwmci_socfpga_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	int ret;

	ret = add_dwmci(host, host->bus_hz, 400000);
	if (ret)
		return ret;

	upriv->mmc = host->mmc;

	return 0;
}

static const struct udevice_id socfpga_dwmmc_ids[] = {
	{ .compatible = "altr,socfpga-dw-mshc" },
	{ }
};

U_BOOT_DRIVER(socfpga_dwmmc_drv) = {
	.name		= "socfpga_dwmmc",
	.id		= UCLASS_MMC,
	.of_match	= socfpga_dwmmc_ids,
	.ofdata_to_platdata = socfpga_dwmmc_ofdata_to_platdata,
	.probe		= socfpga_dwmmc_probe,
	.priv_auto_alloc_size = sizeof(struct dwmci_socfpga_priv_data),
};
