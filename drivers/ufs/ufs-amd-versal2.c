// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024 Advanced Micro Devices, Inc.
 */

#include <clk.h>
#include <dm.h>
#include <ufs.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <zynqmp_firmware.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/time.h>
#include <reset.h>

#include "ufs.h"
#include "ufshcd-dwc.h"
#include "ufshci-dwc.h"

#define SRAM_CSR_INIT_DONE_MASK		BIT(0)
#define SRAM_CSR_EXT_LD_DONE_MASK	BIT(1)
#define SRAM_CSR_BYPASS_MASK		BIT(2)

#define MPHY_FAST_RX_AFE_CAL		BIT(2)
#define MPHY_FW_CALIB_CFG_VAL		BIT(8)

#define TX_RX_CFG_RDY_MASK		GENMASK(3, 0)

#define TIMEOUT_MICROSEC		1000000L

struct ufs_versal2_priv {
	struct ufs_hba *hba;
	struct reset_ctl *rstc;
	struct reset_ctl *rstphy;
	u32 phy_mode;
	u32 host_clk;
	u8 attcompval0;
	u8 attcompval1;
	u8 ctlecompval0;
	u8 ctlecompval1;
};

static int ufs_versal2_phy_reg_write(struct ufs_hba *hba, u32 addr, u32 val)
{
	static struct ufshcd_dme_attr_val phy_write_attrs[] = {
		{ UIC_ARG_MIB(CBCREGADDRLSB), 0, DME_LOCAL },
		{ UIC_ARG_MIB(CBCREGADDRMSB), 0, DME_LOCAL },
		{ UIC_ARG_MIB(CBCREGWRLSB), 0, DME_LOCAL },
		{ UIC_ARG_MIB(CBCREGWRMSB), 0, DME_LOCAL },
		{ UIC_ARG_MIB(CBCREGRDWRSEL), 1, DME_LOCAL },
		{ UIC_ARG_MIB(VS_MPHYCFGUPDT), 1, DME_LOCAL }
	};

	phy_write_attrs[0].mib_val = (u8)addr;
	phy_write_attrs[1].mib_val = (u8)(addr >> 8);
	phy_write_attrs[2].mib_val = (u8)val;
	phy_write_attrs[3].mib_val = (u8)(val >> 8);

	return ufshcd_dwc_dme_set_attrs(hba, phy_write_attrs, ARRAY_SIZE(phy_write_attrs));
}

static int ufs_versal2_phy_reg_read(struct ufs_hba *hba, u32 addr, u32 *val)
{
	u32 mib_val;
	int ret;
	static struct ufshcd_dme_attr_val phy_read_attrs[] = {
		{ UIC_ARG_MIB(CBCREGADDRLSB), 0, DME_LOCAL },
		{ UIC_ARG_MIB(CBCREGADDRMSB), 0, DME_LOCAL },
		{ UIC_ARG_MIB(CBCREGRDWRSEL), 0, DME_LOCAL },
		{ UIC_ARG_MIB(VS_MPHYCFGUPDT), 1, DME_LOCAL }
	};

	phy_read_attrs[0].mib_val = (u8)addr;
	phy_read_attrs[1].mib_val = (u8)(addr >> 8);

	ret = ufshcd_dwc_dme_set_attrs(hba, phy_read_attrs, ARRAY_SIZE(phy_read_attrs));
	if (ret)
		return ret;

	ret = ufshcd_dme_get(hba, UIC_ARG_MIB(CBCREGRDLSB), &mib_val);
	if (ret)
		return ret;

	*val = mib_val;
	ret = ufshcd_dme_get(hba, UIC_ARG_MIB(CBCREGRDMSB), &mib_val);
	if (ret)
		return ret;

	*val |= (mib_val << 8);

	return 0;
}

static int ufs_versal2_enable_phy(struct ufs_hba *hba)
{
	u32 offset, reg;
	int ret;

	ret = ufshcd_dme_set(hba, UIC_ARG_MIB(VS_MPHYDISABLE), 0);
	if (ret)
		return ret;

	ret = ufshcd_dme_set(hba, UIC_ARG_MIB(VS_MPHYCFGUPDT), 1);
	if (ret)
		return ret;

	/* Check Tx/Rx FSM states */
	for (offset = 0; offset < 2; offset++) {
		u32 time_left, mibsel;

		time_left = TIMEOUT_MICROSEC;
		mibsel = UIC_ARG_MIB_SEL(MTX_FSM_STATE, UIC_ARG_MPHY_TX_GEN_SEL_INDEX(offset));
		do {
			ret = ufshcd_dme_get(hba, mibsel, &reg);
			if (ret)
				return ret;

			if (reg == TX_STATE_HIBERN8 || reg == TX_STATE_SLEEP ||
			    reg == TX_STATE_LSBURST)
				break;

			time_left--;
			mdelay(5);
		} while (time_left);

		if (!time_left) {
			dev_err(hba->dev, "Invalid Tx FSM state.\n");
			return -ETIMEDOUT;
		}

		time_left = TIMEOUT_MICROSEC;
		mibsel = UIC_ARG_MIB_SEL(MRX_FSM_STATE, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(offset));
		do {
			ret = ufshcd_dme_get(hba, mibsel, &reg);
			if (ret)
				return ret;

			if (reg == RX_STATE_HIBERN8 || reg == RX_STATE_SLEEP ||
			    reg == RX_STATE_LSBURST)
				break;

			time_left--;
			mdelay(5);
		} while (time_left);

		if (!time_left) {
			dev_err(hba->dev, "Invalid Rx FSM state.\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int ufs_versal2_setup_phy(struct ufs_hba *hba)
{
	struct ufs_versal2_priv *priv = dev_get_priv(hba->dev);
	int ret;
	u32 reg;

	/* Bypass RX-AFE offset calibrations (ATT/CTLE) */
	ret = ufs_versal2_phy_reg_read(hba, FAST_FLAGS(0), &reg);
	if (ret)
		return ret;

	reg |= MPHY_FAST_RX_AFE_CAL;
	ret = ufs_versal2_phy_reg_write(hba, FAST_FLAGS(0), reg);
	if (ret)
		return ret;

	ret = ufs_versal2_phy_reg_read(hba, FAST_FLAGS(1), &reg);
	if (ret)
		return ret;

	reg |= MPHY_FAST_RX_AFE_CAL;
	ret = ufs_versal2_phy_reg_write(hba, FAST_FLAGS(1), reg);
	if (ret)
		return ret;

	/* Program ATT and CTLE compensation values */
	if (priv->attcompval0) {
		ret = ufs_versal2_phy_reg_write(hba, RX_AFE_ATT_IDAC(0), priv->attcompval0);
		if (ret)
			return ret;
	}

	if (priv->attcompval1) {
		ret = ufs_versal2_phy_reg_write(hba, RX_AFE_ATT_IDAC(1), priv->attcompval1);
		if (ret)
			return ret;
	}

	if (priv->ctlecompval0) {
		ret = ufs_versal2_phy_reg_write(hba, RX_AFE_CTLE_IDAC(0), priv->ctlecompval0);
		if (ret)
			return ret;
	}

	if (priv->ctlecompval1) {
		ret = ufs_versal2_phy_reg_write(hba, RX_AFE_CTLE_IDAC(1), priv->ctlecompval1);
		if (ret)
			return ret;
	}

	ret = ufs_versal2_phy_reg_read(hba, FW_CALIB_CCFG(0), &reg);
	if (ret)
		return ret;

	reg |= MPHY_FW_CALIB_CFG_VAL;
	ret = ufs_versal2_phy_reg_write(hba, FW_CALIB_CCFG(0), reg);
	if (ret)
		return ret;

	ret = ufs_versal2_phy_reg_read(hba, FW_CALIB_CCFG(1), &reg);
	if (ret)
		return ret;

	reg |= MPHY_FW_CALIB_CFG_VAL;
	return ufs_versal2_phy_reg_write(hba, FW_CALIB_CCFG(1), reg);
}

static int ufs_versal2_phy_init(struct ufs_hba *hba)
{
	struct ufs_versal2_priv *priv = dev_get_priv(hba->dev);
	u32 reg, time_left;
	int ret;
	static const struct ufshcd_dme_attr_val rmmi_attrs[] = {
		{ UIC_ARG_MIB(CBREFCLKCTRL2), CBREFREFCLK_GATE_OVR_EN, DME_LOCAL },
		{ UIC_ARG_MIB(CBCRCTRL), 1, DME_LOCAL },
		{ UIC_ARG_MIB(CBC10DIRECTCONF2), 1, DME_LOCAL },
		{ UIC_ARG_MIB(VS_MPHYCFGUPDT), 1, DME_LOCAL }
	};

	/* Wait for Tx/Rx config_rdy */
	time_left = TIMEOUT_MICROSEC;
	do {
		time_left--;
		ret = zynqmp_pm_ufs_get_txrx_cfgrdy(&reg);
		if (ret)
			return ret;

		reg &= TX_RX_CFG_RDY_MASK;
		if (!reg)
			break;

		mdelay(5);
	} while (time_left);

	if (!time_left) {
		dev_err(hba->dev, "Tx/Rx configuration signal busy.\n");
		return -ETIMEDOUT;
	}

	ret = ufshcd_dwc_dme_set_attrs(hba, rmmi_attrs, ARRAY_SIZE(rmmi_attrs));
	if (ret)
		return ret;

	/* DeAssert PHY reset */
	ret = reset_deassert(priv->rstphy);
	if (ret) {
		dev_err(hba->dev, "ufsphy reset deassert failed\n");
		return ret;
	}

	/* Wait for SRAM init done */
	time_left = TIMEOUT_MICROSEC;
	do {
		time_left--;
		ret = zynqmp_pm_ufs_sram_csr_read(&reg);
		if (ret)
			return ret;

		reg &= SRAM_CSR_INIT_DONE_MASK;
		if (reg)
			break;

		mdelay(5);
	} while (time_left);

	if (!time_left) {
		dev_err(hba->dev, "SRAM initialization failed.\n");
		return -ETIMEDOUT;
	}

	ret = ufs_versal2_setup_phy(hba);
	if (ret)
		return ret;

	return ufs_versal2_enable_phy(hba);
}

static int ufs_versal2_init(struct ufs_hba *hba)
{
	struct ufs_versal2_priv *priv = dev_get_priv(hba->dev);
	struct clk clk;
	unsigned long core_clk_rate = 0;
	u32 cal;
	int ret = 0;

	priv->phy_mode = UFSHCD_DWC_PHY_MODE_ROM;

	ret = clk_get_by_name(hba->dev, "core_clk", &clk);
	if (ret) {
		dev_err(hba->dev, "failed to get core_clk clock\n");
		return ret;
	}

	core_clk_rate = clk_get_rate(&clk);
	if (IS_ERR_VALUE(core_clk_rate)) {
		dev_err(hba->dev, "%s: unable to find core_clk rate\n",
			__func__);
		return core_clk_rate;
	}
	priv->host_clk = core_clk_rate;

	priv->rstc = devm_reset_control_get(hba->dev, "ufshc-rst");
	if (IS_ERR(priv->rstc)) {
		dev_err(hba->dev, "failed to get reset ctl: ufshc-rst\n");
		return PTR_ERR(priv->rstc);
	}
	priv->rstphy = devm_reset_control_get(hba->dev, "ufsphy-rst");
	if (IS_ERR(priv->rstphy)) {
		dev_err(hba->dev, "failed to get reset ctl: ufsphy-rst\n");
		return PTR_ERR(priv->rstphy);
	}

	ret =  zynqmp_pm_ufs_cal_reg(&cal);
	if (ret)
		return ret;

	priv->attcompval0 = (u8)cal;
	priv->attcompval1 = (u8)(cal >> 8);
	priv->ctlecompval0 = (u8)(cal >> 16);
	priv->ctlecompval1 = (u8)(cal >> 24);

	return ret;
}

static int ufs_versal2_hce_enable_notify(struct ufs_hba *hba,
					 enum ufs_notify_change_status status)
{
	struct ufs_versal2_priv *priv = dev_get_priv(hba->dev);
	u32 sram_csr;
	int ret;

	switch (status) {
	case PRE_CHANGE:
		/* Assert RST_UFS Reset for UFS block in PMX_IOU */
		ret = reset_assert(priv->rstc);
		if (ret) {
			dev_err(hba->dev, "ufshc reset assert failed, err = %d\n", ret);
			return ret;
		}

		/* Assert PHY reset */
		ret = reset_assert(priv->rstphy);
		if (ret) {
			dev_err(hba->dev, "ufsphy reset assert failed, err = %d\n", ret);
			return ret;
		}

		ret = zynqmp_pm_ufs_sram_csr_read(&sram_csr);
		if (ret)
			return ret;

		if (!priv->phy_mode) {
			sram_csr &= ~SRAM_CSR_EXT_LD_DONE_MASK;
			sram_csr |= SRAM_CSR_BYPASS_MASK;
		} else {
			dev_err(hba->dev, "Invalid phy-mode %d.\n", priv->phy_mode);
			return -EINVAL;
		}

		ret = zynqmp_pm_ufs_sram_csr_write(&sram_csr);
		if (ret)
			return ret;

		/* De Assert RST_UFS Reset for UFS block in PMX_IOU */
		ret = reset_deassert(priv->rstc);
		if (ret)
			dev_err(hba->dev, "ufshc reset deassert failed, err = %d\n", ret);

		break;
	case POST_CHANGE:
		ret = ufs_versal2_phy_init(hba);
		if (ret)
			dev_err(hba->dev, "Phy init failed (%d)\n", ret);

		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int ufs_versal2_link_startup_notify(struct ufs_hba *hba,
					   enum ufs_notify_change_status status)
{
	struct ufs_versal2_priv *priv = dev_get_priv(hba->dev);
	int ret = 0;

	switch (status) {
	case PRE_CHANGE:
		if (priv->host_clk) {
			u32 core_clk_div = priv->host_clk / TIMEOUT_MICROSEC;

			ufshcd_writel(hba, core_clk_div, DWC_UFS_REG_HCLKDIV);
		}
		break;
	case POST_CHANGE:
		ret = ufshcd_dwc_link_startup_notify(hba, status);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static struct ufs_hba_ops ufs_versal2_hba_ops = {
	.init = ufs_versal2_init,
	.link_startup_notify = ufs_versal2_link_startup_notify,
	.hce_enable_notify = ufs_versal2_hce_enable_notify,
};

static int ufs_versal2_probe(struct udevice *dev)
{
	int ret;

	/* Perform generic probe */
	ret = ufshcd_probe(dev, &ufs_versal2_hba_ops);
	if (ret)
		dev_err(dev, "ufshcd_probe() failed %d\n", ret);

	return ret;
}

static int ufs_versal2_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ufs_scsi_bind(dev, &scsi_dev);
}

static const struct udevice_id ufs_versal2_ids[] = {
	{
		.compatible = "amd,versal2-ufs",
	},
	{},
};

U_BOOT_DRIVER(ufs_versal2_pltfm) = {
	.name           = "ufs-versal2-pltfm",
	.id             = UCLASS_UFS,
	.of_match       = ufs_versal2_ids,
	.probe          = ufs_versal2_probe,
	.bind           = ufs_versal2_bind,
};
