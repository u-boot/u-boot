// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Amit Singh Tomar <amittomer25@gmail.com>
 *
 * Actions DWMAC specific glue layer
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <clk.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include "designware.h"
#include <asm/arch-owl/regs_s700.h>
#include <linux/bitops.h>

/* pin control for MAC */
#define RMII_TXD01_MFP_CTL0		(0x0 << 16)
#define RMII_RXD01_MFP_CTL0		(0x0 << 8)
#define RMII_TXEN_TXER_MFP_CTL0		(0x0 << 13)
#define RMII_REF_CLK_MFP_CTL0		(0x0 << 6)
#define CLKO_25M_EN_MFP_CTL3		BIT(30)

DECLARE_GLOBAL_DATA_PTR;

static void dwmac_board_setup(void)
{
	clrbits_le32(MFP_CTL0, (RMII_TXD01_MFP_CTL0 | RMII_RXD01_MFP_CTL0 |
		     RMII_TXEN_TXER_MFP_CTL0 | RMII_REF_CLK_MFP_CTL0));

	setbits_le32(MFP_CTL3, CLKO_25M_EN_MFP_CTL3);
}

static int dwmac_s700_probe(struct udevice *dev)
{
	dwmac_board_setup();

	/* This is undocumented, phy interface select register */
	writel(0x4, 0xe024c0a0);

	return designware_eth_probe(dev);
}

static int dwmac_s700_ofdata_to_platdata(struct udevice *dev)
{
	return designware_eth_ofdata_to_platdata(dev);
}

static const struct udevice_id dwmac_s700_ids[] = {
	{.compatible = "actions,s700-ethernet"},
	{ }
};

U_BOOT_DRIVER(dwmac_s700) = {
	.name   = "dwmac_s700",
	.id     = UCLASS_ETH,
	.of_match = dwmac_s700_ids,
	.ofdata_to_platdata = dwmac_s700_ofdata_to_platdata,
	.probe  = dwmac_s700_probe,
	.ops    = &designware_eth_ops,
	.priv_auto_alloc_size = sizeof(struct dw_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
