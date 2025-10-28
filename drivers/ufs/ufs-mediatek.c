// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Loosely based on Linux driver: drivers/ufs/host/ufs-mediatek.c
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <ufs.h>
#include <asm/gpio.h>
#include <reset.h>

#include <linux/arm-smccc.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "ufs.h"
#include "ufs-mediatek.h"
#include "ufs-mediatek-sip.h"

static void ufs_mtk_advertise_quirks(struct ufs_hba *hba)
{
	hba->quirks |= UFSHCI_QUIRK_SKIP_MANUAL_WB_FLUSH_CTRL |
				   UFSHCD_QUIRK_MCQ_BROKEN_INTR |
				   UFSHCD_QUIRK_BROKEN_LSDBS_CAP;
}

static int ufs_mtk_hce_enable_notify(struct ufs_hba *hba,
				     enum ufs_notify_change_status status)
{
	struct ufs_mtk_host *host = dev_get_priv(hba->dev);

	if (status == PRE_CHANGE) {
		if (host->caps & UFS_MTK_CAP_DISABLE_AH8) {
			ufshcd_writel(hba, 0,
				      REG_AUTO_HIBERNATE_IDLE_TIMER);
			hba->capabilities &= ~MASK_AUTO_HIBERN8_SUPPORT;
		}

		/*
		 * Turn on CLK_CG early to bypass abnormal ERR_CHK signal
		 * to prevent host hang issue
		 */
		ufshcd_writel(hba,
			      ufshcd_readl(hba, REG_UFS_XOUFS_CTRL) | 0x80,
			      REG_UFS_XOUFS_CTRL);

		/* DDR_EN setting */
		if (host->ip_ver >= IP_VER_MT6989) {
			ufshcd_rmwl(hba, UFS_MASK(0x7FFF, 8),
				    0x453000, REG_UFS_MMIO_OPT_CTRL_0);
		}
	}

	return 0;
}

static int ufs_mtk_unipro_set_lpm(struct ufs_hba *hba, bool lpm)
{
	int ret;
	struct ufs_mtk_host *host = dev_get_priv(hba->dev);

	ret = ufshcd_dme_set(hba,
			     UIC_ARG_MIB_SEL(VS_UNIPROPOWERDOWNCONTROL, 0),
			     lpm ? 1 : 0);
	if (!ret || !lpm) {
		/*
		 * Forcibly set as non-LPM mode if UIC commands is failed
		 * to use default hba_enable_delay_us value for re-enabling
		 * the host.
		 */
		host->unipro_lpm = lpm;
	}

	return ret;
}

static int ufs_mtk_pre_link(struct ufs_hba *hba)
{
	int ret;
	u32 tmp;

	ret = ufs_mtk_unipro_set_lpm(hba, false);
	if (ret)
		return ret;

	/*
	 * Setting PA_Local_TX_LCC_Enable to 0 before link startup
	 * to make sure that both host and device TX LCC are disabled
	 * once link startup is completed.
	 */
	ret = ufshcd_dme_set(hba, UIC_ARG_MIB(PA_LOCAL_TX_LCC_ENABLE), 0);
	if (ret)
		return ret;

	/* disable deep stall */
	ret = ufshcd_dme_get(hba, UIC_ARG_MIB(VS_SAVEPOWERCONTROL), &tmp);
	if (ret)
		return ret;

	tmp &= ~(1 << 6);

	ret = ufshcd_dme_set(hba, UIC_ARG_MIB(VS_SAVEPOWERCONTROL), tmp);
	if (ret)
		return ret;

	ret = ufshcd_dme_set(hba, UIC_ARG_MIB(PA_SCRAMBLING), tmp);

	return ret;
}

static void ufs_mtk_cfg_unipro_cg(struct ufs_hba *hba, bool enable)
{
	u32 tmp;

	if (enable) {
		ufshcd_dme_get(hba,
			       UIC_ARG_MIB(VS_SAVEPOWERCONTROL), &tmp);
		tmp = tmp |
		      (1 << RX_SYMBOL_CLK_GATE_EN) |
		      (1 << SYS_CLK_GATE_EN) |
		      (1 << TX_CLK_GATE_EN);
		ufshcd_dme_set(hba,
			       UIC_ARG_MIB(VS_SAVEPOWERCONTROL), tmp);

		ufshcd_dme_get(hba,
			       UIC_ARG_MIB(VS_DEBUGCLOCKENABLE), &tmp);
		tmp = tmp & ~(1 << TX_SYMBOL_CLK_REQ_FORCE);
		ufshcd_dme_set(hba,
			       UIC_ARG_MIB(VS_DEBUGCLOCKENABLE), tmp);
	} else {
		ufshcd_dme_get(hba,
			       UIC_ARG_MIB(VS_SAVEPOWERCONTROL), &tmp);
		tmp = tmp & ~((1 << RX_SYMBOL_CLK_GATE_EN) |
			     (1 << SYS_CLK_GATE_EN) |
			     (1 << TX_CLK_GATE_EN));
		ufshcd_dme_set(hba,
			       UIC_ARG_MIB(VS_SAVEPOWERCONTROL), tmp);

		ufshcd_dme_get(hba,
			       UIC_ARG_MIB(VS_DEBUGCLOCKENABLE), &tmp);
		tmp = tmp | (1 << TX_SYMBOL_CLK_REQ_FORCE);
		ufshcd_dme_set(hba,
			       UIC_ARG_MIB(VS_DEBUGCLOCKENABLE), tmp);
	}
}

static void ufs_mtk_post_link(struct ufs_hba *hba)
{
	/* enable unipro clock gating feature */
	ufs_mtk_cfg_unipro_cg(hba, true);
}

static int ufs_mtk_link_startup_notify(struct ufs_hba *hba,
				       enum ufs_notify_change_status status)
{
	int ret = 0;

	switch (status) {
	case PRE_CHANGE:
			ret = ufs_mtk_pre_link(hba);
			break;
	case POST_CHANGE:
			ufs_mtk_post_link(hba);
			break;
	default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

static int ufs_mtk_bind_mphy(struct ufs_hba *hba)
{
	struct ufs_mtk_host *host = dev_get_priv(hba->dev);
	int err = 0;

	err = generic_phy_get_by_index(hba->dev, 0, host->mphy);

	if (IS_ERR(host->mphy)) {
		err = PTR_ERR(host->mphy);
		if (err != -ENODEV) {
			dev_info(hba->dev, "%s: Could NOT get a valid PHY %d\n", __func__,
				 err);
		}
	}

	if (err)
		host->mphy = NULL;

	return err;
}

static void ufs_mtk_init_reset_control(struct ufs_hba *hba,
				       struct reset_ctl **rc,
				       char *str)
{
	*rc = devm_reset_control_get(hba->dev, str);
	if (IS_ERR(*rc)) {
		dev_info(hba->dev, "Failed to get reset control %s: %ld\n",
			 str, PTR_ERR(*rc));
		*rc = NULL;
	}
}

static void ufs_mtk_init_reset(struct ufs_hba *hba)
{
	struct ufs_mtk_host *host = dev_get_priv(hba->dev);

	ufs_mtk_init_reset_control(hba, &host->hci_reset,
				   "hci_rst");
	ufs_mtk_init_reset_control(hba, &host->unipro_reset,
				   "unipro_rst");
	ufs_mtk_init_reset_control(hba, &host->crypto_reset,
				   "crypto_rst");
}

static void ufs_mtk_get_hw_ip_version(struct ufs_hba *hba)
{
	struct ufs_mtk_host *host = dev_get_priv(hba->dev);
	u32 hw_ip_ver;

	hw_ip_ver = ufshcd_readl(hba, REG_UFS_MTK_IP_VER);

	if (((hw_ip_ver & (0xFF << 24)) == (0x1 << 24)) ||
	    ((hw_ip_ver & (0xFF << 24)) == 0)) {
		hw_ip_ver &= ~(0xFF << 24);
		hw_ip_ver |= (0x1 << 28);
	}

	host->ip_ver = hw_ip_ver;

	dev_info(hba->dev, "MediaTek UFS IP Version: 0x%x\n", hw_ip_ver);
}

static int ufs_mtk_setup_ref_clk(struct ufs_hba *hba, bool on)
{
	struct ufs_mtk_host *host = dev_get_priv(hba->dev);
	struct arm_smccc_res res;
	int timeout, time_checked = 0;
	u32 value;

	if (host->ref_clk_enabled == on)
		return 0;

	ufs_mtk_ref_clk_notify(on, PRE_CHANGE, res);

	if (on) {
		ufshcd_writel(hba, REFCLK_REQUEST, REG_UFS_REFCLK_CTRL);
	} else {
		udelay(10);
		ufshcd_writel(hba, REFCLK_RELEASE, REG_UFS_REFCLK_CTRL);
	}

	/* Wait for ack */
	timeout = REFCLK_REQ_TIMEOUT_US;
	do {
		value = ufshcd_readl(hba, REG_UFS_REFCLK_CTRL);

		/* Wait until ack bit equals to req bit */
		if (((value & REFCLK_ACK) >> 1) == (value & REFCLK_REQUEST))
			goto out;

		udelay(200);
		time_checked += 200;
	} while (time_checked != timeout);

	dev_err(hba->dev, "missing ack of refclk req, reg: 0x%x\n", value);

	/*
	 * If clock on timeout, assume clock is off, notify tfa do clock
	 * off setting.(keep DIFN disable, release resource)
	 * If clock off timeout, assume clock will off finally,
	 * set ref_clk_enabled directly.(keep DIFN disable, keep resource)
	 */
	if (on)
		ufs_mtk_ref_clk_notify(false, POST_CHANGE, res);
	else
		host->ref_clk_enabled = false;

	return -ETIMEDOUT;

out:
	host->ref_clk_enabled = on;
	if (on)
		udelay(10);

	ufs_mtk_ref_clk_notify(on, POST_CHANGE, res);

	return 0;
}

/**
 * ufs_mtk_init - bind phy with controller
 * @hba: host controller instance
 *
 * Powers up PHY enabling clocks and regulators.
 *
 * Returns -ENODEV if binding fails, returns negative error
 * on phy power up failure and returns zero on success.
 */
static int ufs_mtk_init(struct ufs_hba *hba)
{
	struct ufs_mtk_host *priv = dev_get_priv(hba->dev);
	int err;

	priv->hba = hba;

	err = ufs_mtk_bind_mphy(hba);
	if (err)
		return -ENODEV;

	ufs_mtk_advertise_quirks(hba);

	ufs_mtk_init_reset(hba);

	// TODO: Clocking

	err = generic_phy_power_on(priv->mphy);
	if (err) {
		dev_err(hba->dev, "%s: phy init failed, err = %d\n",
			__func__, err);
		return err;
	}

	ufs_mtk_setup_ref_clk(hba, true);
	ufs_mtk_get_hw_ip_version(hba);

	return 0;
}

static int ufs_mtk_device_reset(struct ufs_hba *hba)
{
	struct arm_smccc_res res;

	ufs_mtk_device_reset_ctrl(0, res);

	/*
	 * The reset signal is active low. UFS devices shall detect
	 * more than or equal to 1us of positive or negative RST_n
	 * pulse width.
	 *
	 * To be on safe side, keep the reset low for at least 10us.
	 */
	udelay(13);

	ufs_mtk_device_reset_ctrl(1, res);

	/* Some devices may need time to respond to rst_n */
	mdelay(13);

	dev_dbg(hba->dev, "device reset done\n");

	return 0;
}

static struct ufs_hba_ops ufs_mtk_hba_ops = {
	.init			= ufs_mtk_init,
	.hce_enable_notify	= ufs_mtk_hce_enable_notify,
	.link_startup_notify	= ufs_mtk_link_startup_notify,
	.device_reset		= ufs_mtk_device_reset,
};

static int ufs_mtk_probe(struct udevice *dev)
{
	int ret;

	ret = ufshcd_probe(dev, &ufs_mtk_hba_ops);
	if (ret) {
		dev_err(dev, "ufshcd_probe() failed, ret:%d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id ufs_mtk_ids[] = {
	{ .compatible = "mediatek,mt6878-ufshci" },
	{},
};

U_BOOT_DRIVER(mediatek_ufshci) = {
	.name		= "mediatek-ufshci",
	.id		= UCLASS_UFS,
	.of_match	= ufs_mtk_ids,
	.probe		= ufs_mtk_probe,
	.priv_auto	= sizeof(struct ufs_mtk_host),
};
