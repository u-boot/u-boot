// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 AIROHA Inc
 * Author: Christian Marangi <ansuelsmth@gmail.com>
 */

#include <dm.h>
#include <dm/devres.h>
#include <linux/ethtool.h>
#include <net.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>

#include "pcs-airoha.h"

static void airoha_pcs_setup_scu_eth(struct airoha_pcs_priv *priv,
				     phy_interface_t interface)
{
	u32 xsi_sel;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_2500BASEX:
		xsi_sel = AIROHA_SCU_ETH_XSI_HSGMII;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
	default:
		xsi_sel = AIROHA_SCU_ETH_XSI_USXGMII;
	}

	regmap_update_bits(priv->scu, AIROHA_SCU_SSR3,
			   AIROHA_SCU_ETH_XSI_SEL,
			   xsi_sel);
}

static void airoha_pcs_setup_scu_pon(struct airoha_pcs_priv *priv,
				     phy_interface_t interface)
{
	u32 xsi_sel, wan_sel;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		wan_sel = AIROHA_SCU_WAN_SEL_SGMII;
		xsi_sel = AIROHA_SCU_PON_XSI_HSGMII;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		wan_sel = AIROHA_SCU_WAN_SEL_HSGMII;
		xsi_sel = AIROHA_SCU_PON_XSI_HSGMII;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
	default:
		wan_sel = AIROHA_SCU_WAN_SEL_USXGMII;
		xsi_sel = AIROHA_SCU_PON_XSI_USXGMII;
	}

	regmap_update_bits(priv->scu, AIROHA_SCU_SSTR,
			   AIROHA_SCU_PON_XSI_SEL,
			   xsi_sel);

	regmap_update_bits(priv->scu, AIROHA_SCU_WAN_CONF,
			   AIROHA_SCU_WAN_SEL,
			   wan_sel);
}

static int airoha_pcs_setup_scu(struct airoha_pcs_priv *priv,
				phy_interface_t interface)
{
	const struct airoha_pcs_match_data *data = priv->data;
	int ret;

	if (priv->xfi_rst) {
		ret = reset_assert(priv->xfi_rst);
		if (ret)
			return ret;
	}

	switch (data->port_type) {
	case AIROHA_PCS_ETH:
		airoha_pcs_setup_scu_eth(priv, interface);
		break;
	case AIROHA_PCS_PON:
		airoha_pcs_setup_scu_pon(priv, interface);
		break;
	}

	if (priv->xfi_rst) {
		ret = reset_deassert(priv->xfi_rst);
		if (ret)
			return ret;
	}

	/* TODO better handle reset from MAC */
	ret = reset_assert_bulk(&priv->rsts);
	if (ret)
		return ret;

	ret = reset_deassert_bulk(&priv->rsts);
	if (ret)
		return ret;

	return 0;
}

static void airoha_pcs_init_usxgmii(struct airoha_pcs_priv *priv)
{
	const struct airoha_pcs_match_data *data = priv->data;

	regmap_set_bits(priv->multi_sgmii, AIROHA_PCS_MULTI_SGMII_MSG_RX_CTRL_0,
			AIROHA_PCS_HSGMII_XFI_SEL);

	/* Disable Hibernation */
	if (data->hibernation_workaround)
		regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_CTROL_1,
				AIROHA_PCS_USXGMII_SPEED_SEL_H);

	/* FIXME: wait Airoha */
	/* Avoid PCS sending garbage to MAC in some HW revision (E0) */
	if (data->usxgmii_ber_time_fixup)
		regmap_write(priv->usxgmii_pcs, AIROHA_PCS_USGMII_VENDOR_DEFINE_116, 0);

	if (data->usxgmii_rx_gb_out_vld_tweak)
		regmap_clear_bits(priv->usxgmii_pcs, AN7583_PCS_USXGMII_RTL_MODIFIED,
				  AIROHA_PCS_USXGMII_MODIFIED_RX_GB_OUT_VLD);
}

static void airoha_pcs_init_hsgmii(struct airoha_pcs_priv *priv)
{
	regmap_clear_bits(priv->multi_sgmii, AIROHA_PCS_MULTI_SGMII_MSG_RX_CTRL_0,
			  AIROHA_PCS_HSGMII_XFI_SEL);

	regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_1,
			AIROHA_PCS_TBI_10B_MODE);
}

static void airoha_pcs_init_sgmii(struct airoha_pcs_priv *priv)
{
	regmap_clear_bits(priv->multi_sgmii, AIROHA_PCS_MULTI_SGMII_MSG_RX_CTRL_0,
			  AIROHA_PCS_HSGMII_XFI_SEL);

	regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_1,
			AIROHA_PCS_TBI_10B_MODE);

	regmap_update_bits(priv->hsgmii_rate_adp, AIROHA_PCS_HSGMII_RATE_ADAPT_CTRL_6,
			   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_DOUT_L,
			   FIELD_PREP(AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_DOUT_L, 0x07070707));

	regmap_update_bits(priv->hsgmii_rate_adp, AIROHA_PCS_HSGMII_RATE_ADAPT_CTRL_8,
			   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_DOUT_C,
			   FIELD_PREP(AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_DOUT_C, 0xff));
}

static void airoha_pcs_init(struct airoha_pcs_priv *priv,
			    phy_interface_t interface)
{
	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
		airoha_pcs_init_sgmii(priv);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		airoha_pcs_init_hsgmii(priv);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		airoha_pcs_init_usxgmii(priv);
		break;
	default:
		return;
	}
}

static void airoha_pcs_interrupt_init_sgmii(struct airoha_pcs_priv *priv)
{
	/* Disable every interrupt */
	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_HSGMII_PCS_HSGMII_MODE_INTERRUPT,
			  AIROHA_PCS_HSGMII_MODE2_REMOVE_FAULT_OCCUR_INT |
			  AIROHA_PCS_HSGMII_MODE2_AN_CL37_TIMERDONE_INT |
			  AIROHA_PCS_HSGMII_MODE2_AN_MIS_INT |
			  AIROHA_PCS_HSGMII_MODE2_RX_SYN_DONE_INT |
			  AIROHA_PCS_HSGMII_MODE2_AN_DONE_INT);

	/* Clear interrupt */
	regmap_set_bits(priv->usxgmii_pcs, AIROHA_PCS_HSGMII_PCS_HSGMII_MODE_INTERRUPT,
			AIROHA_PCS_HSGMII_MODE2_REMOVE_FAULT_OCCUR_INT_CLEAR |
			AIROHA_PCS_HSGMII_MODE2_AN_CL37_TIMERDONE_INT_CLEAR |
			AIROHA_PCS_HSGMII_MODE2_AN_MIS_INT_CLEAR |
			AIROHA_PCS_HSGMII_MODE2_RX_SYN_DONE_INT_CLEAR |
			AIROHA_PCS_HSGMII_MODE2_AN_DONE_INT_CLEAR);

	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_HSGMII_PCS_HSGMII_MODE_INTERRUPT,
			  AIROHA_PCS_HSGMII_MODE2_REMOVE_FAULT_OCCUR_INT_CLEAR |
			  AIROHA_PCS_HSGMII_MODE2_AN_CL37_TIMERDONE_INT_CLEAR |
			  AIROHA_PCS_HSGMII_MODE2_AN_MIS_INT_CLEAR |
			  AIROHA_PCS_HSGMII_MODE2_RX_SYN_DONE_INT_CLEAR |
			  AIROHA_PCS_HSGMII_MODE2_AN_DONE_INT_CLEAR);
}

static void airoha_pcs_interrupt_init_usxgmii(struct airoha_pcs_priv *priv)
{
	/* Disable every Interrupt */
	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_CTRL_0,
			  AIROHA_PCS_USXGMII_T_TYPE_T_INT_EN |
			  AIROHA_PCS_USXGMII_T_TYPE_D_INT_EN |
			  AIROHA_PCS_USXGMII_T_TYPE_C_INT_EN |
			  AIROHA_PCS_USXGMII_T_TYPE_S_INT_EN);

	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_CTRL_1,
			  AIROHA_PCS_USXGMII_R_TYPE_C_INT_EN |
			  AIROHA_PCS_USXGMII_R_TYPE_S_INT_EN |
			  AIROHA_PCS_USXGMII_TXPCS_FSM_ENC_ERR_INT_EN |
			  AIROHA_PCS_USXGMII_T_TYPE_E_INT_EN);

	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_CTRL_2,
			  AIROHA_PCS_USXGMII_RPCS_FSM_DEC_ERR_INT_EN |
			  AIROHA_PCS_USXGMII_R_TYPE_E_INT_EN |
			  AIROHA_PCS_USXGMII_R_TYPE_T_INT_EN |
			  AIROHA_PCS_USXGMII_R_TYPE_D_INT_EN);

	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_CTRL_3,
			  AIROHA_PCS_USXGMII_FAIL_SYNC_XOR_ST_INT_EN |
			  AIROHA_PCS_USXGMII_RX_BLOCK_LOCK_ST_INT_EN |
			  AIROHA_PCS_USXGMII_LINK_UP_ST_INT_EN |
			  AIROHA_PCS_USXGMII_HI_BER_ST_INT_EN);

	regmap_clear_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_CTRL_4,
			  AIROHA_PCS_USXGMII_LINK_DOWN_ST_INT_EN);

	/* Clear any pending interrupt */
	regmap_set_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_INT_STA_2,
			AIROHA_PCS_USXGMII_RPCS_FSM_DEC_ERR_INT |
			AIROHA_PCS_USXGMII_R_TYPE_E_INT |
			AIROHA_PCS_USXGMII_R_TYPE_T_INT |
			AIROHA_PCS_USXGMII_R_TYPE_D_INT);

	regmap_set_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_INT_STA_3,
			AIROHA_PCS_USXGMII_FAIL_SYNC_XOR_ST_INT |
			AIROHA_PCS_USXGMII_RX_BLOCK_LOCK_ST_INT |
			AIROHA_PCS_USXGMII_LINK_UP_ST_INT |
			AIROHA_PCS_USXGMII_HI_BER_ST_INT);

	regmap_set_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_INT_STA_4,
			AIROHA_PCS_USXGMII_LINK_DOWN_ST_INT);

	/* Interrupt saddly seems to be not weel supported for Link Down.
	 * PCS Poll is a must to correctly read and react on Cable Deatch
	 * as only cable attach interrupt are fired and Link Down interrupt
	 * are fired only in special case like AN restart.
	 */
}

static void airoha_pcs_interrupt_init(struct airoha_pcs_priv *priv,
				      phy_interface_t interface)
{
	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_2500BASEX:
		return airoha_pcs_interrupt_init_sgmii(priv);
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		return airoha_pcs_interrupt_init_usxgmii(priv);
	default:
		return;
	}
}

int airoha_pcs_config(struct airoha_pcs_priv *priv, bool neg_mode,
		      phy_interface_t interface,
		      const unsigned long *advertising,
		      bool permit_pause_to_mac)
{
	const struct airoha_pcs_match_data *data;
	u32 rate_adapt;
	int ret;

	priv->interface = interface;
	data = priv->data;

	/* Apply Analog and Digital configuration for PCS */
	if (data->bringup) {
		ret = data->bringup(priv, interface);
		if (ret)
			return ret;
	}

	/* Set final configuration for various modes */
	airoha_pcs_init(priv, interface);

	/* Configure Interrupt for various modes */
	airoha_pcs_interrupt_init(priv, interface);

	rate_adapt = AIROHA_PCS_HSGMII_RATE_ADAPT_RX_EN |
		     AIROHA_PCS_HSGMII_RATE_ADAPT_TX_EN;

	if (interface == PHY_INTERFACE_MODE_SGMII)
		rate_adapt |= AIROHA_PCS_HSGMII_RATE_ADAPT_RX_BYPASS |
			      AIROHA_PCS_HSGMII_RATE_ADAPT_TX_BYPASS;

	/* AN Auto Settings (Rate Adaptation) */
	regmap_update_bits(priv->hsgmii_rate_adp, AIROHA_PCS_HSGMII_RATE_ADAPT_CTRL_0,
			   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_BYPASS |
			   AIROHA_PCS_HSGMII_RATE_ADAPT_TX_BYPASS |
			   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_EN |
			   AIROHA_PCS_HSGMII_RATE_ADAPT_TX_EN, rate_adapt);

	/* FIXME: With an attached Aeonsemi PHY, AN is needed
	 * even with no inband.
	 */
	if (interface == PHY_INTERFACE_MODE_USXGMII ||
	    interface == PHY_INTERFACE_MODE_10GBASER) {
		if (interface == PHY_INTERFACE_MODE_USXGMII)
			regmap_set_bits(priv->usxgmii_pcs,
					AIROHA_PCS_USXGMII_PCS_AN_CONTROL_0,
					AIROHA_PCS_USXGMII_AN_ENABLE);
		else
			regmap_clear_bits(priv->usxgmii_pcs,
					  AIROHA_PCS_USXGMII_PCS_AN_CONTROL_0,
					  AIROHA_PCS_USXGMII_AN_ENABLE);

		if (data->usxgmii_xfi_mode_sel && neg_mode)
			regmap_set_bits(priv->usxgmii_pcs,
					AIROHA_PCS_USXGMII_PCS_AN_CONTROL_7,
					AIROHA_PCS_USXGMII_XFI_MODE_TX_SEL |
					AIROHA_PCS_USXGMII_XFI_MODE_RX_SEL);
	}

	/* Clear any force bit that my be set by bootloader */
	if (interface == PHY_INTERFACE_MODE_SGMII ||
	    interface == PHY_INTERFACE_MODE_1000BASEX ||
	    interface == PHY_INTERFACE_MODE_2500BASEX) {
		regmap_clear_bits(priv->multi_sgmii, AIROHA_PCS_MULTI_SGMII_SGMII_STS_CTRL_0,
				  AIROHA_PCS_LINK_MODE_P0 |
				  AIROHA_PCS_FORCE_SPD_MODE_P0 |
				  AIROHA_PCS_FORCE_LINKDOWN_P0 |
				  AIROHA_PCS_FORCE_LINKUP_P0);
	}

	/* Toggle Rate Adaption for SGMII/HSGMII mode */ /* TODO */
	if (interface == PHY_INTERFACE_MODE_SGMII ||
	    interface == PHY_INTERFACE_MODE_1000BASEX ||
	    interface == PHY_INTERFACE_MODE_2500BASEX) {
		if (neg_mode)
			regmap_clear_bits(priv->hsgmii_rate_adp,
					  AIROHA_PCS_HSGMII_RATE_ADP_P0_CTRL_0,
					  AIROHA_PCS_HSGMII_P0_DIS_MII_MODE);
		else
			regmap_set_bits(priv->hsgmii_rate_adp,
					AIROHA_PCS_HSGMII_RATE_ADP_P0_CTRL_0,
					AIROHA_PCS_HSGMII_P0_DIS_MII_MODE);
	}

	/* Setup SGMII AN and advertisement in DEV_ABILITY */ /* TODO */
	if (interface == PHY_INTERFACE_MODE_SGMII) {
		if (neg_mode) {
			int advertise = 0x1;

			regmap_update_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_4,
					   AIROHA_PCS_HSGMII_AN_SGMII_DEV_ABILITY,
					   FIELD_PREP(AIROHA_PCS_HSGMII_AN_SGMII_DEV_ABILITY,
						      advertise));

			regmap_set_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_0,
					AIROHA_PCS_HSGMII_AN_SGMII_RA_ENABLE);
		} else {
			regmap_clear_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_0,
					  AIROHA_PCS_HSGMII_AN_SGMII_RA_ENABLE);
		}
	}

	if (interface == PHY_INTERFACE_MODE_2500BASEX) {
		regmap_clear_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_0,
				  AIROHA_PCS_HSGMII_AN_SGMII_RA_ENABLE);

		regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_6,
				AIROHA_PCS_HSGMII_PCS_TX_ENABLE);
	}

	if (interface == PHY_INTERFACE_MODE_SGMII ||
	    interface == PHY_INTERFACE_MODE_1000BASEX) {
		u32 if_mode = AIROHA_PCS_HSGMII_AN_SIDEBAND_EN;

		/* Toggle SGMII or 1000base-x mode */
		if (interface == PHY_INTERFACE_MODE_SGMII)
			if_mode |= AIROHA_PCS_HSGMII_AN_SGMII_EN;

		if (neg_mode)
			regmap_set_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_13,
					AIROHA_PCS_HSGMII_AN_SGMII_REMOTE_FAULT_DIS);
		else
			regmap_clear_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_13,
					  AIROHA_PCS_HSGMII_AN_SGMII_REMOTE_FAULT_DIS);

		if (neg_mode) {
			/* Clear force speed bits and MAC mode */
			regmap_clear_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_6,
					  AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_10 |
					  AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_100 |
					  AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_1000 |
					  AIROHA_PCS_HSGMII_PCS_MAC_MODE |
					  AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT_VAL |
					  AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT);
		} else {
			/* Enable compatibility with MAC PCS Layer */
			if_mode |= AIROHA_PCS_HSGMII_AN_SGMII_COMPAT_EN;

			/* AN off force rate adaption, speed is set later in Link Up */
			regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_6,
					AIROHA_PCS_HSGMII_PCS_MAC_MODE |
					AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT);
		}

		regmap_update_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_13,
				   AIROHA_PCS_HSGMII_AN_SGMII_IF_MODE_5_0, if_mode);

		regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_6,
				AIROHA_PCS_HSGMII_PCS_TX_ENABLE |
				AIROHA_PCS_HSGMII_PCS_MODE2_EN);
	}

	if (interface == PHY_INTERFACE_MODE_1000BASEX &&
	    !neg_mode) {
		regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_1,
				AIROHA_PCS_SGMII_SEND_AN_ERR_EN);

		regmap_set_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_FORCE_CL37,
				AIROHA_PCS_HSGMII_AN_FORCE_AN_DONE);
	}

	/* Configure Flow Control on XFI */
	regmap_update_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			   AIROHA_PCS_XFI_TX_FC_EN | AIROHA_PCS_XFI_RX_FC_EN,
			   permit_pause_to_mac ?
				AIROHA_PCS_XFI_TX_FC_EN | AIROHA_PCS_XFI_RX_FC_EN :
				0);

	return 0;
}

void airoha_pcs_link_up(struct airoha_pcs_priv *priv, unsigned int neg_mode,
			phy_interface_t interface, int speed, int duplex)
{
	const struct airoha_pcs_match_data *data;

	data = priv->data;

	if (neg_mode) {
		if (interface == PHY_INTERFACE_MODE_SGMII) {
			regmap_update_bits(priv->hsgmii_rate_adp,
					   AIROHA_PCS_HSGMII_RATE_ADAPT_CTRL_1,
					   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_WR_THR |
					   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_RD_THR,
					   FIELD_PREP(AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_WR_THR, 0x0) |
					   FIELD_PREP(AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_RD_THR, 0x0));
			udelay(1);
			regmap_update_bits(priv->hsgmii_rate_adp,
					   AIROHA_PCS_HSGMII_RATE_ADAPT_CTRL_1,
					   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_WR_THR |
					   AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_RD_THR,
					   FIELD_PREP(AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_WR_THR, 0xf) |
					   FIELD_PREP(AIROHA_PCS_HSGMII_RATE_ADAPT_RX_AFIFO_RD_THR, 0x5));
		}
	} else {
		if (interface == PHY_INTERFACE_MODE_USXGMII ||
		    interface == PHY_INTERFACE_MODE_10GBASER) {
			u32 mode;
			u32 rate_adapt;

			switch (speed) {
			case SPEED_10000:
				rate_adapt = AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_10000;
				mode = AIROHA_PCS_USXGMII_MODE_10000;
				break;
			/* case SPEED_5000: not supported in U-Boot
				rate_adapt = AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_5000;
				mode = AIROHA_PCS_USXGMII_MODE_5000;
				break; */
			case SPEED_2500:
				rate_adapt = AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_2500;
				mode = AIROHA_PCS_USXGMII_MODE_2500;
				break;
			case SPEED_1000:
				rate_adapt = AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_1000;
				mode = AIROHA_PCS_USXGMII_MODE_1000;
				break;
			case SPEED_100:
				rate_adapt = AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_100;
				mode = AIROHA_PCS_USXGMII_MODE_100;
				break;
			}

			/* Trigger USXGMII change mode and force selected speed */
			regmap_update_bits(priv->usxgmii_pcs, AIROHA_PCS_USXGMII_PCS_AN_CONTROL_7,
					   AIROHA_PCS_USXGMII_RATE_UPDATE_MODE |
					   AIROHA_PCS_USXGMII_MODE,
					   AIROHA_PCS_USXGMII_RATE_UPDATE_MODE | mode);

			regmap_update_bits(priv->hsgmii_rate_adp, AIROHA_PCS_HSGMII_RATE_ADAPT_CTRL_11,
					   AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_EN |
					   AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE,
					   AIROHA_PCS_HSGMII_RATE_ADPT_FORCE_RATE_ADAPT_MODE_EN |
					   rate_adapt);
		}

		if (interface == PHY_INTERFACE_MODE_SGMII ||
		    interface == PHY_INTERFACE_MODE_1000BASEX) {
			u32 force_speed;
			u32 rate_adapt;

			switch (speed) {
			case SPEED_1000:
				force_speed = AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_1000;
				rate_adapt = AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT_VAL_1000;
				break;
			case SPEED_100:
				force_speed = AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_100;
				rate_adapt = AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT_VAL_100;
				break;
			case SPEED_10:
				force_speed = AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_10;
				rate_adapt = AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT_VAL_10;
				break;
			}

			regmap_update_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_CTROL_6,
					   AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_10 |
					   AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_100 |
					   AIROHA_PCS_HSGMII_PCS_SGMII_SPD_FORCE_1000 |
					   AIROHA_PCS_HSGMII_PCS_FORCE_RATEADAPT_VAL,
					   force_speed | rate_adapt);
		}

		if (interface == PHY_INTERFACE_MODE_SGMII ||
		    interface == PHY_INTERFACE_MODE_2500BASEX) {
			u32 ck_gen_mode;
			u32 speed_reg;
			u32 if_mode;

			switch (speed) {
			case SPEED_2500:
				speed_reg = AIROHA_PCS_LINK_MODE_P0_2_5G;
				break;
			case SPEED_1000:
				speed_reg = AIROHA_PCS_LINK_MODE_P0_1G;
				if_mode = AIROHA_PCS_HSGMII_AN_SPEED_FORCE_MODE_1000;
				ck_gen_mode = AIROHA_PCS_HSGMII_PCS_FORCE_CUR_SGMII_MODE_1000;
				break;
			case SPEED_100:
				speed_reg = AIROHA_PCS_LINK_MODE_P0_100M;
				if_mode = AIROHA_PCS_HSGMII_AN_SPEED_FORCE_MODE_100;
				ck_gen_mode = AIROHA_PCS_HSGMII_PCS_FORCE_CUR_SGMII_MODE_100;
				break;
			case SPEED_10:
				speed_reg = AIROHA_PCS_LINK_MODE_P0_100M;
				if_mode = AIROHA_PCS_HSGMII_AN_SPEED_FORCE_MODE_10;
				ck_gen_mode = AIROHA_PCS_HSGMII_PCS_FORCE_CUR_SGMII_MODE_10;
				break;
			}

			if (interface == PHY_INTERFACE_MODE_SGMII) {
				regmap_update_bits(priv->hsgmii_an, AIROHA_PCS_HSGMII_AN_SGMII_REG_AN_13,
						   AIROHA_PCS_HSGMII_AN_SPEED_FORCE_MODE,
						   if_mode);

				regmap_update_bits(priv->hsgmii_pcs, AIROHA_PCS_HSGMII_PCS_AN_SGMII_MODE_FORCE,
						   AIROHA_PCS_HSGMII_PCS_FORCE_CUR_SGMII_MODE |
						   AIROHA_PCS_HSGMII_PCS_FORCE_CUR_SGMII_MODE_SEL,
						   ck_gen_mode |
						   AIROHA_PCS_HSGMII_PCS_FORCE_CUR_SGMII_MODE_SEL);
			}

			regmap_update_bits(priv->multi_sgmii, AIROHA_PCS_MULTI_SGMII_SGMII_STS_CTRL_0,
					   AIROHA_PCS_LINK_MODE_P0 |
					   AIROHA_PCS_FORCE_SPD_MODE_P0,
					   speed_reg |
					   AIROHA_PCS_FORCE_SPD_MODE_P0);
		}
	}

	if (data->link_up)
		data->link_up(priv);

	/* BPI BMI enable */
	regmap_clear_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			  AIROHA_PCS_XFI_RXMPI_STOP |
			  AIROHA_PCS_XFI_RXMBI_STOP |
			  AIROHA_PCS_XFI_TXMPI_STOP |
			  AIROHA_PCS_XFI_TXMBI_STOP);
}

void airoha_pcs_link_down(struct airoha_pcs_priv *priv)
{
	/* MPI MBI disable */
	regmap_set_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			AIROHA_PCS_XFI_RXMPI_STOP |
			AIROHA_PCS_XFI_RXMBI_STOP |
			AIROHA_PCS_XFI_TXMPI_STOP |
			AIROHA_PCS_XFI_TXMBI_STOP);
}

void airoha_pcs_pre_config(struct airoha_pcs_priv *priv, phy_interface_t interface)
{
	/* Select HSGMII or USXGMII in SCU regs */
	airoha_pcs_setup_scu(priv, interface);

	/* MPI MBI disable */
	regmap_set_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			AIROHA_PCS_XFI_RXMPI_STOP |
			AIROHA_PCS_XFI_RXMBI_STOP |
			AIROHA_PCS_XFI_TXMPI_STOP |
			AIROHA_PCS_XFI_TXMBI_STOP);

	/* Write 1 to trigger reset and clear */
	regmap_clear_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_LOGIC_RST,
			  AIROHA_PCS_XFI_MAC_LOGIC_RST);
	regmap_set_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_LOGIC_RST,
			AIROHA_PCS_XFI_MAC_LOGIC_RST);

	udelay(1000);

	/* Clear XFI MAC counter */
	regmap_set_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_CNT_CLR,
			AIROHA_PCS_XFI_GLB_CNT_CLR);
}

int airoha_pcs_post_config(struct airoha_pcs_priv *priv, phy_interface_t interface)
{
	/* Frag disable */
	regmap_update_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			   AIROHA_PCS_XFI_RX_FRAG_LEN,
			   FIELD_PREP(AIROHA_PCS_XFI_RX_FRAG_LEN, 31));
	regmap_update_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			   AIROHA_PCS_XFI_TX_FRAG_LEN,
			   FIELD_PREP(AIROHA_PCS_XFI_TX_FRAG_LEN, 31));

	/* IPG NUM */
	regmap_update_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			   AIROHA_PCS_XFI_IPG_NUM,
			   FIELD_PREP(AIROHA_PCS_XFI_IPG_NUM, 10));

	/* Enable TX/RX flow control */
	regmap_set_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			AIROHA_PCS_XFI_TX_FC_EN);
	regmap_set_bits(priv->xfi_mac, AIROHA_PCS_XFI_MAC_XFI_GIB_CFG,
			AIROHA_PCS_XFI_RX_FC_EN);

	return 0;
}

static const struct regmap_config airoha_pcs_regmap_config = {
	.width = REGMAP_SIZE_32,
};

static int airoha_pcs_probe(struct udevice *dev)
{
	struct regmap_config syscon_config = airoha_pcs_regmap_config;
	struct airoha_pcs_priv *priv = dev_get_priv(dev);
	fdt_addr_t base;
	fdt_size_t size;
	int ret;

	priv->dev = dev;
	priv->data = (void *)dev_get_driver_data(dev);

	base = dev_read_addr_size_name(dev, "xfi_mac", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->xfi_mac = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->xfi_mac))
		return PTR_ERR(priv->xfi_mac);

	base = dev_read_addr_size_name(dev, "hsgmii_an", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->hsgmii_an = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->hsgmii_an))
		return PTR_ERR(priv->hsgmii_an);

	base = dev_read_addr_size_name(dev, "hsgmii_pcs", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->hsgmii_pcs = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->hsgmii_pcs))
		return PTR_ERR(priv->hsgmii_pcs);

	base = dev_read_addr_size_name(dev, "hsgmii_rate_adp", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->hsgmii_rate_adp = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->hsgmii_rate_adp))
		return PTR_ERR(priv->hsgmii_rate_adp);

	base = dev_read_addr_size_name(dev, "multi_sgmii", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->multi_sgmii = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->multi_sgmii))
		return PTR_ERR(priv->multi_sgmii);

	base = dev_read_addr_size_name(dev, "usxgmii", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->usxgmii_pcs = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->usxgmii_pcs))
		return PTR_ERR(priv->usxgmii_pcs);

	base = dev_read_addr_size_name(dev, "xfi_pma", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->xfi_pma = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->xfi_pma))
		return PTR_ERR(priv->xfi_pma);

	base = dev_read_addr_size_name(dev, "xfi_ana", &size);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	syscon_config.r_start = base;
	syscon_config.r_size = size;
	priv->xfi_ana = devm_regmap_init(dev, NULL, NULL, &syscon_config);
	if (IS_ERR(priv->xfi_ana))
		return PTR_ERR(priv->xfi_ana);

	/* SCU is used to toggle XFI or HSGMII in global SoC registers */
	priv->scu = syscon_regmap_lookup_by_phandle(dev, "airoha,scu");
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	priv->rsts.resets = devm_kcalloc(dev, AIROHA_PCS_MAX_NUM_RSTS,
					 sizeof(struct reset_ctl), GFP_KERNEL);
	if (!priv->rsts.resets)
		return -ENOMEM;
	priv->rsts.count = AIROHA_PCS_MAX_NUM_RSTS;

	ret = reset_get_by_name(dev, "mac", &priv->rsts.resets[0]);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "phy", &priv->rsts.resets[1]);
	if (ret)
		return ret;

	priv->xfi_rst = devm_reset_control_get_optional(dev, "xfi");

	return 0;
}

static const struct airoha_pcs_match_data an7581_pcs_eth = {
	.port_type = AIROHA_PCS_ETH,
	.hibernation_workaround = true,
	.usxgmii_ber_time_fixup = true,
	.bringup = an7581_pcs_bringup,
	.link_up = an7581_pcs_phya_link_up,
};

static const struct airoha_pcs_match_data an7581_pcs_pon = {
	.port_type = AIROHA_PCS_PON,
	.hibernation_workaround = true,
	.usxgmii_ber_time_fixup = true,
	.bringup = an7581_pcs_bringup,
	.link_up = an7581_pcs_phya_link_up,
};

static const struct udevice_id airoha_pcs_of_table[] = {
	{ .compatible = "airoha,an7581-pcs-eth",
	  .data = (ulong)&an7581_pcs_eth },
	{ .compatible = "airoha,an7581-pcs-pon",
	  .data = (ulong)&an7581_pcs_pon },
	{ },
};

U_BOOT_DRIVER(airoha_pcs) = {
	.name = "airoha-pcs",
	.id = UCLASS_MISC,
	.of_match = airoha_pcs_of_table,
	.probe = airoha_pcs_probe,
	.priv_auto = sizeof(struct airoha_pcs_priv),
};
