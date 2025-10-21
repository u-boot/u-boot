// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author: Christian Marangi <ansuelsmth@gmail.com>
 */
#include <dm.h>
#include <dm/device_compat.h>
#include <regmap.h>

#include "pcs-airoha.h"

static void an7581_pcs_jcpll_bringup(struct airoha_pcs_priv *priv,
				     phy_interface_t interface)
{
	u32 kband_vref;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_2500BASEX:
		kband_vref = 0x10;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		kband_vref = 0xf;
		break;
	default:
		return;
	}

	/* Setup LDO */
	udelay(200);

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SPARE_H,
			AIROHA_PCS_ANA_JCPLL_SPARE_L_LDO);

	/* Setup RSTB */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_RST_DLY,
			   AIROHA_PCS_ANA_JCPLL_RST_DLY,
			   AIROHA_PCS_ANA_JCPLL_RST_DLY_150_200);

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_RST_DLY,
			AIROHA_PCS_ANA_JCPLL_PLL_RSTB);

	/* Enable PLL force selection and Force Disable */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_JCPLL_CKOUT_EN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_JCPLL_EN |
			   AIROHA_PCS_PMA_FORCE_DA_JCPLL_EN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_JCPLL_EN);

	/* Setup SDM */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_RST_DLY,
			   AIROHA_PCS_ANA_JCPLL_SDM_DI_LS |
			   AIROHA_PCS_ANA_JCPLL_SDM_DI_EN,
			   AIROHA_PCS_ANA_JCPLL_SDM_DI_LS_2_23);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SDM_IFM,
			   AIROHA_PCS_ANA_JCPLL_SDM_OUT |
			   AIROHA_PCS_ANA_JCPLL_SDM_ORD |
			   AIROHA_PCS_ANA_JCPLL_SDM_MODE |
			   AIROHA_PCS_ANA_JCPLL_SDM_IFM,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_SDM_ORD, 0x0) |
			   AIROHA_PCS_ANA_JCPLL_SDM_ORD_3SDM |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_SDM_MODE, 0x0));

	regmap_clear_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SDM_HREN,
			  AIROHA_PCS_ANA_JCPLL_SDM_HREN);

	/* Setup SSC */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SSC_DELTA,
			   AIROHA_PCS_ANA_JCPLL_SSC_PERIOD |
			   AIROHA_PCS_ANA_JCPLL_SSC_DELTA,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_SSC_PERIOD, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_SSC_DELTA, 0x0));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SSC_TRI_EN,
			   AIROHA_PCS_ANA_JCPLL_SSC_DELTA1 |
			   AIROHA_PCS_ANA_JCPLL_SSC_TRI_EN,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_SSC_DELTA1, 0x0));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_VCO_TCLVAR,
			   AIROHA_PCS_ANA_JCPLL_SSC_PHASE_INI |
			   AIROHA_PCS_ANA_JCPLL_SSC_EN |
			   AIROHA_PCS_ANA_JCPLL_VCO_VCOVAR_BIAS_L |
			   AIROHA_PCS_ANA_JCPLL_VCO_TCLVAR,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_VCOVAR_BIAS_L, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_TCLVAR, 0x0));

	/* Setup LPF */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_IB_EXT_EN,
			   AIROHA_PCS_ANA_JCPLL_CHP_IOFST |
			   AIROHA_PCS_ANA_JCPLL_CHP_IBIAS |
			   AIROHA_PCS_ANA_JCPLL_LPF_SHCK_EN,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_CHP_IOFST, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_CHP_IBIAS, 0x18));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_LPF_BR,
			   AIROHA_PCS_ANA_JCPLL_LPF_BWR |
			   AIROHA_PCS_ANA_JCPLL_LPF_BP |
			   AIROHA_PCS_ANA_JCPLL_LPF_BC |
			   AIROHA_PCS_ANA_JCPLL_LPF_BR,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_LPF_BWR, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_LPF_BP, 0x10) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_LPF_BC, 0x1f) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_LPF_BR, BIT(3) | BIT(1)));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_LPF_BWC,
			   AIROHA_PCS_ANA_JCPLL_LPF_BWC,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_LPF_BWC, 0x0));

	/* Setup VCO */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_VCODIV,
			   AIROHA_PCS_ANA_JCPLL_VCO_SCAPWR |
			   AIROHA_PCS_ANA_JCPLL_VCO_HALFLSB_EN |
			   AIROHA_PCS_ANA_JCPLL_VCO_CFIX,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_SCAPWR, 0x4) |
			   AIROHA_PCS_ANA_JCPLL_VCO_HALFLSB_EN |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_CFIX, 0x1));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_VCO_TCLVAR,
			   AIROHA_PCS_ANA_JCPLL_VCO_VCOVAR_BIAS_L |
			   AIROHA_PCS_ANA_JCPLL_VCO_VCOVAR_BIAS_H |
			   AIROHA_PCS_ANA_JCPLL_VCO_TCLVAR,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_VCOVAR_BIAS_L, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_VCOVAR_BIAS_H, 0x3) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_VCO_TCLVAR, 0x3));

	/* Setup PCW */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_JCPLL_SDM_PCW,
			   AIROHA_PCS_PMA_FORCE_DA_JCPLL_SDM_PCW,
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_JCPLL_SDM_PCW, 0x25800000));

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_RX_FE_VOS,
			AIROHA_PCS_PMA_FORCE_SEL_DA_JCPLL_SDM_PCW);

	/* Setup DIV */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_MMD_PREDIV_MODE,
			   AIROHA_PCS_ANA_JCPLL_POSTDIV_D5 |
			   AIROHA_PCS_ANA_JCPLL_MMD_PREDIV_MODE,
			   AIROHA_PCS_ANA_JCPLL_MMD_PREDIV_MODE_2);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_VCODIV,
			   AIROHA_PCS_ANA_JCPLL_VCODIV,
			   AIROHA_PCS_ANA_JCPLL_VCODIV_1);

	/* Setup KBand */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_KBAND_KFC,
			   AIROHA_PCS_ANA_JCPLL_KBAND_KS |
			   AIROHA_PCS_ANA_JCPLL_KBAND_KF |
			   AIROHA_PCS_ANA_JCPLL_KBAND_KFC,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_KBAND_KS, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_KBAND_KF, 0x3) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_KBAND_KFC, 0x0));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_LPF_BWC,
			   AIROHA_PCS_ANA_JCPLL_KBAND_DIV |
			   AIROHA_PCS_ANA_JCPLL_KBAND_CODE |
			   AIROHA_PCS_ANA_JCPLL_KBAND_OPTION,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_KBAND_DIV, 0x2) |
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_KBAND_CODE, 0xe4));

	/* Setup TCL */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SPARE_H,
			   AIROHA_PCS_ANA_JCPLL_TCL_KBAND_VREF,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_TCL_KBAND_VREF, kband_vref));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_SDM_HREN,
			   AIROHA_PCS_ANA_JCPLL_TCL_AMP_VREF |
			   AIROHA_PCS_ANA_JCPLL_TCL_AMP_GAIN |
			   AIROHA_PCS_ANA_JCPLL_TCL_AMP_EN,
			   FIELD_PREP(AIROHA_PCS_ANA_JCPLL_TCL_AMP_VREF, 0x5) |
			   AIROHA_PCS_ANA_JCPLL_TCL_AMP_GAIN_4 |
			   AIROHA_PCS_ANA_JCPLL_TCL_AMP_EN);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_JCPLL_TCL_CMP_EN,
			   AIROHA_PCS_ANA_JCPLL_TCL_LPF_BW |
			   AIROHA_PCS_ANA_JCPLL_TCL_LPF_EN,
			   AIROHA_PCS_ANA_JCPLL_TCL_LPF_BW_1 |
			   AIROHA_PCS_ANA_JCPLL_TCL_LPF_EN);

	/* Enable PLL */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_JCPLL_CKOUT_EN,
			AIROHA_PCS_PMA_FORCE_DA_JCPLL_EN);

	/* Enale PLL Output */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_JCPLL_CKOUT_EN,
			AIROHA_PCS_PMA_FORCE_SEL_DA_JCPLL_CKOUT_EN |
			AIROHA_PCS_PMA_FORCE_DA_JCPLL_CKOUT_EN);
}

static void an7581_pcs_txpll_bringup(struct airoha_pcs_priv *priv,
				     phy_interface_t interface)
{
	u32 lpf_chp_ibias, lpf_bp, lpf_bwr, lpf_bwc;
	u32 vco_cfix;
	u32 pcw;
	u32 tcl_amp_vref;
	bool sdm_hren;
	bool vcodiv;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		lpf_chp_ibias = 0xf;
		lpf_bp = BIT(1);
		lpf_bwr = BIT(3) | BIT(1) | BIT(0);
		lpf_bwc = BIT(4) | BIT(3);
		vco_cfix = BIT(1) | BIT(0);
		pcw = BIT(27);
		tcl_amp_vref = BIT(3) | BIT(1) | BIT(0);
		vcodiv = false;
		sdm_hren = false;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		lpf_chp_ibias = 0xa;
		lpf_bp = BIT(2) | BIT(0);
		lpf_bwr = 0;
		lpf_bwc = 0;
		vco_cfix = 0;
		pcw = BIT(27) | BIT(25);
		tcl_amp_vref = BIT(3) | BIT(2) | BIT(0);
		vcodiv = true;
		sdm_hren = false;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		lpf_chp_ibias = 0xf;
		lpf_bp = BIT(1);
		lpf_bwr = BIT(3) | BIT(1) | BIT(0);
		lpf_bwc = BIT(4) | BIT(3);
		vco_cfix = BIT(0);
		pcw = BIT(27) | BIT(22);
		tcl_amp_vref = BIT(3) | BIT(1) | BIT(0);
		vcodiv = false;
		sdm_hren = true;
		break;
	default:
		return;
	}

	/* Setup VCO LDO Output */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SSC_PERIOD,
			   AIROHA_PCS_ANA_TXPLL_LDO_VCO_OUT |
			   AIROHA_PCS_ANA_TXPLL_LDO_OUT,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LDO_VCO_OUT, 0x1) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LDO_OUT, 0x1));

	/* Setup RSTB */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_REFIN_INTERNAL,
			   AIROHA_PCS_ANA_TXPLL_PLL_RSTB |
			   AIROHA_PCS_ANA_TXPLL_RST_DLY |
			   AIROHA_PCS_ANA_TXPLL_REFIN_DIV |
			   AIROHA_PCS_ANA_TXPLL_REFIN_INTERNAL,
			   AIROHA_PCS_ANA_TXPLL_PLL_RSTB |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_RST_DLY, 0x4) |
			   AIROHA_PCS_ANA_TXPLL_REFIN_DIV_1 |
			   AIROHA_PCS_ANA_TXPLL_REFIN_INTERNAL);

	/* Enable PLL force selection and Force Disable */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TXPLL_CKOUT_EN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TXPLL_EN |
			   AIROHA_PCS_PMA_FORCE_DA_TXPLL_EN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TXPLL_EN);

	/* Setup SDM */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SDM_DI_EN,
			   AIROHA_PCS_ANA_TXPLL_SDM_MODE |
			   AIROHA_PCS_ANA_TXPLL_SDM_IFM |
			   AIROHA_PCS_ANA_TXPLL_SDM_DI_LS |
			   AIROHA_PCS_ANA_TXPLL_SDM_DI_EN,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_SDM_MODE, 0) |
			   AIROHA_PCS_ANA_TXPLL_SDM_DI_LS_2_23);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SDM_ORD,
			   AIROHA_PCS_ANA_TXPLL_SDM_HREN |
			   AIROHA_PCS_ANA_TXPLL_SDM_OUT |
			   AIROHA_PCS_ANA_TXPLL_SDM_ORD,
			   (sdm_hren ? AIROHA_PCS_ANA_TXPLL_SDM_HREN : 0) |
			   AIROHA_PCS_ANA_TXPLL_SDM_ORD_3SDM);

	/* Setup SSC */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SSC_DELTA1,
			   AIROHA_PCS_ANA_TXPLL_SSC_DELTA |
			   AIROHA_PCS_ANA_TXPLL_SSC_DELTA1,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_SSC_DELTA, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_SSC_DELTA1, 0x0));

	regmap_clear_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SSC_EN,
			  AIROHA_PCS_ANA_TXPLL_SSC_TRI_EN |
			  AIROHA_PCS_ANA_TXPLL_SSC_PHASE_INI |
			  AIROHA_PCS_ANA_TXPLL_SSC_EN);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SSC_PERIOD,
			   AIROHA_PCS_ANA_TXPLL_SSC_PERIOD,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_SSC_PERIOD, 0x0));

	/* Setup LPF */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_CHP_IBIAS,
			   AIROHA_PCS_ANA_TXPLL_LPF_BC |
			   AIROHA_PCS_ANA_TXPLL_LPF_BR |
			   AIROHA_PCS_ANA_TXPLL_CHP_IOFST |
			   AIROHA_PCS_ANA_TXPLL_CHP_IBIAS,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LPF_BC, 0x1f) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LPF_BR, 0x5) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_CHP_IOFST, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_CHP_IBIAS, lpf_chp_ibias));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_LPF_BP,
			   AIROHA_PCS_ANA_TXPLL_LPF_BWC |
			   AIROHA_PCS_ANA_TXPLL_LPF_BWR |
			   AIROHA_PCS_ANA_TXPLL_LPF_BP,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LPF_BWC, lpf_bwc) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LPF_BWR, lpf_bwr) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_LPF_BP, lpf_bp));

	/* Setup VCO */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_TCL_LPF_EN,
			   AIROHA_PCS_ANA_TXPLL_VCO_CFIX,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_VCO_CFIX, vco_cfix));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_VCO_HALFLSB_EN,
			   AIROHA_PCS_ANA_TXPLL_VCO_VCOVAR_BIAS_L |
			   AIROHA_PCS_ANA_TXPLL_VCO_VCOVAR_BIAS_H |
			   AIROHA_PCS_ANA_TXPLL_VCO_TCLVAR |
			   AIROHA_PCS_ANA_TXPLL_VCO_SCAPWR |
			   AIROHA_PCS_ANA_TXPLL_VCO_HALFLSB_EN,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_VCO_VCOVAR_BIAS_L, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_VCO_VCOVAR_BIAS_H, 0x4) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_VCO_TCLVAR, 0x4) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_VCO_SCAPWR, 0x7) |
			   AIROHA_PCS_ANA_TXPLL_VCO_HALFLSB_EN);

	/* Setup PCW */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TXPLL_SDM_PCW,
			   AIROHA_PCS_PMA_FORCE_DA_TXPLL_SDM_PCW, pcw);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_IDAC,
			AIROHA_PCS_PMA_FORCE_SEL_DA_TXPLL_SDM_PCW);

	/* Setup KBand */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_KBAND_CODE,
			   AIROHA_PCS_ANA_TXPLL_KBAND_KF |
			   AIROHA_PCS_ANA_TXPLL_KBAND_KFC |
			   AIROHA_PCS_ANA_TXPLL_KBAND_DIV |
			   AIROHA_PCS_ANA_TXPLL_KBAND_CODE,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_KBAND_KF, 0x3) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_KBAND_KFC, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_KBAND_DIV, 0x4) |
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_KBAND_CODE, 0xe4));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_KBAND_KS,
			   AIROHA_PCS_ANA_TXPLL_KBAND_KS,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_KBAND_KS, 0x1));

	regmap_clear_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_LPF_BP,
			  AIROHA_PCS_ANA_TXPLL_KBAND_OPTION);

	/* Setup DIV */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_KBAND_KS,
			   AIROHA_PCS_ANA_TXPLL_MMD_PREDIV_MODE |
			   AIROHA_PCS_ANA_TXPLL_POSTDIV_EN,
			   AIROHA_PCS_ANA_TXPLL_MMD_PREDIV_MODE_2);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_TCL_LPF_EN,
			   AIROHA_PCS_ANA_TXPLL_VCODIV,
			   vcodiv ? AIROHA_PCS_ANA_TXPLL_VCODIV_2 :
				    AIROHA_PCS_ANA_TXPLL_VCODIV_1);

	/* Setup TCL */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_TCL_KBAND_VREF,
			   AIROHA_PCS_ANA_TXPLL_TCL_KBAND_VREF,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_TCL_KBAND_VREF, 0xf));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_TCL_AMP_GAIN,
			   AIROHA_PCS_ANA_TXPLL_TCL_AMP_VREF |
			   AIROHA_PCS_ANA_TXPLL_TCL_AMP_GAIN,
			   FIELD_PREP(AIROHA_PCS_ANA_TXPLL_TCL_AMP_VREF, tcl_amp_vref) |
			   AIROHA_PCS_ANA_TXPLL_TCL_AMP_GAIN_4);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_TCL_LPF_EN,
			   AIROHA_PCS_ANA_TXPLL_TCL_LPF_BW |
			   AIROHA_PCS_ANA_TXPLL_TCL_LPF_EN,
			   AIROHA_PCS_ANA_TXPLL_TCL_LPF_BW_0_5 |
			   AIROHA_PCS_ANA_TXPLL_TCL_LPF_EN);

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TXPLL_SDM_ORD,
			AIROHA_PCS_ANA_TXPLL_TCL_AMP_EN);

	/* Enable PLL */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TXPLL_CKOUT_EN,
			AIROHA_PCS_PMA_FORCE_DA_TXPLL_EN);

	/* Enale PLL Output */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TXPLL_CKOUT_EN,
			AIROHA_PCS_PMA_FORCE_SEL_DA_TXPLL_CKOUT_EN |
			AIROHA_PCS_PMA_FORCE_DA_TXPLL_CKOUT_EN);
}

static void an7581_pcs_tx_bringup(struct airoha_pcs_priv *priv,
				  phy_interface_t interface)
{
	u32 tx_rate_ctrl;
	u32 ckin_divisor;
	u32 fir_cn1, fir_c0b, fir_c1;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		ckin_divisor = BIT(1);
		tx_rate_ctrl = BIT(0);
		fir_cn1 = 0;
		fir_c0b = 12;
		fir_c1 = 0;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		ckin_divisor = BIT(2);
		tx_rate_ctrl = BIT(0);
		fir_cn1 = 0;
		fir_c0b = 11;
		fir_c1 = 1;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		ckin_divisor = BIT(2) | BIT(0);
		tx_rate_ctrl = BIT(1);
		fir_cn1 = 1;
		fir_c0b = 1;
		fir_c1 = 11;
		break;
	default:
		return;
	}

	/* Set TX rate ctrl */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_XPON_TX_RATE_CTRL,
			   AIROHA_PCS_PMA_PON_TX_RATE_CTRL,
			   FIELD_PREP(AIROHA_PCS_PMA_PON_TX_RATE_CTRL,
				      tx_rate_ctrl));

	/* Setup TX Config */
	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_TX_CKLDO_EN,
			AIROHA_PCS_ANA_TX_DMEDGEGEN_EN |
			AIROHA_PCS_ANA_TX_CKLDO_EN);

	udelay(1);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TX_ACJTAG_EN,
			AIROHA_PCS_PMA_FORCE_SEL_DA_TX_CKIN_SEL |
			AIROHA_PCS_PMA_FORCE_DA_TX_CKIN_SEL);

	/* FIXME: Ask Airoha TX term is OK to reset? */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TX_TERM_SEL,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_CKIN_DIVISOR |
			   AIROHA_PCS_PMA_FORCE_DA_TX_CKIN_DIVISOR |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_TERM_SEL |
			   AIROHA_PCS_PMA_FORCE_DA_TX_TERM_SEL,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_CKIN_DIVISOR |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_TX_CKIN_DIVISOR,
				      ckin_divisor) |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_TX_TERM_SEL, 0x0));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TX_RATE_CTRL,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_RATE_CTRL |
			   AIROHA_PCS_PMA_FORCE_DA_TX_RATE_CTRL,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_RATE_CTRL |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_TX_RATE_CTRL,
				      tx_rate_ctrl));

	/* Setup TX FIR Load Parameters (Reference 660mV) */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TX_FIR_C0B,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_CN1 |
			   AIROHA_PCS_PMA_FORCE_DA_TX_FIR_CN1 |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_C0B |
			   AIROHA_PCS_PMA_FORCE_DA_TX_FIR_C0B,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_CN1 |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_TX_FIR_CN1, fir_cn1) |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_C0B |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_TX_FIR_C0B, fir_c0b));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_TX_FIR_C1,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_C2 |
			   AIROHA_PCS_PMA_FORCE_DA_TX_FIR_C2 |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_C1 |
			   AIROHA_PCS_PMA_FORCE_DA_TX_FIR_C1,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_TX_FIR_C1 |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_TX_FIR_C1, fir_c1));

	/* Reset TX Bar */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_TX_RST_B,
			AIROHA_PCS_PMA_TXCALIB_RST_B | AIROHA_PCS_PMA_TX_TOP_RST_B);
}

static void an7581_pcs_rx_bringup(struct airoha_pcs_priv *priv,
				  phy_interface_t interface)
{
	u32 rx_rate_ctrl;
	u32 osr;
	u32 pr_cdr_beta_dac;
	u32 cdr_pr_buf_in_sr;
	bool cdr_pr_cap_en;
	u32 sigdet_vth_sel;
	u32 phyck_div, phyck_sel;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		osr = BIT(1) | BIT(0); /* 1.25G */
		pr_cdr_beta_dac = BIT(3);
		rx_rate_ctrl = 0;
		cdr_pr_cap_en = false;
		cdr_pr_buf_in_sr = BIT(2) | BIT(1) | BIT(0);
		sigdet_vth_sel = BIT(2) | BIT(1);
		phyck_div = BIT(5) | BIT(3) | BIT(0);
		phyck_sel = BIT(0);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		osr = BIT(0); /* 2.5G */
		pr_cdr_beta_dac = BIT(2) | BIT(1);
		rx_rate_ctrl = 0;
		cdr_pr_cap_en = true;
		cdr_pr_buf_in_sr = BIT(2) | BIT(1);
		sigdet_vth_sel = BIT(2) | BIT(1);
		phyck_div = BIT(3) | BIT(1) | BIT(0);
		phyck_sel = BIT(0);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		osr = 0; /* 10G */
		cdr_pr_cap_en = false;
		pr_cdr_beta_dac = BIT(3);
		rx_rate_ctrl = BIT(1);
		cdr_pr_buf_in_sr = BIT(2) | BIT(1) | BIT(0);
		sigdet_vth_sel = BIT(1);
		phyck_div = BIT(6) | BIT(1);
		phyck_sel = BIT(1);
		break;
	default:
		return;
	}

	/* Set RX rate ctrl */
	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_FLL_2,
				   AIROHA_PCS_PMA_CK_RATE,
				   AIROHA_PCS_PMA_CK_RATE_10);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_XPON_RX_RESERVED_1,
			   AIROHA_PCS_PMA_XPON_RX_RATE_CTRL,
			   FIELD_PREP(AIROHA_PCS_PMA_XPON_RX_RATE_CTRL, rx_rate_ctrl));

	/* Setup RX Path */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_FLL_5,
			   AIROHA_PCS_PMA_FLL_IDAC_MIN |
			   AIROHA_PCS_PMA_FLL_IDAC_MAX,
			   FIELD_PREP(AIROHA_PCS_PMA_FLL_IDAC_MIN, 0x400) |
			   FIELD_PREP(AIROHA_PCS_PMA_FLL_IDAC_MAX, 0x3ff));

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_DAC_D1_BYPASS_AEQ,
			AIROHA_PCS_ANA_RX_DAC_EYE_BYPASS_AEQ |
			AIROHA_PCS_ANA_RX_DAC_E1_BYPASS_AEQ |
			AIROHA_PCS_ANA_RX_DAC_E0_BYPASS_AEQ |
			AIROHA_PCS_ANA_RX_DAC_D1_BYPASS_AEQ);

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_FE_PEAKING_CTRL_MSB,
			AIROHA_PCS_ANA_RX_DAC_D0_BYPASS_AEQ);

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_FE_VCM_GEN_PWDB,
			AIROHA_PCS_ANA_FE_VCM_GEN_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_LCPLL_PWCTL_SETTING_1,
			AIROHA_PCS_PMA_LCPLL_MAN_PWDB);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_AEQ_CFORCE,
			   AIROHA_PCS_ANA_AEQ_OFORCE,
			   AIROHA_PCS_ANA_AEQ_OFORCE_CTLE);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_OSCAL_WATCH_WNDW,
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE,
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_VGA2VOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_VGA2IOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_VGA1VOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_VGA1IOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_CTLE2VOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_CTLE2IOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_CTLE1VOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_CTLE1IOS |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_LVSH |
			   AIROHA_PCS_ANA_RX_OSCAL_FORCE_COMPOS);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_DISB_MODE_4,
			  AIROHA_PCS_PMA_DISB_BLWC_OFFSET);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_EXTRAL_CTRL,
			  AIROHA_PCS_PMA_DISB_LEQ);

	regmap_clear_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PD_PICAL_CKD8_INV,
			  AIROHA_PCS_ANA_CDR_PD_EDGE_DIS |
			  AIROHA_PCS_ANA_CDR_PD_PICAL_CKD8_INV);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_AEQ_BYPASS,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_AEQ_CKON |
			   AIROHA_PCS_PMA_FORCE_DA_AEQ_CKON,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_AEQ_CKON);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_AEQ_RSTB,
			AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_INJCK_SEL |
			AIROHA_PCS_PMA_FORCE_DA_CDR_INJCK_SEL);

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PR_MONPR_EN,
			   AIROHA_PCS_ANA_RX_DAC_MON |
			   AIROHA_PCS_ANA_CDR_PR_XFICK_EN |
			   AIROHA_PCS_ANA_CDR_PR_MONDPI_EN |
			   AIROHA_PCS_ANA_CDR_PR_MONDPR_EN,
			   FIELD_PREP(AIROHA_PCS_ANA_RX_DAC_MON, 0x0) |
			   AIROHA_PCS_ANA_CDR_PR_XFICK_EN);

	/* Setup FE Gain and FE Peacking */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_FE_GAIN_CTRL,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_FE_GAIN_CTRL |
			   AIROHA_PCS_PMA_FORCE_DA_RX_FE_GAIN_CTRL,
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_RX_FE_GAIN_CTRL, 0x0));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_JCPLL_SDM_SCAN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_FE_PEAKING_CTRL |
			   AIROHA_PCS_PMA_FORCE_DA_RX_FE_PEAKING_CTRL,
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_RX_FE_PEAKING_CTRL, 0x0));

	/* Setup FE VOS */
	if (interface != PHY_INTERFACE_MODE_USXGMII &&
	    interface != PHY_INTERFACE_MODE_10GBASER)
		regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_RX_FE_VOS,
				   AIROHA_PCS_PMA_FORCE_SEL_DA_FE_VOS |
				   AIROHA_PCS_PMA_FORCE_DA_FE_VOS,
				   AIROHA_PCS_PMA_FORCE_SEL_DA_FE_VOS |
				   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_FE_VOS, 0x0));

	/* Setup FLL PR FMeter (no bypass mode)*/
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PLL_TDC_FREQDET_0,
			   AIROHA_PCS_PMA_PLL_LOCK_CYCLECNT,
			   FIELD_PREP(AIROHA_PCS_PMA_PLL_LOCK_CYCLECNT, 0x1));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PLL_TDC_FREQDET_1,
			   AIROHA_PCS_PMA_PLL_LOCK_TARGET_END |
			   AIROHA_PCS_PMA_PLL_LOCK_TARGET_BEG,
			   FIELD_PREP(AIROHA_PCS_PMA_PLL_LOCK_TARGET_END, 0xffff) |
			   FIELD_PREP(AIROHA_PCS_PMA_PLL_LOCK_TARGET_BEG, 0x0));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PLL_TDC_FREQDET_3,
			   AIROHA_PCS_PMA_PLL_LOCK_LOCKTH,
			   FIELD_PREP(AIROHA_PCS_PMA_PLL_LOCK_LOCKTH, 0x1));

	/* FIXME: Warn and Ask Airoha about typo in air_eth_xsgmii.c line 1391 */
	/* AIROHA_PCS_ANA_REV_1_FE_BUF1_BIAS_CTRL is set 0x0 in SDK but seems a typo */
	/* Setup REV */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_REV_0,
			   AIROHA_PCS_ANA_REV_1_FE_BUF1_BIAS_CTRL |
			   AIROHA_PCS_ANA_REV_1_FE_BUF2_BIAS_CTRL |
			   AIROHA_PCS_ANA_REV_1_SIGDET_ILEAK,
			   FIELD_PREP(AIROHA_PCS_ANA_REV_1_FE_BUF1_BIAS_CTRL, BIT(2)) |
			   FIELD_PREP(AIROHA_PCS_ANA_REV_1_FE_BUF2_BIAS_CTRL, BIT(2)) |
			   FIELD_PREP(AIROHA_PCS_ANA_REV_1_SIGDET_ILEAK, 0x0));

	/* Setup Rdy Timeout */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_5,
			   AIROHA_PCS_PMA_RX_RDY |
			   AIROHA_PCS_PMA_RX_BLWC_RDY_EN,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_RDY, 0xa) |
			   FIELD_PREP(AIROHA_PCS_PMA_RX_BLWC_RDY_EN, 0x5));

	/* Setup CaBoundry Init */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_0,
			   AIROHA_PCS_PMA_RX_OS_START |
			   AIROHA_PCS_PMA_OSC_SPEED_OPT,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_OS_START, 0x1) |
			   AIROHA_PCS_PMA_OSC_SPEED_OPT_0_1);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_6,
			   AIROHA_PCS_PMA_RX_OS_END,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_OS_END, 0x2));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_1,
			   AIROHA_PCS_PMA_RX_PICAL_END |
			   AIROHA_PCS_PMA_RX_PICAL_START,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_PICAL_END, 0x32) |
			   FIELD_PREP(AIROHA_PCS_PMA_RX_PICAL_START, 0x2));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_4,
			   AIROHA_PCS_PMA_RX_SDCAL_END |
			   AIROHA_PCS_PMA_RX_SDCAL_START,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_SDCAL_END, 0x32) |
			   FIELD_PREP(AIROHA_PCS_PMA_RX_SDCAL_START, 0x2));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_2,
			   AIROHA_PCS_PMA_RX_PDOS_END |
			   AIROHA_PCS_PMA_RX_PDOS_START,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_PDOS_END, 0x32) |
			   FIELD_PREP(AIROHA_PCS_PMA_RX_PDOS_START, 0x2));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_CTRL_SEQUENCE_CTRL_3,
			   AIROHA_PCS_PMA_RX_FEOS_END |
			   AIROHA_PCS_PMA_RX_FEOS_START,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_FEOS_END, 0x32) |
			   FIELD_PREP(AIROHA_PCS_PMA_RX_FEOS_START, 0x2));

	/* Setup By Serdes*/
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_AEQ_SPEED,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_OSR_SEL |
			   AIROHA_PCS_PMA_FORCE_DA_OSR_SEL,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_OSR_SEL |
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_DA_OSR_SEL, osr));

	/* Setup RX OSR */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PD_PICAL_CKD8_INV,
			   AIROHA_PCS_ANA_CDR_PD_EDGE_DIS,
			   osr ? AIROHA_PCS_ANA_CDR_PD_EDGE_DIS : 0);

	/* Setup CDR LPF Ratio */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_LPF_RATIO,
			   AIROHA_PCS_ANA_CDR_LPF_TOP_LIM |
			   AIROHA_PCS_ANA_CDR_LPF_RATIO,
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_LPF_TOP_LIM, 0x20000) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_LPF_RATIO, osr));

	/* Setup CDR PR */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PR_BETA_DAC,
			   AIROHA_PCS_ANA_CDR_PR_KBAND_DIV |
			   AIROHA_PCS_ANA_CDR_PR_BETA_SEL |
			   AIROHA_PCS_ANA_CDR_PR_VCOADC_OS |
			   AIROHA_PCS_ANA_CDR_PR_BETA_DAC,
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_KBAND_DIV, 0x4) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_BETA_SEL, 0x1) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_VCOADC_OS, 0x8) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_BETA_DAC, pr_cdr_beta_dac));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PR_VREG_IBAND_VAL,
			   AIROHA_PCS_ANA_CDR_PR_FBKSEL |
			   AIROHA_PCS_ANA_CDR_PR_DAC_BAND |
			   AIROHA_PCS_ANA_CDR_PR_VREG_CKBUF_VAL |
			   AIROHA_PCS_ANA_CDR_PR_VREG_IBAND_VAL,
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_FBKSEL, 0x0) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_DAC_BAND, pr_cdr_beta_dac) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_VREG_CKBUF_VAL, 0x6) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_PR_VREG_IBAND_VAL, 0x6));

	/* Setup Eye Mon */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PHY_EQ_CTRL_2,
			   AIROHA_PCS_PMA_EQ_DEBUG_SEL |
			   AIROHA_PCS_PMA_FOM_NUM_ORDER |
			   AIROHA_PCS_PMA_A_SEL,
			   FIELD_PREP(AIROHA_PCS_PMA_EQ_DEBUG_SEL, 0x0) |
			   FIELD_PREP(AIROHA_PCS_PMA_FOM_NUM_ORDER, 0x1) |
			   FIELD_PREP(AIROHA_PCS_PMA_A_SEL, 0x3));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_EYE_TOP_EYECNT_CTRL_2,
			   AIROHA_PCS_PMA_DATA_SHIFT |
			   AIROHA_PCS_PMA_EYECNT_FAST,
			   AIROHA_PCS_PMA_EYECNT_FAST);

	/* Calibration Start */

	/* Enable SYS */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_SYS_EN_SEL_0,
			   AIROHA_PCS_PMA_RX_SYS_EN_SEL,
			   FIELD_PREP(AIROHA_PCS_PMA_RX_SYS_EN_SEL, 0x1));

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_LCPLL_PWCTL_SETTING_0,
			AIROHA_PCS_PMA_SW_LCPLL_EN);

	udelay(500);

	/* Setup FLL PR FMeter (bypass mode)*/
	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_DISB_MODE_8,
			  AIROHA_PCS_PMA_DISB_FBCK_LOCK);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_FORCE_MODE_9,
			AIROHA_PCS_PMA_FORCE_FBCK_LOCK);

	/* Enable CMLEQ */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_FE_EQ_HZEN,
			   AIROHA_PCS_ANA_RX_FE_VB_EQ3_EN |
			   AIROHA_PCS_ANA_RX_FE_VB_EQ2_EN |
			   AIROHA_PCS_ANA_RX_FE_VB_EQ1_EN |
			   AIROHA_PCS_ANA_RX_FE_EQ_HZEN,
			   AIROHA_PCS_ANA_RX_FE_VB_EQ3_EN |
			   AIROHA_PCS_ANA_RX_FE_VB_EQ2_EN |
			   AIROHA_PCS_ANA_RX_FE_VB_EQ1_EN);

	/* Setup CDR PR */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PR_MONPR_EN,
			   AIROHA_PCS_ANA_CDR_PR_CAP_EN |
			   AIROHA_PCS_ANA_CDR_BUF_IN_SR,
			   (cdr_pr_cap_en ? AIROHA_PCS_ANA_CDR_PR_CAP_EN : 0) |
			   FIELD_PREP(AIROHA_PCS_ANA_CDR_BUF_IN_SR, cdr_pr_buf_in_sr));

	/* Setup CDR xxx Pwdb, set force and disable */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_PWDB |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PWDB |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_PIEYE_PWDB |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PIEYE_PWDB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_PWDB |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_PIEYE_PWDB);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PD_PWDB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_KBAND_RSTB |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_KBAND_RSTB |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PD_PWDB |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PD_PWDB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PD_PWDB);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_RX_FE_PWDB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_PDOSCAL_EN |
			   AIROHA_PCS_PMA_FORCE_DA_RX_PDOSCAL_EN |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_FE_PWDB |
			   AIROHA_PCS_PMA_FORCE_DA_RX_FE_PWDB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_FE_PWDB);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_RX_SCAN_RST_B,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_SIGDET_PWDB |
			   AIROHA_PCS_PMA_FORCE_DA_RX_SIGDET_PWDB |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_SCAN_RST_B |
			   AIROHA_PCS_PMA_FORCE_DA_RX_SCAN_RST_B,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_RX_SIGDET_PWDB);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_DA_XPON_PWDB_0,
			  AIROHA_PCS_PMA_XPON_CDR_PD_PWDB |
			  AIROHA_PCS_PMA_XPON_CDR_PR_PIEYE_PWDB |
			  AIROHA_PCS_PMA_XPON_CDR_PW_PWDB |
			  AIROHA_PCS_PMA_XPON_RX_FE_PWDB);

	/* FIXME: Ask Airoha WHY it's cleared? */
	/* regmap_clear_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_SIGDET_NOVTH,
	 *		  AIROHA_PCS_ANA_RX_FE_50OHMS_SEL);
	 */

	/* Setup SigDet */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_SIGDET_NOVTH,
			   AIROHA_PCS_ANA_RX_SIGDET_VTH_SEL |
			   AIROHA_PCS_ANA_RX_SIGDET_PEAK,
			   FIELD_PREP(AIROHA_PCS_ANA_RX_SIGDET_VTH_SEL, sigdet_vth_sel) |
			   FIELD_PREP(AIROHA_PCS_ANA_RX_SIGDET_PEAK, BIT(1)));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_DAC_RANGE,
			   AIROHA_PCS_ANA_RX_SIGDET_LPF_CTRL,
			   FIELD_PREP(AIROHA_PCS_ANA_RX_SIGDET_LPF_CTRL, BIT(1) | BIT(0)));

	/* Disable SigDet Pwdb */
	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_DA_XPON_PWDB_1,
			  AIROHA_PCS_PMA_RX_SIDGET_PWDB);

	/* Setup PHYCK */
	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_PHYCK_DIV,
			   AIROHA_PCS_ANA_RX_TDC_CK_SEL |
			   AIROHA_PCS_ANA_RX_PHYCK_RSTB |
			   AIROHA_PCS_ANA_RX_PHYCK_SEL |
			   AIROHA_PCS_ANA_RX_PHYCK_DIV,
			   AIROHA_PCS_ANA_RX_PHYCK_RSTB |
			   FIELD_PREP(AIROHA_PCS_ANA_RX_PHYCK_SEL, phyck_sel) |
			   FIELD_PREP(AIROHA_PCS_ANA_RX_PHYCK_DIV, phyck_div));

	regmap_update_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_RX_BUSBIT_SEL,
			   AIROHA_PCS_ANA_RX_PHY_CK_SEL_FORCE |
			   AIROHA_PCS_ANA_RX_PHY_CK_SEL,
			   AIROHA_PCS_ANA_RX_PHY_CK_SEL_FORCE);

	udelay(100);

	/* Enable CDR xxx Pwdb */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PWDB |
			AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PIEYE_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PD_PWDB,
			AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PD_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_RX_FE_PWDB,
			AIROHA_PCS_PMA_FORCE_DA_RX_FE_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_RX_SCAN_RST_B,
			AIROHA_PCS_PMA_FORCE_DA_RX_SIGDET_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_DA_XPON_PWDB_0,
			AIROHA_PCS_PMA_XPON_CDR_PD_PWDB |
			AIROHA_PCS_PMA_XPON_CDR_PR_PIEYE_PWDB |
			AIROHA_PCS_PMA_XPON_CDR_PW_PWDB |
			AIROHA_PCS_PMA_XPON_RX_FE_PWDB);

	/* Enable SigDet Pwdb */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_DA_XPON_PWDB_1,
			AIROHA_PCS_PMA_RX_SIDGET_PWDB);
}

static unsigned int an7581_pcs_apply_cdr_pr_idac(struct airoha_pcs_priv *priv,
						 u32 cdr_pr_idac)
{
	u32 val;

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_IDAC,
			   AIROHA_PCS_PMA_FORCE_CDR_PR_IDAC,
			   FIELD_PREP(AIROHA_PCS_PMA_FORCE_CDR_PR_IDAC,
				      cdr_pr_idac));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_RX_FREQ_DET_4,
			   AIROHA_PCS_PMA_FREQLOCK_DET_EN,
			   AIROHA_PCS_PMA_FREQLOCK_DET_EN_FORCE_0);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_RX_FREQ_DET_4,
			   AIROHA_PCS_PMA_FREQLOCK_DET_EN,
			   AIROHA_PCS_PMA_FREQLOCK_DET_EN_NORMAL);

	udelay(5000);

	regmap_read(priv->xfi_pma, AIROHA_PCS_PMA_RX_FREQDET, &val);

	return FIELD_GET(AIROHA_PCS_PMA_FL_OUT, val);
}

static u32 an7581_pcs_rx_prcal_idac_major(struct airoha_pcs_priv *priv,
					  u32 target_fl_out)
{
	unsigned int fl_out_diff = UINT_MAX;
	unsigned int prcal_search;
	u32 cdr_pr_idac = 0;

	for (prcal_search = 0; prcal_search < 8 ; prcal_search++) {
		unsigned int fl_out_diff_new;
		unsigned int fl_out;
		u32 cdr_pr_idac_tmp;

		/* try to find the upper value by setting the last 3 bit */
		cdr_pr_idac_tmp = FIELD_PREP(AIROHA_PCS_PMA_FORCE_CDR_PR_IDAC_MAJOR,
					     prcal_search);
		fl_out = an7581_pcs_apply_cdr_pr_idac(priv, cdr_pr_idac_tmp);

		/* Use absolute values to find the closest one to target */
		fl_out_diff_new = abs(fl_out - target_fl_out);
		dev_dbg(priv->dev, "Tested CDR Pr Idac: %x Fl Out: %x Diff: %u\n",
			cdr_pr_idac_tmp, fl_out, fl_out_diff_new);
		if (fl_out_diff_new < fl_out_diff) {
			cdr_pr_idac = cdr_pr_idac_tmp;
			fl_out_diff = fl_out_diff_new;
		}
	}

	return cdr_pr_idac;
}

static u32 an7581_pcs_rx_prcal_idac_minor(struct airoha_pcs_priv *priv, u32 target_fl_out,
					  u32 cdr_pr_idac_major)
{
	unsigned int remaining_prcal_search_bits = 0;
	u32 cdr_pr_idac = cdr_pr_idac_major;
	unsigned int fl_out, fl_out_diff;
	int best_prcal_search_bit = -1;
	int prcal_search_bit;

	fl_out = an7581_pcs_apply_cdr_pr_idac(priv, cdr_pr_idac);
	fl_out_diff = abs(fl_out - target_fl_out);

	/* Deadline search part.
	 * We start from top bits to bottom as we progressively decrease the
	 * signal.
	 */
	for (prcal_search_bit = 7; prcal_search_bit >= 0; prcal_search_bit--) {
		unsigned int fl_out_diff_new;
		u32 cdr_pr_idac_tmp;

		cdr_pr_idac_tmp = cdr_pr_idac | BIT(prcal_search_bit);
		fl_out = an7581_pcs_apply_cdr_pr_idac(priv, cdr_pr_idac_tmp);

		/* Use absolute values to find the closest one to target */
		fl_out_diff_new = abs(fl_out - target_fl_out);
		dev_dbg(priv->dev, "Tested CDR Pr Idac: %x Fl Out: %x Diff: %u\n",
			cdr_pr_idac_tmp, fl_out, fl_out_diff_new);
		if (fl_out_diff_new < fl_out_diff) {
			best_prcal_search_bit = prcal_search_bit;
			fl_out_diff = fl_out_diff_new;
		}
	}

	/* Set the idac with the best value we found and
	 * reset the search bit to start from bottom to top.
	 */
	if (best_prcal_search_bit >= 0) {
		cdr_pr_idac |= BIT(best_prcal_search_bit);
		remaining_prcal_search_bits = best_prcal_search_bit;
		prcal_search_bit = 0;
	}

	/* Fine tune part.
	 * Test remaining bits to find an even closer signal level to target
	 * by increasing the signal.
	 */
	while (remaining_prcal_search_bits) {
		unsigned int fl_out_diff_new;
		u32 cdr_pr_idac_tmp;

		cdr_pr_idac_tmp = cdr_pr_idac | BIT(prcal_search_bit);
		fl_out = an7581_pcs_apply_cdr_pr_idac(priv, cdr_pr_idac_tmp);

		/* Use absolute values to find the closest one to target */
		fl_out_diff_new = abs(fl_out - target_fl_out);
		/* Assume we found the deadline when the new absolue signal difference
		 * from target is greater than the previous and the difference is at
		 * least 10% greater between the old and new value.
		 * This is to account for signal detection level tollerance making
		 * sure we are actually over a deadline (AKA we are getting farther
		 * from target)
		 */
		dev_dbg(priv->dev, "Tested CDR Pr Idac: %x Fl Out: %x Diff: %u\n",
			cdr_pr_idac_tmp, fl_out, fl_out_diff_new);
		if (fl_out_diff_new > fl_out_diff &&
		    (abs(fl_out_diff_new - fl_out_diff) * 100) / fl_out_diff > 10) {
			/* Exit early if we are already at the deadline */
			if (prcal_search_bit == 0)
				break;

			/* We found the deadline, set the value to the previous
			 * bit, and reset the loop to fine tune with the
			 * remaining values.
			 */
			cdr_pr_idac |= BIT(prcal_search_bit - 1);
			remaining_prcal_search_bits = prcal_search_bit - 1;
			prcal_search_bit = 0;
		} else {
			/* Update the signal level diff and try the next bit */
			fl_out_diff = fl_out_diff_new;

			/* If we didn't found the deadline, set the last bit
			 * and reset the loop to fine tune with the remainig
			 * values.
			 */
			if (prcal_search_bit == remaining_prcal_search_bits - 1) {
				cdr_pr_idac |= BIT(prcal_search_bit);
				remaining_prcal_search_bits = prcal_search_bit;
				prcal_search_bit = 0;
			} else {
				prcal_search_bit++;
			}
		}
	}

	return cdr_pr_idac;
}

static void an7581_pcs_rx_prcal(struct airoha_pcs_priv *priv,
				phy_interface_t interface)
{
	u32 cdr_pr_idac_major, cdr_pr_idac;
	unsigned int fl_out, fl_out_diff;

	u32 target_fl_out;
	u32 cyclecnt;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:  /* DS_1.25G      / US_1.25G  */
	case PHY_INTERFACE_MODE_1000BASEX:
		target_fl_out = 0xa3d6;
		cyclecnt = 32767;
		break;
	case PHY_INTERFACE_MODE_2500BASEX: /* DS_9.95328G   / US_9.95328G */
		target_fl_out = 0xa000;
		cyclecnt = 20000;
		break;
	case PHY_INTERFACE_MODE_USXGMII: /* DS_10.3125G  / US_1.25G */
	case PHY_INTERFACE_MODE_10GBASER:
		target_fl_out = 0x9edf;
		cyclecnt = 32767;
		break;
	default:
		return;
	}

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SW_RST_SET,
			AIROHA_PCS_PMA_SW_REF_RST_N);

	udelay(100);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_RX_FREQ_DET_2,
			   AIROHA_PCS_PMA_LOCK_TARGET_END |
			   AIROHA_PCS_PMA_LOCK_TARGET_BEG,
			   FIELD_PREP(AIROHA_PCS_PMA_LOCK_TARGET_END, target_fl_out + 100) |
			   FIELD_PREP(AIROHA_PCS_PMA_LOCK_TARGET_BEG, target_fl_out - 100));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_RX_FREQ_DET_1,
			   AIROHA_PCS_PMA_UNLOCK_CYCLECNT |
			   AIROHA_PCS_PMA_LOCK_CYCLECNT,
			   FIELD_PREP(AIROHA_PCS_PMA_UNLOCK_CYCLECNT, cyclecnt) |
			   FIELD_PREP(AIROHA_PCS_PMA_LOCK_CYCLECNT, cyclecnt));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_RX_FREQ_DET_4,
			   AIROHA_PCS_PMA_LOCK_UNLOCKTH |
			   AIROHA_PCS_PMA_LOCK_LOCKTH,
			   FIELD_PREP(AIROHA_PCS_PMA_LOCK_UNLOCKTH, 3) |
			   FIELD_PREP(AIROHA_PCS_PMA_LOCK_LOCKTH, 3));

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SS_RX_FREQ_DET_3,
			   AIROHA_PCS_PMA_UNLOCK_TARGET_END |
			   AIROHA_PCS_PMA_UNLOCK_TARGET_BEG,
			   FIELD_PREP(AIROHA_PCS_PMA_UNLOCK_TARGET_END, target_fl_out + 100) |
			   FIELD_PREP(AIROHA_PCS_PMA_UNLOCK_TARGET_BEG, target_fl_out - 100));

	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PR_INJ_MODE,
			AIROHA_PCS_ANA_CDR_PR_INJ_FORCE_OFF);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_LPF_C_EN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_LPF_R_EN |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_LPF_R_EN |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_LPF_C_EN |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_LPF_C_EN,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_LPF_R_EN |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_PR_LPF_R_EN |
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_LPF_C_EN);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_IDAC,
			AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_IDAC);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_PWDB);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			  AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PWDB);

	/* Calibration logic:
	 * First check the major value by looping with every
	 * value in the last 3 bit of CDR_PR_IDAC.
	 * Get the signal level and save the value that is closer to
	 * the target.
	 *
	 * Then check each remaining 7 bits in search of the deadline
	 * where the signal gets farther than signal target.
	 *
	 * Finally fine tune for the remaining bits to find the one that
	 * produce the closest signal level.
	 */
	cdr_pr_idac_major = an7581_pcs_rx_prcal_idac_major(priv,  target_fl_out);

	cdr_pr_idac = an7581_pcs_rx_prcal_idac_minor(priv, target_fl_out, cdr_pr_idac_major);

	fl_out = an7581_pcs_apply_cdr_pr_idac(priv, cdr_pr_idac);
	fl_out_diff = abs(fl_out - target_fl_out);
	if (fl_out_diff > 100) {
		u32 pr_idac_major = FIELD_GET(AIROHA_PCS_PMA_FORCE_CDR_PR_IDAC_MAJOR,
					      cdr_pr_idac_major);
		unsigned int fl_out_tmp, fl_out_diff_tmp;
		u32 cdr_pr_idac_tmp;

		if (pr_idac_major > 0) {
			cdr_pr_idac_tmp = FIELD_PREP(AIROHA_PCS_PMA_FORCE_CDR_PR_IDAC_MAJOR,
						     pr_idac_major - 1);

			dev_dbg(priv->dev, "Fl Out is %d far from target %d with Pr Idac %x. Trying with Pr Idac %x.\n",
				fl_out_diff, target_fl_out, cdr_pr_idac_major, cdr_pr_idac_tmp);

			cdr_pr_idac_tmp = an7581_pcs_rx_prcal_idac_minor(priv, target_fl_out,
									 cdr_pr_idac_tmp);

			fl_out_tmp = an7581_pcs_apply_cdr_pr_idac(priv, cdr_pr_idac_tmp);
			fl_out_diff_tmp = abs(fl_out_tmp - target_fl_out);
			if (fl_out_diff_tmp < fl_out_diff) {
				fl_out = fl_out_tmp;
				fl_out_diff = fl_out_diff_tmp;
				cdr_pr_idac = cdr_pr_idac_tmp;
			}
		}
	}
	dev_dbg(priv->dev, "Selected CDR Pr Idac: %x Fl Out: %x\n", cdr_pr_idac, fl_out);
	if (fl_out_diff > 100)
		dev_dbg(priv->dev, "Fl Out is %d far from target %d on intermediate calibration.\n",
			 fl_out_diff, target_fl_out);


	/* Setup Load Band */
	regmap_clear_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CDR_PR_INJ_MODE,
			  AIROHA_PCS_ANA_CDR_PR_INJ_FORCE_OFF);

	/* Disable force of LPF C previously enabled */
	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_LPF_C_EN,
			  AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_LPF_C_EN);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_IDAC,
			  AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_IDAC);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_FLL_B,
			AIROHA_PCS_PMA_LOAD_EN);

	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_RX_FLL_1,
			   AIROHA_PCS_PMA_LPATH_IDAC,
			   FIELD_PREP(AIROHA_PCS_PMA_LPATH_IDAC, cdr_pr_idac));

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			  AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PWDB);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			AIROHA_PCS_PMA_FORCE_DA_CDR_PR_PWDB);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_PR_PIEYE_PWDB,
			  AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_PR_PWDB);

	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_SW_RST_SET,
			  AIROHA_PCS_PMA_SW_REF_RST_N);

	udelay(100);
}

/* This is used to both calibrate and lock to signal (after a previous
 * calibration) after a global reset.
 */
static void an7581_pcs_cdr_reset(struct airoha_pcs_priv *priv,
				 phy_interface_t interface, bool calibrate)
{
	/* Setup LPF L2D force and disable */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_LPF_LCK_2DATA,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_LPF_LCK2DATA |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_LPF_LCK2DATA,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_LPF_LCK2DATA);

	/* Calibrate IDAC and setup Load Band */
	if (calibrate)
		an7581_pcs_rx_prcal(priv, interface);

	/* Setup LPF RSTB force and disable */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_LPF_LCK_2DATA,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_LPF_RSTB |
			   AIROHA_PCS_PMA_FORCE_DA_CDR_LPF_RSTB,
			   AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_LPF_RSTB);

	udelay(700);

	/* Force Enable LPF RSTB */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_LPF_LCK_2DATA,
			AIROHA_PCS_PMA_FORCE_DA_CDR_LPF_RSTB);

	udelay(100);

	/* Force Enable LPF L2D */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_LPF_LCK_2DATA,
			AIROHA_PCS_PMA_FORCE_DA_CDR_LPF_LCK2DATA);

	/* Disable LPF RSTB force bit */
	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_LPF_LCK_2DATA,
			  AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_LPF_RSTB);

	/* Disable LPF L2D force bit */
	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_PXP_CDR_LPF_LCK_2DATA,
			  AIROHA_PCS_PMA_FORCE_SEL_DA_CDR_LPF_LCK2DATA);
}

static int an7581_pcs_phya_bringup(struct airoha_pcs_priv *priv,
				   phy_interface_t interface)
{
	int calibration_try = 0;
	u32 val;

	an7581_pcs_tx_bringup(priv, interface);
	an7581_pcs_rx_bringup(priv, interface);

	udelay(100);

retry_calibration:
	an7581_pcs_cdr_reset(priv, interface, true);

	/* Global reset clear */
	regmap_update_bits(priv->xfi_pma, AIROHA_PCS_PMA_SW_RST_SET,
			   AIROHA_PCS_PMA_SW_HSG_RXPCS_RST_N |
			   AIROHA_PCS_PMA_SW_HSG_TXPCS_RST_N |
			   AIROHA_PCS_PMA_SW_XFI_RXPCS_BIST_RST_N |
			   AIROHA_PCS_PMA_SW_XFI_RXPCS_RST_N |
			   AIROHA_PCS_PMA_SW_XFI_TXPCS_RST_N |
			   AIROHA_PCS_PMA_SW_TX_FIFO_RST_N |
			   AIROHA_PCS_PMA_SW_REF_RST_N |
			   AIROHA_PCS_PMA_SW_ALLPCS_RST_N |
			   AIROHA_PCS_PMA_SW_PMA_RST_N |
			   AIROHA_PCS_PMA_SW_TX_RST_N |
			   AIROHA_PCS_PMA_SW_RX_RST_N |
			   AIROHA_PCS_PMA_SW_RX_FIFO_RST_N,
			   AIROHA_PCS_PMA_SW_REF_RST_N);

	udelay(100);

	/* Global reset */
	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SW_RST_SET,
			AIROHA_PCS_PMA_SW_HSG_RXPCS_RST_N |
			AIROHA_PCS_PMA_SW_HSG_TXPCS_RST_N |
			AIROHA_PCS_PMA_SW_XFI_RXPCS_BIST_RST_N |
			AIROHA_PCS_PMA_SW_XFI_RXPCS_RST_N |
			AIROHA_PCS_PMA_SW_XFI_TXPCS_RST_N |
			AIROHA_PCS_PMA_SW_TX_FIFO_RST_N |
			AIROHA_PCS_PMA_SW_REF_RST_N |
			AIROHA_PCS_PMA_SW_ALLPCS_RST_N |
			AIROHA_PCS_PMA_SW_PMA_RST_N |
			AIROHA_PCS_PMA_SW_TX_RST_N |
			AIROHA_PCS_PMA_SW_RX_RST_N |
			AIROHA_PCS_PMA_SW_RX_FIFO_RST_N);

	udelay(5000);

	an7581_pcs_cdr_reset(priv, interface, false);

	/* It was discovered that after a global reset and auto mode gets
	 * actually enabled, the fl_out from calibration might change and
	 * might deviates a lot from the expected value it was calibrated for.
	 * To correctly work, the PCS FreqDet module needs to Lock to the fl_out
	 * (frequency level output) or no signal can correctly be transmitted.
	 * This is detected by checking the FreqDet module Lock bit.
	 *
	 * If it's detected that the FreqDet module is not locked, retry
	 * calibration. From observation on real hardware with a 10g SFP module,
	 * it required a maximum of an additional calibration to actually make
	 * the FreqDet module to lock. Try 10 times before failing to handle
	 * really strange case.
	 */
	regmap_read(priv->xfi_pma, AIROHA_PCS_PMA_RX_FREQDET, &val);
	if (!(val & AIROHA_PCS_PMA_FBCK_LOCK)) {
		if (calibration_try > AIROHA_PCS_MAX_CALIBRATION_TRY) {
			dev_err(priv->dev, "No FBCK Lock from FreqDet module after %d calibration try. PCS won't work.\n",
				AIROHA_PCS_MAX_CALIBRATION_TRY);
			return -EIO;
		}

		calibration_try++;

		dev_dbg(priv->dev, "No FBCK Lock from FreqDet module, retry calibration.\n");
		goto retry_calibration;
	}

	return 0;
}

static void an7581_pcs_pll_bringup(struct airoha_pcs_priv *priv,
				   phy_interface_t interface)
{
	an7581_pcs_jcpll_bringup(priv, interface);

	udelay(200);

	an7581_pcs_txpll_bringup(priv, interface);

	udelay(200);
}

int an7581_pcs_bringup(struct airoha_pcs_priv *priv,
				     phy_interface_t interface)
{
	/* Enable Analog Common Lane */
	regmap_set_bits(priv->xfi_ana, AIROHA_PCS_ANA_PXP_CMN_EN,
			AIROHA_PCS_ANA_CMN_EN);

	/* Setup PLL */
	an7581_pcs_pll_bringup(priv, interface);

	/* Setup PHYA */
	return an7581_pcs_phya_bringup(priv, interface);
}

void an7581_pcs_phya_link_up(struct airoha_pcs_priv *priv)
{
	/* Reset TXPCS on link up */
	regmap_clear_bits(priv->xfi_pma, AIROHA_PCS_PMA_SW_RST_SET,
			  AIROHA_PCS_PMA_SW_HSG_TXPCS_RST_N);

	udelay(100);

	regmap_set_bits(priv->xfi_pma, AIROHA_PCS_PMA_SW_RST_SET,
			AIROHA_PCS_PMA_SW_HSG_TXPCS_RST_N);
}
