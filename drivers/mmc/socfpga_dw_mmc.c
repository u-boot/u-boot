/*
 * (C) Copyright 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <fdtdec.h>
#include <libfdt.h>
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

static int socfpga_dwmci_of_probe(const void *blob, int node, const int idx)
{
	/* FIXME: probe from DT eventually too/ */
	const unsigned long clk = cm_get_mmc_controller_clk_hz();

	struct dwmci_host *host;
	fdt_addr_t reg_base;
	int bus_width, fifo_depth;

	if (clk == 0) {
		printf("DWMMC%d: MMC clock is zero!", idx);
		return -EINVAL;
	}

	/* Get the register address from the device node */
	reg_base = fdtdec_get_addr(blob, node, "reg");
	if (!reg_base) {
		printf("DWMMC%d: Can't get base address\n", idx);
		return -EINVAL;
	}

	/* Get the bus width from the device node */
	bus_width = fdtdec_get_int(blob, node, "bus-width", 0);
	if (bus_width <= 0) {
		printf("DWMMC%d: Can't get bus-width\n", idx);
		return -EINVAL;
	}

	fifo_depth = fdtdec_get_int(blob, node, "fifo-depth", 0);
	if (fifo_depth < 0) {
		printf("DWMMC%d: Can't get FIFO depth\n", idx);
		return -EINVAL;
	}

	/* Allocate the host */
	host = calloc(1, sizeof(*host));
	if (!host)
		return -ENOMEM;

	host->name = "SOCFPGA DWMMC";
	host->ioaddr = (void *)reg_base;
	host->buswidth = bus_width;
	host->clksel = socfpga_dwmci_clksel;
	host->dev_index = idx;
	/* Fixed clock divide by 4 which due to the SDMMC wrapper */
	host->bus_hz = clk;
	host->fifoth_val = MSIZE(0x2) |
		RX_WMARK(fifo_depth / 2 - 1) | TX_WMARK(fifo_depth / 2);

	return add_dwmci(host, host->bus_hz, 400000);
}

static int socfpga_dwmci_process_node(const void *blob, int nodes[],
				      int count)
{
	int i, node, ret;

	for (i = 0; i < count; i++) {
		node = nodes[i];
		if (node <= 0)
			continue;

		ret = socfpga_dwmci_of_probe(blob, node, i);
		if (ret) {
			printf("%s: failed to decode dev %d\n", __func__, i);
			return ret;
		}
	}
	return 0;
}

int socfpga_dwmmc_init(const void *blob)
{
	int nodes[2];	/* Max. two controllers. */
	int ret, count;

	count = fdtdec_find_aliases_for_id(blob, "mmc",
					   COMPAT_ALTERA_SOCFPGA_DWMMC,
					   nodes, ARRAY_SIZE(nodes));

	ret = socfpga_dwmci_process_node(blob, nodes, count);

	return ret;
}
