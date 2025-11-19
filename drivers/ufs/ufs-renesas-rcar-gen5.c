// SPDX-License-Identifier: GPL-2.0-only
/*
 * Renesas UFS host controller driver
 *
 * Copyright (C) 2025 Renesas Electronics Corporation
 */

#include <clk.h>
#include <dm.h>
#include <ufs.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/iopoll.h>
#include <reset.h>

#include "ufs.h"

#define UFS_RENESAS_TIMEOUT_US		100000

struct ufs_renesas_priv {
	struct clk_bulk		clks;
	struct reset_ctl_bulk	resets;
	fdt_addr_t		phy_base;
	/* The hardware needs initialization once */
	bool			initialized;
};

static void ufs_dme_command(struct ufs_hba *hba, u32 cmd,
			    u32 arg1, u32 arg2, u32 arg3)
{
	ufshcd_writel(hba, arg1, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, arg2, REG_UIC_COMMAND_ARG_2);
	ufshcd_writel(hba, arg3, REG_UIC_COMMAND_ARG_3);
	ufshcd_writel(hba, cmd, REG_UIC_COMMAND);
}

static int ufs_renesas_pre_init(struct ufs_hba *hba)
{
	struct ufs_renesas_priv *priv = dev_get_priv(hba->dev);
	u32 val32;
	u16 val16;
	int ret;

	writew(0x0001, priv->phy_base + 0x20000);
	writew(0x005c, priv->phy_base + 0x20212);
	writew(0x005c, priv->phy_base + 0x20214);
	writew(0x005c, priv->phy_base + 0x20216);
	writew(0x005c, priv->phy_base + 0x20218);
	writew(0x036a, priv->phy_base + 0x201d0);
	writew(0x0102, priv->phy_base + 0x201d2);
	writew(0x001f, priv->phy_base + 0x20082);
	writew(0x000b, priv->phy_base + 0x20084);
	writew(0x0126, priv->phy_base + 0x201d2);
	writew(0x01dc, priv->phy_base + 0x20214);
	writew(0x01dc, priv->phy_base + 0x20218);
	writew(0x0000, priv->phy_base + 0x201cc);
	writew(0x0200, priv->phy_base + 0x201ce);
	writew(0x0000, priv->phy_base + 0x20212);
	writew(0x0000, priv->phy_base + 0x20216);

	ret = readw_poll_timeout(priv->phy_base + 0x201ec, val16,
				 !(val16 & BIT(12)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readw_poll_timeout(priv->phy_base + 0x201e4, val16,
				 !(val16 & BIT(12)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readw_poll_timeout(priv->phy_base + 0x201f0, val16,
				 !(val16 & BIT(12)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	writew(0x0000, priv->phy_base + 0x20000);

	ufshcd_writel(hba, BIT(0), REG_CONTROLLER_ENABLE);

	ret = read_poll_timeout(ufshcd_readl, val32, (val32 & BIT(0)),
				1, UFS_RENESAS_TIMEOUT_US,
				hba, REG_CONTROLLER_ENABLE);
	if (ret)
		return ret;

	ret = read_poll_timeout(ufshcd_readl, val32, (val32 & BIT(3)),
				1, UFS_RENESAS_TIMEOUT_US,
				hba, REG_CONTROLLER_STATUS);
	if (ret)
		return ret;

	/* Skip IE because we cannot handle interrupts here */
	ufs_dme_command(hba, 0x00000002, 0x81010000, 0x00000000, 0x00000005);
	ufs_dme_command(hba, 0x00000002, 0x81150000, 0x00000000, 0x00000001);
	ufs_dme_command(hba, 0x00000002, 0x81180000, 0x00000000, 0x00000001);
	ufs_dme_command(hba, 0x00000002, 0x80090000, 0x00000000, 0x00000000);
	ufs_dme_command(hba, 0x00000002, 0x800a0000, 0x00000000, 0x000000c8);
	ufs_dme_command(hba, 0x00000002, 0x80090001, 0x00000000, 0x00000000);
	ufs_dme_command(hba, 0x00000002, 0x800a0001, 0x00000000, 0x000000c8);
	ufs_dme_command(hba, 0x00000002, 0x800a0004, 0x00000000, 0x00000000);
	ufs_dme_command(hba, 0x00000002, 0x800b0004, 0x00000000, 0x00000064);
	ufs_dme_command(hba, 0x00000002, 0x800a0005, 0x00000000, 0x00000000);
	ufs_dme_command(hba, 0x00000002, 0x800b0005, 0x00000000, 0x00000064);
	ufs_dme_command(hba, 0x00000002, 0xd0850000, 0x00000000, 0x00000001);

	writew(0x0001, priv->phy_base + 0x20000);

	clrbits_le16(priv->phy_base + 0x20022, BIT(0));

	ret = readw_poll_timeout(priv->phy_base + (0x00198 << 1), val16,
				 (val16 & BIT(0)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	writew(0x0368, priv->phy_base + 0x201d0);

	ret = readw_poll_timeout(priv->phy_base + 0x201e4, val16,
				 !(val16 & BIT(11)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readw_poll_timeout(priv->phy_base + 0x201e8, val16,
				 !(val16 & BIT(11)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readw_poll_timeout(priv->phy_base + 0x201ec, val16,
				 !(val16 & BIT(11)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readw_poll_timeout(priv->phy_base + 0x201f0, val16,
				 !(val16 & BIT(11)), UFS_RENESAS_TIMEOUT_US);
	if (ret)
		return ret;

	priv->initialized = true;

	return 0;
}

static int ufs_renesas_init(struct ufs_hba *hba)
{
	hba->quirks |= UFSHCD_QUIRK_BROKEN_64BIT_ADDRESS | UFSHCD_QUIRK_HIBERN_FASTAUTO |
		       UFSHCD_QUIRK_BROKEN_LCC;

	return 0;
}

static int ufs_renesas_hce_enable_notify(struct ufs_hba *hba,
					 enum ufs_notify_change_status status)
{
	struct ufs_renesas_priv *priv = dev_get_priv(hba->dev);
	int ret;

	if (priv->initialized)
		return 0;

	if (status == PRE_CHANGE) {
		ret = ufs_renesas_pre_init(hba);
		if (ret)
			return ret;
	}

	priv->initialized = true;

	return 0;
}

static int ufs_renesas_link_startup_notify(struct ufs_hba *hba,
					   enum ufs_notify_change_status status)
{
	if (status == PRE_CHANGE)
		return ufshcd_dme_set(hba, UIC_ARG_MIB(PA_LOCAL_TX_LCC_ENABLE), 0);

	return 0;
}

static int ufs_renesas_get_max_pwr_mode(struct ufs_hba *hba,
					struct ufs_pwr_mode_info *max_pwr_info)
{
	max_pwr_info->info.gear_rx = UFS_HS_G5;
	max_pwr_info->info.gear_tx = UFS_HS_G5;
	max_pwr_info->info.pwr_tx = FASTAUTO_MODE;
	max_pwr_info->info.pwr_rx = FASTAUTO_MODE;
	max_pwr_info->info.hs_rate = PA_HS_MODE_A;

	max_pwr_info->info.lane_rx = 1;
	max_pwr_info->info.lane_tx = 1;

	dev_info(hba->dev, "Max HS Gear: %d\n", max_pwr_info->info.gear_rx);

	return 0;
}

static struct ufs_hba_ops ufs_renesas_vops = {
	.init			= ufs_renesas_init,
	.hce_enable_notify	= ufs_renesas_hce_enable_notify,
	.link_startup_notify	= ufs_renesas_link_startup_notify,
	.get_max_pwr_mode	= ufs_renesas_get_max_pwr_mode,
};

static int ufs_renesas_pltfm_probe(struct udevice *dev)
{
	struct ufs_renesas_priv *priv = dev_get_priv(dev);
	int ret;

	priv->phy_base = dev_read_addr_name(dev, "phy");
	if (priv->phy_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret < 0)
		return ret;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret < 0)
		goto err_clk_get;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		goto err_clk_enable;

	reset_assert_bulk(&priv->resets);
	reset_deassert_bulk(&priv->resets);

	ret = ufshcd_probe(dev, &ufs_renesas_vops);
	if (ret) {
		dev_err(dev, "ufshcd_probe() failed %d\n", ret);
		goto err_ufshcd_probe;
	}

	return 0;

err_ufshcd_probe:
	reset_assert_bulk(&priv->resets);
	clk_disable_bulk(&priv->clks);
err_clk_enable:
	clk_release_bulk(&priv->clks);
err_clk_get:
	reset_release_bulk(&priv->resets);
	return ret;
}

static int ufs_renesas_pltfm_remove(struct udevice *dev)
{
	struct ufs_renesas_priv *priv = dev_get_priv(dev);

	reset_release_bulk(&priv->resets);
	clk_disable_bulk(&priv->clks);
	clk_release_bulk(&priv->clks);

	return 0;
}

static const struct udevice_id ufs_renesas_pltfm_ids[] = {
	{ .compatible = "renesas,r8a78000-ufs" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ufs_renesas_gen5) = {
	.name		= "ufs-renesas-gen5",
	.id		= UCLASS_UFS,
	.of_match	= ufs_renesas_pltfm_ids,
	.probe		= ufs_renesas_pltfm_probe,
	.remove		= ufs_renesas_pltfm_remove,
	.priv_auto	= sizeof(struct ufs_renesas_priv),
};
