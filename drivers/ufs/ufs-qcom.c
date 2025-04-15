// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2013-2016, Linux Foundation. All rights reserved.
 * Copyright (C) 2023-2024 Linaro Limited
 * Authors:
 * - Bhupesh Sharma <bhupesh.sharma@linaro.org>
 * - Neil Armstrong <neil.armstrong@linaro.org>
 *
 * Based on Linux driver
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <ufs.h>
#include <asm/gpio.h>

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "ufs.h"
#include "ufs-qcom.h"

#define ceil(freq, div) ((freq) % (div) == 0 ? ((freq) / (div)) : ((freq) / (div) + 1))

static void ufs_qcom_dev_ref_clk_ctrl(struct ufs_hba *hba, bool enable);

static int ufs_qcom_enable_clks(struct ufs_qcom_priv *priv)
{
	int err;

	if (priv->is_clks_enabled)
		return 0;

	err = clk_enable_bulk(&priv->clks);
	if (err)
		return err;

	priv->is_clks_enabled = true;

	return 0;
}

static int ufs_qcom_init_clks(struct ufs_qcom_priv *priv)
{
	int err;
	struct udevice *dev = priv->hba->dev;

	err = clk_get_bulk(dev, &priv->clks);
	if (err)
		return err;

	return 0;
}

static int ufs_qcom_check_hibern8(struct ufs_hba *hba)
{
	int err, retry_count = 50;
	u32 tx_fsm_val = 0;

	do {
		err = ufshcd_dme_get(hba,
				UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE,
					UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
				&tx_fsm_val);
		if (err || tx_fsm_val == TX_FSM_HIBERN8)
			break;

		/* max. 200us */
		udelay(200);
		retry_count--;
	} while (retry_count != 0);

	/* Check the state again */
	err = ufshcd_dme_get(hba,
			UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE,
				UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
				&tx_fsm_val);

	if (err) {
		dev_err(hba->dev, "%s: unable to get TX_FSM_STATE, err %d\n",
			__func__, err);
	} else if (tx_fsm_val != TX_FSM_HIBERN8) {
		err = tx_fsm_val;
		dev_err(hba->dev, "%s: invalid TX_FSM_STATE = %d\n",
			__func__, err);
	}

	return err;
}

static void ufs_qcom_select_unipro_mode(struct ufs_qcom_priv *priv)
{
	ufshcd_rmwl(priv->hba, QUNIPRO_SEL, QUNIPRO_SEL, REG_UFS_CFG1);

	if (priv->hw_ver.major >= 0x05)
		ufshcd_rmwl(priv->hba, QUNIPRO_G4_SEL, 0, REG_UFS_CFG0);
}

/*
 * ufs_qcom_reset - reset host controller and PHY
 */
static int ufs_qcom_reset(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	int ret;

	ret = reset_assert(&priv->core_reset);
	if (ret) {
		dev_err(hba->dev, "%s: core_reset assert failed, err = %d\n",
			__func__, ret);
		return ret;
	}

	/*
	 * The hardware requirement for delay between assert/deassert
	 * is at least 3-4 sleep clock (32.7KHz) cycles, which comes to
	 * ~125us (4/32768). To be on the safe side add 200us delay.
	 */
	udelay(210);

	ret = reset_deassert(&priv->core_reset);
	if (ret)
		dev_err(hba->dev, "%s: core_reset deassert failed, err = %d\n",
			__func__, ret);

	udelay(1100);

	return 0;
}

/**
 * ufs_qcom_advertise_quirks - advertise the known QCOM UFS controller quirks
 * @hba: host controller instance
 *
 * QCOM UFS host controller might have some non standard behaviours (quirks)
 * than what is specified by UFSHCI specification. Advertise all such
 * quirks to standard UFS host controller driver so standard takes them into
 * account.
 */
static void ufs_qcom_advertise_quirks(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);

	if (priv->hw_ver.major == 0x2)
		hba->quirks |= UFSHCD_QUIRK_BROKEN_UFS_HCI_VERSION;

	if (priv->hw_ver.major > 0x3)
		hba->quirks |= UFSHCD_QUIRK_REINIT_AFTER_MAX_GEAR_SWITCH;
}

/**
 * ufs_qcom_setup_clocks - enables/disable clocks
 * @hba: host controller instance
 * @on: If true, enable clocks else disable them.
 * @status: PRE_CHANGE or POST_CHANGE notify
 *
 * Returns 0 on success, non-zero on failure.
 */
static int ufs_qcom_setup_clocks(struct ufs_hba *hba, bool on,
				 enum ufs_notify_change_status status)
{
	switch (status) {
	case PRE_CHANGE:
		if (!on)
			/* disable device ref_clk */
			ufs_qcom_dev_ref_clk_ctrl(hba, false);
		break;
	case POST_CHANGE:
		if (on)
			/* enable the device ref clock for HS mode*/
			ufs_qcom_dev_ref_clk_ctrl(hba, true);
		break;
	}

	return 0;
}

static u32 ufs_qcom_get_hs_gear(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);

	/*
	 * TOFIX: v4 controllers *should* be able to support HS Gear 4
	 * but so far pwr_mode switch is failing on v4 controllers and HS Gear 4.
	 * only enable HS Gear > 3 for Controlers major version 5 and later.
	 */
	if (priv->hw_ver.major > 0x4)
		return UFS_QCOM_MAX_GEAR(ufshcd_readl(hba, REG_UFS_PARAM0));

	/* Default is HS-G3 */
	return UFS_HS_G3;
}

static int ufs_get_max_pwr_mode(struct ufs_hba *hba,
				struct ufs_pwr_mode_info *max_pwr_info)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	u32 max_gear = ufs_qcom_get_hs_gear(hba);

	max_pwr_info->info.gear_rx = min(max_pwr_info->info.gear_rx, max_gear);
	/* Qualcomm UFS only support symmetric Gear */
	max_pwr_info->info.gear_tx = max_pwr_info->info.gear_rx;

	if (priv->hw_ver.major >= 0x4 && max_pwr_info->info.gear_rx > UFS_HS_G3)
		ufshcd_dme_set(hba,
			       UIC_ARG_MIB(PA_TXHSADAPTTYPE),
			       PA_INITIAL_ADAPT);

	dev_info(hba->dev, "Max HS Gear: %d\n", max_pwr_info->info.gear_rx);

	return 0;
}

static int ufs_qcom_power_up_sequence(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	struct phy phy;
	int ret;

	/* Reset UFS Host Controller and PHY */
	ret = ufs_qcom_reset(hba);
	if (ret)
		dev_warn(hba->dev, "%s: host reset returned %d\n",
			 __func__, ret);

	/* get phy */
	ret = generic_phy_get_by_name(hba->dev, "ufsphy", &phy);
	if (ret) {
		dev_warn(hba->dev, "%s: Unable to get QMP ufs phy, ret = %d\n",
			 __func__, ret);
		return ret;
	}

	/* phy initialization */
	ret = generic_phy_init(&phy);
	if (ret) {
		dev_err(hba->dev, "%s: phy init failed, ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* power on phy */
	ret = generic_phy_power_on(&phy);
	if (ret) {
		dev_err(hba->dev, "%s: phy power on failed, ret = %d\n",
			__func__, ret);
		goto out_disable_phy;
	}

	ufs_qcom_select_unipro_mode(priv);

	return 0;

out_disable_phy:
	generic_phy_exit(&phy);

	return ret;
}

/*
 * The UTP controller has a number of internal clock gating cells (CGCs).
 * Internal hardware sub-modules within the UTP controller control the CGCs.
 * Hardware CGCs disable the clock to inactivate UTP sub-modules not involved
 * in a specific operation, UTP controller CGCs are by default disabled and
 * this function enables them (after every UFS link startup) to save some power
 * leakage.
 */
static void ufs_qcom_enable_hw_clk_gating(struct ufs_hba *hba)
{
	ufshcd_rmwl(hba, REG_UFS_CFG2_CGC_EN_ALL, REG_UFS_CFG2_CGC_EN_ALL,
		    REG_UFS_CFG2);

	/* Ensure that HW clock gating is enabled before next operations */
	ufshcd_readl(hba, REG_UFS_CFG2);
}

static int ufs_qcom_hce_enable_notify(struct ufs_hba *hba,
				      enum ufs_notify_change_status status)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	int err;

	switch (status) {
	case PRE_CHANGE:
		ufs_qcom_power_up_sequence(hba);
		/*
		 * The PHY PLL output is the source of tx/rx lane symbol
		 * clocks, hence, enable the lane clocks only after PHY
		 * is initialized.
		 */
		err = ufs_qcom_enable_clks(priv);
		break;
	case POST_CHANGE:
		/* check if UFS PHY moved from DISABLED to HIBERN8 */
		err = ufs_qcom_check_hibern8(hba);
		ufs_qcom_enable_hw_clk_gating(hba);
		break;
	default:
		dev_err(hba->dev, "%s: invalid status %d\n", __func__, status);
		err = -EINVAL;
		break;
	}

	return err;
}

/* Look for the maximum core_clk_unipro clock value */
static u32 ufs_qcom_get_core_clk_unipro_max_freq(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	ofnode node = dev_ofnode(priv->hba->dev);
	struct ofnode_phandle_args opp_table;
	int pos, ret;
	u32 clk = 0;

	/* Get core_clk_unipro clock index */
	pos = ofnode_stringlist_search(node, "clock-names", "core_clk_unipro");
	if (pos < 0)
		goto fallback;

	/* Try parsing the opps */
	if (!ofnode_parse_phandle_with_args(node, "required-opps",
					    NULL, 0, 0, &opp_table) &&
	    ofnode_device_is_compatible(opp_table.node, "operating-points-v2")) {
		ofnode opp_node;

		ofnode_for_each_subnode(opp_node, opp_table.node) {
			u64 opp_clk;
			/* opp-hw contains the OPP frequency */
			ret = ofnode_read_u64_index(opp_node, "opp-hz", pos, &opp_clk);
			if (ret)
				continue;

			/* We don't handle larger clock values, ignore */
			if (opp_clk > U32_MAX)
				continue;

			/* Only keep the largest value */
			if (opp_clk > clk)
				clk = opp_clk;
		}

		/* If we get a valid clock, return it or check legacy*/
		if (clk)
			return clk;
	}

	/* Legacy freq-table-hz has a pair of u32 per clocks entry, min then max */
	if (!ofnode_read_u32_index(node, "freq-table-hz", pos * 2 + 1, &clk) &&
	    clk > 0)
		return clk;

fallback:
	/* default for backwards compatibility */
	return UNIPRO_CORE_CLK_FREQ_150_MHZ * 1000 * 1000;
};

static int ufs_qcom_set_clk_40ns_cycles(struct ufs_hba *hba,
					u32 cycles_in_1us)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	u32 cycles_in_40ns;
	int err;
	u32 reg;

	/*
	 * UFS host controller V4.0.0 onwards needs to program
	 * PA_VS_CORE_CLK_40NS_CYCLES attribute per programmed
	 * frequency of unipro core clk of UFS host controller.
	 */
	if (priv->hw_ver.major < 4)
		return 0;

	/*
	 * Generic formulae for cycles_in_40ns = (freq_unipro/25) is not
	 * applicable for all frequencies. For ex: ceil(37.5 MHz/25) will
	 * be 2 and ceil(403 MHZ/25) will be 17 whereas Hardware
	 * specification expect to be 16. Hence use exact hardware spec
	 * mandated value for cycles_in_40ns instead of calculating using
	 * generic formulae.
	 */
	switch (cycles_in_1us) {
	case UNIPRO_CORE_CLK_FREQ_403_MHZ:
		cycles_in_40ns = 16;
		break;
	case UNIPRO_CORE_CLK_FREQ_300_MHZ:
		cycles_in_40ns = 12;
		break;
	case UNIPRO_CORE_CLK_FREQ_201_5_MHZ:
		cycles_in_40ns = 8;
		break;
	case UNIPRO_CORE_CLK_FREQ_150_MHZ:
		cycles_in_40ns = 6;
		break;
	case UNIPRO_CORE_CLK_FREQ_100_MHZ:
		cycles_in_40ns = 4;
		break;
	case  UNIPRO_CORE_CLK_FREQ_75_MHZ:
		cycles_in_40ns = 3;
		break;
	case UNIPRO_CORE_CLK_FREQ_37_5_MHZ:
		cycles_in_40ns = 2;
		break;
	default:
		dev_err(hba->dev, "UNIPRO clk freq %u MHz not supported\n",
			cycles_in_1us);
		return -EINVAL;
	}

	err = ufshcd_dme_get(hba, UIC_ARG_MIB(PA_VS_CORE_CLK_40NS_CYCLES), &reg);
	if (err)
		return err;

	reg &= ~PA_VS_CORE_CLK_40NS_CYCLES_MASK;
	reg |= cycles_in_40ns;

	return ufshcd_dme_set(hba, UIC_ARG_MIB(PA_VS_CORE_CLK_40NS_CYCLES), reg);
}

static int ufs_qcom_set_core_clk_ctrl(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	u32 core_clk_ctrl_reg;
	u32 cycles_in_1us;
	int err;

	cycles_in_1us = ceil(ufs_qcom_get_core_clk_unipro_max_freq(hba),
			     (1000 * 1000));
	err = ufshcd_dme_get(hba,
			     UIC_ARG_MIB(DME_VS_CORE_CLK_CTRL),
			     &core_clk_ctrl_reg);
	if (err)
		return err;

	/* Bit mask is different for UFS host controller V4.0.0 onwards */
	if (priv->hw_ver.major >= 4) {
		core_clk_ctrl_reg &= ~CLK_1US_CYCLES_MASK_V4;
		core_clk_ctrl_reg |= FIELD_PREP(CLK_1US_CYCLES_MASK_V4, cycles_in_1us);
	} else {
		core_clk_ctrl_reg &= ~CLK_1US_CYCLES_MASK;
		core_clk_ctrl_reg |= FIELD_PREP(CLK_1US_CYCLES_MASK, cycles_in_1us);
	}

	/* Clear CORE_CLK_DIV_EN */
	core_clk_ctrl_reg &= ~DME_VS_CORE_CLK_CTRL_CORE_CLK_DIV_EN_BIT;

	err = ufshcd_dme_set(hba,
			     UIC_ARG_MIB(DME_VS_CORE_CLK_CTRL),
			     core_clk_ctrl_reg);
	if (err)
		return err;

	/* Configure unipro core clk 40ns attribute */
	return ufs_qcom_set_clk_40ns_cycles(hba, cycles_in_1us);
}

static u32 ufs_qcom_get_local_unipro_ver(struct ufs_hba *hba)
{
	/* HCI version 1.0 and 1.1 supports UniPro 1.41 */
	switch (hba->version) {
	case UFSHCI_VERSION_10:
	case UFSHCI_VERSION_11:
		return UFS_UNIPRO_VER_1_41;

	case UFSHCI_VERSION_20:
	case UFSHCI_VERSION_21:
	default:
		return UFS_UNIPRO_VER_1_6;
	}
}

static int ufs_qcom_link_startup_notify(struct ufs_hba *hba,
					enum ufs_notify_change_status status)
{
	int err = 0;

	switch (status) {
	case PRE_CHANGE:
		err = ufs_qcom_set_core_clk_ctrl(hba);
		if (err)
			dev_err(hba->dev, "cfg core clk ctrl failed\n");
		/*
		 * Some UFS devices (and may be host) have issues if LCC is
		 * enabled. So we are setting PA_Local_TX_LCC_Enable to 0
		 * before link startup which will make sure that both host
		 * and device TX LCC are disabled once link startup is
		 * completed.
		 */
		if (ufs_qcom_get_local_unipro_ver(hba) != UFS_UNIPRO_VER_1_41)
			err = ufshcd_dme_set(hba, UIC_ARG_MIB(PA_LOCAL_TX_LCC_ENABLE), 0);

		break;
	default:
		break;
	}

	return err;
}

static void ufs_qcom_dev_ref_clk_ctrl(struct ufs_hba *hba, bool enable)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);

	if (enable ^ priv->is_dev_ref_clk_enabled) {
		u32 temp = readl_relaxed(hba->mmio_base + REG_UFS_CFG1);

		if (enable)
			temp |= BIT(26);
		else
			temp &= ~BIT(26);

		/*
		 * If we are here to disable this clock it might be immediately
		 * after entering into hibern8 in which case we need to make
		 * sure that device ref_clk is active for specific time after
		 * hibern8 enter.
		 */
		if (!enable)
			udelay(10);

		writel_relaxed(temp, hba->mmio_base + REG_UFS_CFG1);

		/*
		 * Make sure the write to ref_clk reaches the destination and
		 * not stored in a Write Buffer (WB).
		 */
		readl(hba->mmio_base + REG_UFS_CFG1);

		/*
		 * If we call hibern8 exit after this, we need to make sure that
		 * device ref_clk is stable for at least 1us before the hibern8
		 * exit command.
		 */
		if (enable)
			udelay(1);

		priv->is_dev_ref_clk_enabled = enable;
	}
}

/**
 * ufs_qcom_init - bind phy with controller
 * @hba: host controller instance
 *
 * Powers up PHY enabling clocks and regulators.
 *
 * Returns -EPROBE_DEFER if binding fails, returns negative error
 * on phy power up failure and returns zero on success.
 */
static int ufs_qcom_init(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);
	int err;

	priv->hba = hba;

	/* setup clocks */
	ufs_qcom_setup_clocks(hba, true, PRE_CHANGE);

	if (priv->hw_ver.major >= 0x4)
		ufshcd_dme_set(hba,
			       UIC_ARG_MIB(PA_TXHSADAPTTYPE),
			       PA_NO_ADAPT);

	ufs_qcom_setup_clocks(hba, true, POST_CHANGE);

	ufs_qcom_get_controller_revision(hba, &priv->hw_ver.major,
					 &priv->hw_ver.minor,
					 &priv->hw_ver.step);
	dev_info(hba->dev, "Qcom UFS HC version: %d.%d.%d\n",
		 priv->hw_ver.major,
		 priv->hw_ver.minor,
		 priv->hw_ver.step);

	err = ufs_qcom_init_clks(priv);
	if (err) {
		dev_err(hba->dev, "failed to initialize clocks, err:%d\n", err);
		return err;
	}

	ufs_qcom_advertise_quirks(hba);
	ufs_qcom_setup_clocks(hba, true, POST_CHANGE);

	return 0;
}

/**
 * ufs_qcom_device_reset() - toggle the (optional) device reset line
 * @hba: per-adapter instance
 *
 * Toggles the (optional) reset line to reset the attached device.
 */
static int ufs_qcom_device_reset(struct ufs_hba *hba)
{
	struct ufs_qcom_priv *priv = dev_get_priv(hba->dev);

	if (!dm_gpio_is_valid(&priv->reset))
		return 0;

	/*
	 * The UFS device shall detect reset pulses of 1us, sleep for 10us to
	 * be on the safe side.
	 */
	dm_gpio_set_value(&priv->reset, true);
	udelay(10);

	dm_gpio_set_value(&priv->reset, false);
	udelay(10);

	return 0;
}

static struct ufs_hba_ops ufs_qcom_hba_ops = {
	.init			= ufs_qcom_init,
	.get_max_pwr_mode	= ufs_get_max_pwr_mode,
	.hce_enable_notify	= ufs_qcom_hce_enable_notify,
	.link_startup_notify	= ufs_qcom_link_startup_notify,
	.device_reset		= ufs_qcom_device_reset,
};

static int ufs_qcom_probe(struct udevice *dev)
{
	struct ufs_qcom_priv *priv = dev_get_priv(dev);
	int ret;

	/* get resets */
	ret = reset_get_by_name(dev, "rst", &priv->core_reset);
	if (ret) {
		dev_err(dev, "failed to get reset, ret:%d\n", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get reset GPIO\n");
	}

	ret = ufshcd_probe(dev, &ufs_qcom_hba_ops);
	if (ret) {
		dev_err(dev, "ufshcd_probe() failed, ret:%d\n", ret);
		return ret;
	}

	return 0;
}

static int ufs_qcom_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ufs_scsi_bind(dev, &scsi_dev);
}

static const struct udevice_id ufs_qcom_ids[] = {
	{ .compatible = "qcom,ufshc" },
	{},
};

U_BOOT_DRIVER(qcom_ufshcd) = {
	.name		= "qcom-ufshcd",
	.id		= UCLASS_UFS,
	.of_match	= ufs_qcom_ids,
	.probe		= ufs_qcom_probe,
	.bind		= ufs_qcom_bind,
	.priv_auto	= sizeof(struct ufs_qcom_priv),
};
