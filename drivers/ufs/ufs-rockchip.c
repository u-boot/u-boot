// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip UFS Host Controller driver
 *
 * Copyright (C) 2025 Rockchip Electronics Co.Ltd.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <reset.h>
#include <ufs.h>

#include "ufs.h"
#include "unipro.h"
#include "ufs-rockchip.h"

static int ufs_rockchip_hce_enable_notify(struct ufs_hba *hba,
					  enum ufs_notify_change_status status)
{
	int err = 0;

	if (status != POST_CHANGE)
		return 0;

	ufshcd_dme_reset(hba);
	ufshcd_dme_enable(hba);

	if (hba->ops->phy_initialization) {
		err = hba->ops->phy_initialization(hba);
		if (err)
			dev_err(hba->dev,
				"Phy init failed (%d)\n", err);
	}

	return err;
}

static void ufs_rockchip_rk3576_phy_parameter_init(struct ufs_hba *hba)
{
	struct ufs_rockchip_host *host = dev_get_priv(hba->dev);

	ufs_sys_writel(host->mphy_base, 0x80, CMN_REG23);
	ufs_sys_writel(host->mphy_base, 0xB5, TRSV0_REG14);
	ufs_sys_writel(host->mphy_base, 0xB5, TRSV1_REG14);
	ufs_sys_writel(host->mphy_base, 0x03, TRSV0_REG15);
	ufs_sys_writel(host->mphy_base, 0x03, TRSV1_REG15);
	ufs_sys_writel(host->mphy_base, 0x38, TRSV0_REG08);
	ufs_sys_writel(host->mphy_base, 0x38, TRSV1_REG08);
	ufs_sys_writel(host->mphy_base, 0x50, TRSV0_REG29);
	ufs_sys_writel(host->mphy_base, 0x50, TRSV1_REG29);
	ufs_sys_writel(host->mphy_base, 0x80, TRSV0_REG2E);
	ufs_sys_writel(host->mphy_base, 0x80, TRSV1_REG2E);
	ufs_sys_writel(host->mphy_base, 0x18, TRSV0_REG3C);
	ufs_sys_writel(host->mphy_base, 0x18, TRSV1_REG3C);
	ufs_sys_writel(host->mphy_base, 0x03, TRSV0_REG16);
	ufs_sys_writel(host->mphy_base, 0x03, TRSV1_REG16);
	ufs_sys_writel(host->mphy_base, 0x20, TRSV0_REG17);
	ufs_sys_writel(host->mphy_base, 0x20, TRSV1_REG17);
	ufs_sys_writel(host->mphy_base, 0xC0, TRSV0_REG18);
	ufs_sys_writel(host->mphy_base, 0xC0, TRSV1_REG18);
	ufs_sys_writel(host->mphy_base, 0x03, CMN_REG25);
	ufs_sys_writel(host->mphy_base, 0x03, TRSV0_REG3D);
	ufs_sys_writel(host->mphy_base, 0x03, TRSV1_REG3D);
	ufs_sys_writel(host->mphy_base, 0xC0, CMN_REG23);
	udelay(1);
	ufs_sys_writel(host->mphy_base, 0x00, CMN_REG23);
	udelay(200);
}

static int ufs_rockchip_rk3576_phy_init(struct ufs_hba *hba)
{
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_LOCAL_TX_LCC_ENABLE, 0x0), 0x0);
	/* enable the mphy DME_SET cfg */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MPHY_CFG, 0x0), MPHY_CFG_ENABLE);
	for (int i = 0; i < 2; i++) {
		/* Configuration M-TX */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_CLK_PRD, SEL_TX_LANE0 + i), 0x06);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_CLK_PRD_EN, SEL_TX_LANE0 + i), 0x02);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_LINERESET_VALUE, SEL_TX_LANE0 + i), 0x44);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_LINERESET_PVALUE1, SEL_TX_LANE0 + i), 0xe6);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_LINERESET_PVALUE2, SEL_TX_LANE0 + i), 0x07);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_TASE_VALUE, SEL_TX_LANE0 + i), 0x93);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_BASE_NVALUE, SEL_TX_LANE0 + i), 0xc9);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_TX_POWER_SAVING_CTRL, SEL_TX_LANE0 + i), 0x00);
		/* Configuration M-RX */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_CLK_PRD, SEL_RX_LANE0 + i), 0x06);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_CLK_PRD_EN, SEL_RX_LANE0 + i), 0x00);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_LINERESET_VALUE, SEL_RX_LANE0 + i), 0x58);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_LINERESET_PVALUE1, SEL_RX_LANE0 + i), 0x8c);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_LINERESET_PVALUE2, SEL_RX_LANE0 + i), 0x02);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_LINERESET_OPTION, SEL_RX_LANE0 + i), 0xf6);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_POWER_SAVING_CTRL, SEL_RX_LANE0 + i), 0x69);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VND_RX_SAVE_DET_CTRL, SEL_RX_LANE0 + i), 0x18);
	}

	/* disable the mphy DME_SET cfg */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MPHY_CFG, 0x0), MPHY_CFG_DISABLE);

	ufs_rockchip_rk3576_phy_parameter_init(hba);

	/* start link up */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MIB_T_DBG_CPORT_TX_ENDIAN, 0), 0x0);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MIB_T_DBG_CPORT_RX_ENDIAN, 0), 0x0);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(N_DEVICEID, 0), 0x0);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(N_DEVICEID_VALID, 0), 0x1);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(T_PEERDEVICEID, 0), 0x1);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(T_CONNECTIONSTATE, 0), 0x1);

	return 0;
}

static int ufs_rockchip_common_init(struct ufs_hba *hba)
{
	struct udevice *dev = hba->dev;
	struct ufs_rockchip_host *host = dev_get_priv(dev);
	struct resource res;
	int err;

	/* system control register for hci */
	err = dev_read_resource_byname(dev, "hci_grf", &res);
	if (err) {
		dev_err(dev, "cannot ioremap for hci system control register\n");
		return err;
	}
	host->ufs_sys_ctrl = (void *)(res.start);

	/* system control register for mphy */
	err = dev_read_resource_byname(dev, "mphy_grf", &res);
	if (err) {
		dev_err(dev, "cannot ioremap for mphy system control register\n");
		return err;
	}
	host->ufs_phy_ctrl = (void *)(res.start);

	/* mphy base register */
	err = dev_read_resource_byname(dev, "mphy", &res);
	if (err) {
		dev_err(dev, "cannot ioremap for mphy base register\n");
		return err;
	}
	host->mphy_base = (void *)(res.start);

	host->phy_config_mode = dev_read_u32_default(dev, "ufs-phy-config-mode", 0);

	err = reset_get_bulk(dev, &host->rsts);
	if (err) {
		dev_err(dev, "Can't get reset: %d\n", err);
		return err;
	}

	host->hba = hba;

	return 0;
}

static int ufs_rockchip_rk3576_init(struct ufs_hba *hba)
{
	int ret = 0;

	ret = ufs_rockchip_common_init(hba);
	if (ret) {
		dev_err(hba->dev, "%s: ufs common init fail\n", __func__);
		return ret;
	}

	return 0;
}

static struct ufs_hba_ops ufs_hba_rk3576_vops = {
	.init = ufs_rockchip_rk3576_init,
	.phy_initialization = ufs_rockchip_rk3576_phy_init,
	.hce_enable_notify = ufs_rockchip_hce_enable_notify,
};

static const struct udevice_id ufs_rockchip_of_match[] = {
	{ .compatible = "rockchip,rk3576-ufshc", .data = (ulong)&ufs_hba_rk3576_vops},
	{},
};

static int ufs_rockchip_probe(struct udevice *dev)
{
	struct ufs_hba_ops *ops = (struct ufs_hba_ops *)dev_get_driver_data(dev);
	int err;

	err = ufshcd_probe(dev, ops);
	if (err)
		dev_err(dev, "ufshcd_pltfrm_init() failed %d\n", err);

	return err;
}

U_BOOT_DRIVER(rockchip_ufs) = {
	.name		= "ufshcd-rockchip",
	.id		= UCLASS_UFS,
	.of_match	= ufs_rockchip_of_match,
	.probe		= ufs_rockchip_probe,
	.priv_auto	= sizeof(struct ufs_rockchip_host),
};
