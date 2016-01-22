/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_HDMI_H
#define _ASM_ARCH_HDMI_H


#define HDMI_EDID_BLOCK_SIZE            128

struct rk3288_hdmi {
	u32 reserved0[0x100];
	u32 ih_fc_stat0;
	u32 ih_fc_stat1;
	u32 ih_fc_stat2;
	u32 ih_as_stat0;
	u32 ih_phy_stat0;
	u32 ih_i2cm_stat0;
	u32 ih_cec_stat0;
	u32 ih_vp_stat0;
	u32 ih_i2cmphy_stat0;
	u32 ih_ahbdmaaud_stat0;
	u32 reserved1[0x17f-0x109];
	u32 ih_mute_fc_stat0;
	u32 ih_mute_fc_stat1;
	u32 ih_mute_fc_stat2;
	u32 ih_mute_as_stat0;
	u32 ih_mute_phy_stat0;
	u32 ih_mute_i2cm_stat0;
	u32 ih_mute_cec_stat0;
	u32 ih_mute_vp_stat0;
	u32 ih_mute_i2cmphy_stat0;
	u32 ih_mute_ahbdmaaud_stat0;
	u32 reserved2[0x1fe - 0x189];
	u32 ih_mute;
	u32 tx_invid0;
	u32 tx_instuffing;
	u32 tx_gydata0;
	u32 tx_gydata1;
	u32 tx_rcrdata0;
	u32 tx_rcrdata1;
	u32 tx_bcbdata0;
	u32 tx_bcbdata1;
	u32 reserved3[0x7ff-0x207];
	u32 vp_status;
	u32 vp_pr_cd;
	u32 vp_stuff;
	u32 vp_remap;
	u32 vp_conf;
	u32 vp_stat;
	u32 vp_int;
	u32 vp_mask;
	u32 vp_pol;
	u32 reserved4[0xfff-0x808];
	u32 fc_invidconf;
	u32 fc_inhactv0;
	u32 fc_inhactv1;
	u32 fc_inhblank0;
	u32 fc_inhblank1;
	u32 fc_invactv0;
	u32 fc_invactv1;
	u32 fc_invblank;
	u32 fc_hsyncindelay0;
	u32 fc_hsyncindelay1;
	u32 fc_hsyncinwidth0;
	u32 fc_hsyncinwidth1;
	u32 fc_vsyncindelay;
	u32 fc_vsyncinwidth;
	u32 fc_infreq0;
	u32 fc_infreq1;
	u32 fc_infreq2;
	u32 fc_ctrldur;
	u32 fc_exctrldur;
	u32 fc_exctrlspac;
	u32 fc_ch0pream;
	u32 fc_ch1pream;
	u32 fc_ch2pream;
	u32 fc_aviconf3;
	u32 fc_gcp;
	u32 fc_aviconf0;
	u32 fc_aviconf1;
	u32 fc_aviconf2;
	u32 fc_avivid;
	u32 fc_avietb0;
	u32 fc_avietb1;
	u32 fc_avisbb0;
	u32 fc_avisbb1;
	u32 fc_avielb0;
	u32 fc_avielb1;
	u32 fc_avisrb0;
	u32 fc_avisrb1;
	u32 fc_audiconf0;
	u32 fc_audiconf1;
	u32 fc_audiconf2;
	u32 fc_audiconf3;
	u32 fc_vsdieeeid0;
	u32 fc_vsdsize;
	u32 reserved7[0x2fff-0x102a];
	u32 phy_conf0;
	u32 phy_tst0;
	u32 phy_tst1;
	u32 phy_tst2;
	u32 phy_stat0;
	u32 phy_int0;
	u32 phy_mask0;
	u32 phy_pol0;
	u32 reserved8[0x301f-0x3007];
	u32 phy_i2cm_slave_addr;
	u32 phy_i2cm_address_addr;
	u32 phy_i2cm_datao_1_addr;
	u32 phy_i2cm_datao_0_addr;
	u32 phy_i2cm_datai_1_addr;
	u32 phy_i2cm_datai_0_addr;
	u32 phy_i2cm_operation_addr;
	u32 phy_i2cm_int_addr;
	u32 phy_i2cm_ctlint_addr;
	u32 phy_i2cm_div_addr;
	u32 phy_i2cm_softrstz_addr;
	u32 phy_i2cm_ss_scl_hcnt_1_addr;
	u32 phy_i2cm_ss_scl_hcnt_0_addr;
	u32 phy_i2cm_ss_scl_lcnt_1_addr;
	u32 phy_i2cm_ss_scl_lcnt_0_addr;
	u32 phy_i2cm_fs_scl_hcnt_1_addr;
	u32 phy_i2cm_fs_scl_hcnt_0_addr;
	u32 phy_i2cm_fs_scl_lcnt_1_addr;
	u32 phy_i2cm_fs_scl_lcnt_0_addr;
	u32 reserved9[0x30ff-0x3032];
	u32 aud_conf0;
	u32 aud_conf1;
	u32 aud_int;
	u32 aud_conf2;
	u32 aud_int1;
	u32 reserved32[0x31ff-0x3104];
	u32 aud_n1;
	u32 aud_n2;
	u32 aud_n3;
	u32 aud_cts1;
	u32 aud_cts2;
	u32 aud_cts3;
	u32 aud_inputclkfs;
	u32 reserved12[0x3fff-0x3206];
	u32 mc_sfrdiv;
	u32 mc_clkdis;
	u32 mc_swrstz;
	u32 mc_opctrl;
	u32 mc_flowctrl;
	u32 mc_phyrstz;
	u32 mc_lockonclock;
	u32 mc_heacphy_rst;
	u32 reserved13[0x40ff-0x4007];
	u32 csc_cfg;
	u32 csc_scale;
	struct {
		u32 msb;
		u32 lsb;
	} csc_coef[3][4];
	u32 reserved17[0x7dff-0x4119];
	u32 i2cm_slave;
	u32 i2c_address;
	u32 i2cm_datao;
	u32 i2cm_datai;
	u32 i2cm_operation;
	u32 i2cm_int;
	u32 i2cm_ctlint;
	u32 i2cm_div;
	u32 i2cm_segaddr;
	u32 i2cm_softrstz;
	u32 i2cm_segptr;
	u32 i2cm_ss_scl_hcnt_1_addr;
	u32 i2cm_ss_scl_hcnt_0_addr;
	u32 i2cm_ss_scl_lcnt_1_addr;
	u32 i2cm_ss_scl_lcnt_0_addr;
	u32 i2cm_fs_scl_hcnt_1_addr;
	u32 i2cm_fs_scl_hcnt_0_addr;
	u32 i2cm_fs_scl_lcnt_1_addr;
	u32 i2cm_fs_scl_lcnt_0_addr;
	u32 reserved18[0x7e1f-0x7e12];
	u32 i2cm_buf0;
};
check_member(rk3288_hdmi, i2cm_buf0, 0x1f880);

enum {
	/* HDMI PHY registers define */
	PHY_OPMODE_PLLCFG = 0x06,
	PHY_CKCALCTRL = 0x05,
	PHY_CKSYMTXCTRL = 0x09,
	PHY_VLEVCTRL = 0x0e,
	PHY_PLLCURRCTRL = 0x10,
	PHY_PLLPHBYCTRL = 0x13,
	PHY_PLLGMPCTRL = 0x15,
	PHY_PLLCLKBISTPHASE = 0x17,
	PHY_TXTERM = 0x19,

	/* ih_phy_stat0 field values */
	HDMI_IH_PHY_STAT0_HPD = 0x1,

	/* ih_mute field values */
	HDMI_IH_MUTE_MUTE_WAKEUP_INTERRUPT = 0x2,
	HDMI_IH_MUTE_MUTE_ALL_INTERRUPT = 0x1,

	/* tx_invid0 field values */
	HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE = 0x00,
	HDMI_TX_INVID0_VIDEO_MAPPING_MASK = 0x1f,
	HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET = 0,

	/* tx_instuffing field values */
	HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE = 0x4,
	HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE = 0x2,
	HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE = 0x1,

	/* vp_pr_cd field values */
	HDMI_VP_PR_CD_COLOR_DEPTH_MASK = 0xf0,
	HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET = 4,
	HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK = 0x0f,
	HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET = 0,

	/* vp_stuff field values */
	HDMI_VP_STUFF_IDEFAULT_PHASE_MASK = 0x20,
	HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET = 5,
	HDMI_VP_STUFF_YCC422_STUFFING_MASK = 0x4,
	HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE = 0x4,
	HDMI_VP_STUFF_PP_STUFFING_MASK = 0x2,
	HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE = 0x2,
	HDMI_VP_STUFF_PR_STUFFING_MASK = 0x1,
	HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE = 0x1,

	/* vp_conf field values */
	HDMI_VP_CONF_BYPASS_EN_MASK = 0x40,
	HDMI_VP_CONF_BYPASS_EN_ENABLE = 0x40,
	HDMI_VP_CONF_PP_EN_ENMASK = 0x20,
	HDMI_VP_CONF_PP_EN_DISABLE = 0x00,
	HDMI_VP_CONF_PR_EN_MASK = 0x10,
	HDMI_VP_CONF_PR_EN_DISABLE = 0x00,
	HDMI_VP_CONF_YCC422_EN_MASK = 0x8,
	HDMI_VP_CONF_YCC422_EN_DISABLE = 0x0,
	HDMI_VP_CONF_BYPASS_SELECT_MASK = 0x4,
	HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER = 0x4,
	HDMI_VP_CONF_OUTPUT_SELECTOR_MASK = 0x3,
	HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS = 0x3,

	/* vp_remap field values */
	HDMI_VP_REMAP_YCC422_16BIT = 0x0,

	/* fc_invidconf field values */
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_MASK = 0x80,
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_ACTIVE = 0x80,
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE = 0x00,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_MASK = 0x40,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH = 0x40,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_MASK = 0x20,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH = 0x20,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_MASK = 0x10,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH = 0x10,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_DVI_MODEZ_MASK = 0x8,
	HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE = 0x8,
	HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE = 0x0,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK = 0x2,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_HIGH = 0x2,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW = 0x0,
	HDMI_FC_INVIDCONF_IN_I_P_MASK = 0x1,
	HDMI_FC_INVIDCONF_IN_I_P_INTERLACED = 0x1,
	HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE = 0x0,


	/* fc_aviconf0-fc_aviconf3 field values */
	HDMI_FC_AVICONF0_PIX_FMT_MASK = 0x03,
	HDMI_FC_AVICONF0_PIX_FMT_RGB = 0x00,
	HDMI_FC_AVICONF0_PIX_FMT_YCBCR422 = 0x01,
	HDMI_FC_AVICONF0_PIX_FMT_YCBCR444 = 0x02,
	HDMI_FC_AVICONF0_ACTIVE_FMT_MASK = 0x40,
	HDMI_FC_AVICONF0_ACTIVE_FMT_INFO_PRESENT = 0x40,
	HDMI_FC_AVICONF0_ACTIVE_FMT_NO_INFO = 0x00,
	HDMI_FC_AVICONF0_BAR_DATA_MASK = 0x0c,
	HDMI_FC_AVICONF0_BAR_DATA_NO_DATA = 0x00,
	HDMI_FC_AVICONF0_BAR_DATA_VERT_BAR = 0x04,
	HDMI_FC_AVICONF0_BAR_DATA_HORIZ_BAR = 0x08,
	HDMI_FC_AVICONF0_BAR_DATA_VERT_HORIZ_BAR = 0x0c,
	HDMI_FC_AVICONF0_SCAN_INFO_MASK = 0x30,
	HDMI_FC_AVICONF0_SCAN_INFO_OVERSCAN = 0x10,
	HDMI_FC_AVICONF0_SCAN_INFO_UNDERSCAN = 0x20,
	HDMI_FC_AVICONF0_SCAN_INFO_NODATA = 0x00,

	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_MASK = 0x0f,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_USE_CODED = 0x08,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_4_3 = 0x09,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_16_9 = 0x0a,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_14_9 = 0x0b,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_MASK = 0x30,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_NO_DATA = 0x00,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_4_3 = 0x10,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_16_9 = 0x20,
	HDMI_FC_AVICONF1_COLORIMETRY_MASK = 0xc0,
	HDMI_FC_AVICONF1_COLORIMETRY_NO_DATA = 0x00,
	HDMI_FC_AVICONF1_COLORIMETRY_SMPTE = 0x40,
	HDMI_FC_AVICONF1_COLORIMETRY_ITUR = 0x80,
	HDMI_FC_AVICONF1_COLORIMETRY_EXTENDED_INFO = 0xc0,

	HDMI_FC_AVICONF2_SCALING_MASK = 0x03,
	HDMI_FC_AVICONF2_SCALING_NONE = 0x00,
	HDMI_FC_AVICONF2_SCALING_HORIZ = 0x01,
	HDMI_FC_AVICONF2_SCALING_VERT = 0x02,
	HDMI_FC_AVICONF2_SCALING_HORIZ_vert = 0x03,
	HDMI_FC_AVICONF2_RGB_QUANT_MASK = 0x0c,
	HDMI_FC_AVICONF2_RGB_QUANT_DEFAULT = 0x00,
	HDMI_FC_AVICONF2_RGB_QUANT_LIMITED_RANGE = 0x04,
	HDMI_FC_AVICONF2_RGB_QUANT_FULL_RANGE = 0x08,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_MASK = 0x70,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC601 = 0x00,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC709 = 0x10,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_SYCC601 = 0x20,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_ADOBE_YCC601 = 0x30,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_ADOBE_RGB = 0x40,
	HDMI_FC_AVICONF2_IT_CONTENT_MASK = 0x80,
	HDMI_FC_AVICONF2_IT_CONTENT_NO_DATA = 0x00,
	HDMI_FC_AVICONF2_IT_CONTENT_VALID = 0x80,

	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_MASK = 0x03,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GRAPHICS = 0x00,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_PHOTO = 0x01,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_CINEMA = 0x02,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GAME = 0x03,
	HDMI_FC_AVICONF3_QUANT_RANGE_MASK = 0x0c,
	HDMI_FC_AVICONF3_QUANT_RANGE_LIMITED = 0x00,
	HDMI_FC_AVICONF3_QUANT_RANGE_FULL = 0x04,

	/* fc_gcp field values*/
	HDMI_FC_GCP_SET_AVMUTE = 0x02,
	HDMI_FC_GCP_CLEAR_AVMUTE = 0x01,

	/* phy_conf0 field values */
	HDMI_PHY_CONF0_PDZ_MASK = 0x80,
	HDMI_PHY_CONF0_PDZ_OFFSET = 7,
	HDMI_PHY_CONF0_ENTMDS_MASK = 0x40,
	HDMI_PHY_CONF0_ENTMDS_OFFSET = 6,
	HDMI_PHY_CONF0_SPARECTRL_MASK = 0x20,
	HDMI_PHY_CONF0_SPARECTRL_OFFSET = 5,
	HDMI_PHY_CONF0_GEN2_PDDQ_MASK = 0x10,
	HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET = 4,
	HDMI_PHY_CONF0_GEN2_TXPWRON_MASK = 0x8,
	HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET = 3,
	HDMI_PHY_CONF0_SELDATAENPOL_MASK = 0x2,
	HDMI_PHY_CONF0_SELDATAENPOL_OFFSET = 1,
	HDMI_PHY_CONF0_SELDIPIF_MASK = 0x1,
	HDMI_PHY_CONF0_SELDIPIF_OFFSET = 0,

	/* phy_tst0 field values */
	HDMI_PHY_TST0_TSTCLR_MASK = 0x20,
	HDMI_PHY_TST0_TSTCLR_OFFSET = 5,

	/* phy_stat0 field values */
	HDMI_PHY_HPD = 0x02,
	HDMI_PHY_TX_PHY_LOCK = 0x01,

	/* phy_i2cm_slave_addr field values */
	HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2 = 0x69,

	/* phy_i2cm_operation_addr field values */
	HDMI_PHY_I2CM_OPERATION_ADDR_WRITE = 0x10,

	/* hdmi_phy_i2cm_int_addr */
	HDMI_PHY_I2CM_INT_ADDR_DONE_POL = 0x08,

	/* hdmi_phy_i2cm_ctlint_addr */
	HDMI_PHY_I2CM_CTLINT_ADDR_NAC_POL = 0x80,
	HDMI_PHY_I2CM_CTLINT_ADDR_ARBITRATION_POL = 0x08,

	/* aud_conf0 field values */
	HDMI_AUD_CONF0_SW_AUDIO_FIFO_RST = 0x80,
	HDMI_AUD_CONF0_I2S_SELECT = 0x20,
	HDMI_AUD_CONF0_I2S_IN_EN_0 = 0x01,
	HDMI_AUD_CONF0_I2S_IN_EN_1 = 0x02,
	HDMI_AUD_CONF0_I2S_IN_EN_2 = 0x04,
	HDMI_AUD_CONF0_I2S_IN_EN_3 = 0x08,

	/* aud_conf0 field values */
	HDMI_AUD_CONF1_I2S_MODE_STANDARD_MODE = 0x0,
	HDMI_AUD_CONF1_I2S_WIDTH_16BIT = 0x10,

	/* aud_n3 field values */
	HDMI_AUD_N3_NCTS_ATOMIC_WRITE = 0x80,
	HDMI_AUD_N3_AUDN19_16_MASK = 0x0f,

	/* aud_cts3 field values */
	HDMI_AUD_CTS3_N_SHIFT_OFFSET = 5,
	HDMI_AUD_CTS3_N_SHIFT_MASK = 0xe0,
	HDMI_AUD_CTS3_N_SHIFT_1 = 0,
	HDMI_AUD_CTS3_N_SHIFT_16 = 0x20,
	HDMI_AUD_CTS3_N_SHIFT_32 = 0x40,
	HDMI_AUD_CTS3_N_SHIFT_64 = 0x60,
	HDMI_AUD_CTS3_N_SHIFT_128 = 0x80,
	HDMI_AUD_CTS3_N_SHIFT_256 = 0xa0,
	HDMI_AUD_CTS3_CTS_MANUAL = 0x10,
	HDMI_AUD_CTS3_AUDCTS19_16_MASK = 0x0f,

	/* aud_inputclkfs filed values */
	HDMI_AUD_INPUTCLKFS_128 = 0x0,

	/* mc_clkdis field values */
	HDMI_MC_CLKDIS_AUDCLK_DISABLE = 0x8,
	HDMI_MC_CLKDIS_TMDSCLK_DISABLE = 0x2,
	HDMI_MC_CLKDIS_PIXELCLK_DISABLE = 0x1,

	/* mc_swrstz field values */
	HDMI_MC_SWRSTZ_II2SSWRST_REQ = 0x08,
	HDMI_MC_SWRSTZ_TMDSSWRST_REQ = 0x02,

	/* mc_flowctrl field values */
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH = 0x1,
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS = 0x0,

	/* mc_phyrstz field values */
	HDMI_MC_PHYRSTZ_ASSERT = 0x0,
	HDMI_MC_PHYRSTZ_DEASSERT = 0x1,

	/* mc_heacphy_rst field values */
	HDMI_MC_HEACPHY_RST_ASSERT = 0x1,

	/* csc_cfg field values */
	HDMI_CSC_CFG_INTMODE_DISABLE = 0x00,

	/* csc_scale field values */
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK = 0xf0,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP = 0x00,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP = 0x50,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP = 0x60,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP = 0x70,
	HDMI_CSC_SCALE_CSCSCALE_MASK = 0x03,

	/* i2cm filed values */
	HDMI_I2CM_SLAVE_DDC_ADDR = 0x50,
	HDMI_I2CM_SEGADDR_DDC = 0x30,
	HDMI_I2CM_OPT_RD8_EXT = 0x8,
	HDMI_I2CM_OPT_RD8 = 0x4,
	HDMI_I2CM_DIV_FAST_STD_MODE = 0x8,
	HDMI_I2CM_DIV_FAST_MODE = 0x8,
	HDMI_I2CM_DIV_STD_MODE = 0x0,
	HDMI_I2CM_SOFTRSTZ = 0x1,
};

/*
struct display_timing;
struct rk3288_grf;

int rk_hdmi_init(struct rk3288_grf *grf, u32 vop_id);
int rk_hdmi_enable(const struct display_timing *edid);
int rk_hdmi_get_edid(struct rk3288_grf *grf, struct display_timing *edid);
*/

#endif
