/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_HDMI_H
#define _TEGRA_HDMI_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/* Register definitions for the Tegra high-definition multimedia interface */

/* High-Definition Multimedia Interface (HDMI_) regs */
struct hdmi_ctlr {
	/* Address 0x000 ~ 0x0d2 */
	uint ctxsw;						/* _CTXSW */  /* 0x00 */

	uint nv_pdisp_sor_state0;				/* _NV_PDISP_SOR_STATE0 */
	uint nv_pdisp_sor_state1;				/* _NV_PDISP_SOR_STATE1 */
	uint nv_pdisp_sor_state2;				/* _NV_PDISP_SOR_STATE2 */

	uint nv_pdisp_rg_hdcp_an_msb;				/* _NV_PDISP_RG_HDCP_AN_MSB */
	uint nv_pdisp_rg_hdcp_an_lsb;				/* _NV_PDISP_RG_HDCP_AN_LSB */
	uint nv_pdisp_rg_hdcp_cn_msb;				/* _NV_PDISP_RG_HDCP_CN_MSB */
	uint nv_pdisp_rg_hdcp_cn_lsb;				/* _NV_PDISP_RG_HDCP_CN_LSB */
	uint nv_pdisp_rg_hdcp_aksv_msb;				/* _NV_PDISP_RG_HDCP_AKSV_MSB */
	uint nv_pdisp_rg_hdcp_aksv_lsb;				/* _NV_PDISP_RG_HDCP_AKSV_LSB */
	uint nv_pdisp_rg_hdcp_bksv_msb;				/* _NV_PDISP_RG_HDCP_BKSV_MSB */
	uint nv_pdisp_rg_hdcp_bksv_lsb;				/* _NV_PDISP_RG_HDCP_BKSV_LSB */
	uint nv_pdisp_rg_hdcp_cksv_msb;				/* _NV_PDISP_RG_HDCP_CKSV_MSB */
	uint nv_pdisp_rg_hdcp_cksv_lsb;				/* _NV_PDISP_RG_HDCP_CKSV_LSB */
	uint nv_pdisp_rg_hdcp_dksv_msb;				/* _NV_PDISP_RG_HDCP_DKSV_MSB */
	uint nv_pdisp_rg_hdcp_dksv_lsb;				/* _NV_PDISP_RG_HDCP_DKSV_LSB */
	uint nv_pdisp_rg_hdcp_ctrl;				/* _NV_PDISP_RG_HDCP_CTRL */  /* 0x10 */
	uint nv_pdisp_rg_hdcp_cmode;				/* _NV_PDISP_RG_HDCP_CMODE */
	uint nv_pdisp_rg_hdcp_mprime_msb;			/* _NV_PDISP_RG_HDCP_MPRIME_MSB */
	uint nv_pdisp_rg_hdcp_mprime_lsb;			/* _NV_PDISP_RG_HDCP_MPRIME_LSB */
	uint nv_pdisp_rg_hdcp_sprime_msb;			/* _NV_PDISP_RG_HDCP_SPRIME_MSB */
	uint nv_pdisp_rg_hdcp_sprime_lsb2;			/* _NV_PDISP_RG_HDCP_SPRIME_LSB2 */
	uint nv_pdisp_rg_hdcp_sprime_lsb1;			/* _NV_PDISP_RG_HDCP_SPRIME_LSB1 */
	uint nv_pdisp_rg_hdcp_ri;				/* _NV_PDISP_RG_HDCP_RI */
	uint nv_pdisp_rg_hdcp_cs_msb;				/* _NV_PDISP_RG_HDCP_CS_MSB */
	uint nv_pdisp_rg_hdcp_cs_lsb;				/* _NV_PDISP_RG_HDCP_CS_LSB */

	uint nv_pdisp_hdmi_audio_emu0;				/* _NV_PDISP_HDMI_AUDIO_EMU0 */
	uint nv_pdisp_hdmi_audio_emu_rdata0;			/* _NV_PDISP_HDMI_AUDIO_EMU_RDATA0 */
	uint nv_pdisp_hdmi_audio_emu1;				/* _NV_PDISP_HDMI_AUDIO_EMU1 */
	uint nv_pdisp_hdmi_audio_emu2;				/* _NV_PDISP_HDMI_AUDIO_EMU2 */
	uint nv_pdisp_hdmi_audio_infoframe_ctrl;		/* _NV_PDISP_HDMI_AUDIO_INFOFRAME_CTRL */
	uint nv_pdisp_hdmi_audio_infoframe_status;		/* _NV_PDISP_HDMI_AUDIO_INFOFRAME_STATUS */
	uint nv_pdisp_hdmi_audio_infoframe_header;		/* _NV_PDISP_HDMI_AUDIO_INFOFRAME_HEADER */  /* 0x20 */
	uint nv_pdisp_hdmi_audio_infoframe_subpack0_low;	/* _NV_PDISP_HDMI_AUDIO_INFOFRAME_SUBPACK0_LOW */
	uint nv_pdisp_hdmi_audio_infoframe_subpack0_high;	/* _NV_PDISP_HDMI_AUDIO_INFOFRAME_SUBPACK0_HIGH */

	uint nv_pdisp_hdmi_avi_infoframe_ctrl;			/* _NV_PDISP_HDMI_AVI_INFOFRAME_CTRL */
	uint nv_pdisp_hdmi_avi_infoframe_status;		/* _NV_PDISP_HDMI_AVI_INFOFRAME_STATUS */
	uint nv_pdisp_hdmi_avi_infoframe_header;		/* _NV_PDISP_HDMI_AVI_INFOFRAME_HEADER */
	uint nv_pdisp_hdmi_avi_infoframe_subpack0_low;		/* _NV_PDISP_HDMI_AVI_INFOFRAME_SUBPACK0_LOW */
	uint nv_pdisp_hdmi_avi_infoframe_subpack0_high;		/* _NV_PDISP_HDMI_AVI_INFOFRAME_SUBPACK0_HIGH */
	uint nv_pdisp_hdmi_avi_infoframe_subpack1_low;		/* _NV_PDISP_HDMI_AVI_INFOFRAME_SUBPACK1_LOW */
	uint nv_pdisp_hdmi_avi_infoframe_subpack1_high;		/* _NV_PDISP_HDMI_AVI_INFOFRAME_SUBPACK1_HIGH */

	uint nv_pdisp_hdmi_generic_ctrl;			/* _NV_PDISP_HDMI_GENERIC_CTRL */
	uint nv_pdisp_hdmi_generic_status;			/* _NV_PDISP_HDMI_GENERIC_STATUS */
	uint nv_pdisp_hdmi_generic_header;			/* _NV_PDISP_HDMI_GENERIC_HEADER */
	uint nv_pdisp_hdmi_generic_subpack0_low;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK0_LOW */
	uint nv_pdisp_hdmi_generic_subpack0_high;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK0_HIGH */
	uint nv_pdisp_hdmi_generic_subpack1_low;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK1_LOW */
	uint nv_pdisp_hdmi_generic_subpack1_high;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK1_HIGH */
	uint nv_pdisp_hdmi_generic_subpack2_low;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK2_LOW */
	uint nv_pdisp_hdmi_generic_subpack2_high;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK2_HIGH */
	uint nv_pdisp_hdmi_generic_subpack3_low;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK3_LOW */
	uint nv_pdisp_hdmi_generic_subpack3_high;		/* _NV_PDISP_HDMI_GENERIC_SUBPACK3_HIGH */

	uint nv_pdisp_hdmi_acr_ctrl;				/* _NV_PDISP_HDMI_ACR_CTRL */
	uint nv_pdisp_hdmi_acr_0320_subpack_low;		/* _NV_PDISP_HDMI_ACR_0320_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_0320_subpack_high;		/* _NV_PDISP_HDMI_ACR_0320_SUBPACK_HIGH */
	uint nv_pdisp_hdmi_acr_0441_subpack_low;		/* _NV_PDISP_HDMI_ACR_0441_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_0441_subpack_high;		/* _NV_PDISP_HDMI_ACR_0441_SUBPACK_HIGH */
	uint nv_pdisp_hdmi_acr_0882_subpack_low;		/* _NV_PDISP_HDMI_ACR_0882_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_0882_subpack_high;		/* _NV_PDISP_HDMI_ACR_0882_SUBPACK_HIGH */
	uint nv_pdisp_hdmi_acr_1764_subpack_low;		/* _NV_PDISP_HDMI_ACR_1764_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_1764_subpack_high;		/* _NV_PDISP_HDMI_ACR_1764_SUBPACK_HIGH */
	uint nv_pdisp_hdmi_acr_0480_subpack_low;		/* _NV_PDISP_HDMI_ACR_0480_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_0480_subpack_high;		/* _NV_PDISP_HDMI_ACR_0480_SUBPACK_HIGH */
	uint nv_pdisp_hdmi_acr_0960_subpack_low;		/* _NV_PDISP_HDMI_ACR_0960_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_0960_subpack_high;		/* _NV_PDISP_HDMI_ACR_0960_SUBPACK_HIGH */
	uint nv_pdisp_hdmi_acr_1920_subpack_low;		/* _NV_PDISP_HDMI_ACR_1920_SUBPACK_LOW */
	uint nv_pdisp_hdmi_acr_1920_subpack_high;		/* _NV_PDISP_HDMI_ACR_1920_SUBPACK_HIGH */

	uint nv_pdisp_hdmi_ctrl;				/* _NV_PDISP_HDMI_CTRL */
	uint nv_pdisp_hdmi_vsync_keepout;			/* _NV_PDISP_HDMI_VSYNC_KEEPOUT */
	uint nv_pdisp_hdmi_vsync_window;			/* _NV_PDISP_HDMI_VSYNC_WINDOW */
	uint nv_pdisp_hdmi_gcp_ctrl;				/* _NV_PDISP_HDMI_GCP_CTRL */
	uint nv_pdisp_hdmi_gcp_status;				/* _NV_PDISP_HDMI_GCP_STATUS */
	uint nv_pdisp_hdmi_gcp_subpack;				/* _NV_PDISP_HDMI_GCP_SUBPACK */
	uint nv_pdisp_hdmi_channel_status1;			/* _NV_PDISP_HDMI_CHANNEL_STATUS1 */
	uint nv_pdisp_hdmi_channel_status2;			/* _NV_PDISP_HDMI_CHANNEL_STATUS2 */
	uint nv_pdisp_hdmi_emu0;				/* _NV_PDISP_HDMI_EMU0 */
	uint nv_pdisp_hdmi_emu1;				/* _NV_PDISP_HDMI_EMU1 */
	uint nv_pdisp_hdmi_emu1_rdata;				/* _NV_PDISP_HDMI_EMU1_RDATA */
	uint nv_pdisp_hdmi_spare;				/* _NV_PDISP_HDMI_SPARE */
	uint nv_pdisp_hdmi_spdif_chn_status1;			/* _NV_PDISP_HDMI_SPDIF_CHN_STATUS1 */
	uint nv_pdisp_hdmi_spdif_chn_status2;			/* _NV_PDISP_HDMI_SPDIF_CHN_STATUS2 */

	uint nv_pdisp_hdcprif_rom_ctrl;				/* _NV_PDISP_HDCPRIF_ROM_CTRL */

	uint unused;

	uint nv_pdisp_sor_cap;					/* _NV_PDISP_SOR_CAP */
	uint nv_pdisp_sor_pwr;					/* _NV_PDISP_SOR_PWR */
	uint nv_pdisp_sor_test;					/* _NV_PDISP_SOR_TEST */
	uint nv_pdisp_sor_pll0;					/* _NV_PDISP_SOR_PLL0 */
	uint nv_pdisp_sor_pll1;					/* _NV_PDISP_SOR_PLL1 */
	uint nv_pdisp_sor_pll2;					/* _NV_PDISP_SOR_PLL2 */
	uint nv_pdisp_sor_cstm;					/* _NV_PDISP_SOR_CSTM */
	uint nv_pdisp_sor_lvds;					/* _NV_PDISP_SOR_LVDS */
	uint nv_pdisp_sor_crca;					/* _NV_PDISP_SOR_CRCA */
	uint nv_pdisp_sor_crcb;					/* _NV_PDISP_SOR_CRCB */
	uint nv_pdisp_sor_blank;				/* _NV_PDISP_SOR_BLANK */

	uint nv_pdisp_sor_seq_ctl;				/* _NV_PDISP_SOR_SEQ_CTL */
	uint nv_pdisp_sor_seq_inst0;				/* _NV_PDISP_SOR_SEQ_INST0 */
	uint nv_pdisp_sor_seq_inst1;				/* _NV_PDISP_SOR_SEQ_INST1 */
	uint nv_pdisp_sor_seq_inst2;				/* _NV_PDISP_SOR_SEQ_INST2 */
	uint nv_pdisp_sor_seq_inst3;				/* _NV_PDISP_SOR_SEQ_INST3 */
	uint nv_pdisp_sor_seq_inst4;				/* _NV_PDISP_SOR_SEQ_INST4 */
	uint nv_pdisp_sor_seq_inst5;				/* _NV_PDISP_SOR_SEQ_INST5 */
	uint nv_pdisp_sor_seq_inst6;				/* _NV_PDISP_SOR_SEQ_INST6 */
	uint nv_pdisp_sor_seq_inst7;				/* _NV_PDISP_SOR_SEQ_INST7 */
	uint nv_pdisp_sor_seq_inst8;				/* _NV_PDISP_SOR_SEQ_INST8 */
	uint nv_pdisp_sor_seq_inst9;				/* _NV_PDISP_SOR_SEQ_INST9 */
	uint nv_pdisp_sor_seq_insta;				/* _NV_PDISP_SOR_SEQ_INSTA */
	uint nv_pdisp_sor_seq_instb;				/* _NV_PDISP_SOR_SEQ_INSTB */
	uint nv_pdisp_sor_seq_instc;				/* _NV_PDISP_SOR_SEQ_INSTC */
	uint nv_pdisp_sor_seq_instd;				/* _NV_PDISP_SOR_SEQ_INSTD */
	uint nv_pdisp_sor_seq_inste;				/* _NV_PDISP_SOR_SEQ_INSTE */
	uint nv_pdisp_sor_seq_instf;				/* _NV_PDISP_SOR_SEQ_INSTF */

	uint unused1[2];

	uint nv_pdisp_sor_vcrca0;				/* _NV_PDISP_SOR_VCRCA0 */
	uint nv_pdisp_sor_vcrca1;				/* _NV_PDISP_SOR_VCRCA1 */
	uint nv_pdisp_sor_ccrca0;				/* _NV_PDISP_SOR_CCRCA0 */
	uint nv_pdisp_sor_ccrca1;				/* _NV_PDISP_SOR_CCRCA1 */

	uint nv_pdisp_sor_edataa0;				/* _NV_PDISP_SOR_EDATAA0 */
	uint nv_pdisp_sor_edataa1;				/* _NV_PDISP_SOR_EDATAA1 */

	uint nv_pdisp_sor_counta0;				/* _NV_PDISP_SOR_COUNTA0 */
	uint nv_pdisp_sor_counta1;				/* _NV_PDISP_SOR_COUNTA1 */

	uint nv_pdisp_sor_debuga0;				/* _NV_PDISP_SOR_DEBUGA0 */
	uint nv_pdisp_sor_debuga1;				/* _NV_PDISP_SOR_DEBUGA1 */

	uint nv_pdisp_sor_trig;					/* _NV_PDISP_SOR_TRIG */
	uint nv_pdisp_sor_mscheck;				/* _NV_PDISP_SOR_MSCHECK */
	uint nv_pdisp_sor_lane_drive_current;			/* _NV_PDISP_SOR_LANE_DRIVE_CURRENT */

	uint nv_pdisp_audio_debug0;				/* _NV_PDISP_AUDIO_DEBUG0 0x7f */
	uint nv_pdisp_audio_debug1;				/* _NV_PDISP_AUDIO_DEBUG1 0x80 */
	uint nv_pdisp_audio_debug2;				/* _NV_PDISP_AUDIO_DEBUG2 0x81 */

	uint nv_pdisp_audio_fs1;				/* _NV_PDISP_AUDIO_FS1 0x82 */
	uint nv_pdisp_audio_fs2;				/* _NV_PDISP_AUDIO_FS2 */
	uint nv_pdisp_audio_fs3;				/* _NV_PDISP_AUDIO_FS3 */
	uint nv_pdisp_audio_fs4;				/* _NV_PDISP_AUDIO_FS4 */
	uint nv_pdisp_audio_fs5;				/* _NV_PDISP_AUDIO_FS5 */
	uint nv_pdisp_audio_fs6;				/* _NV_PDISP_AUDIO_FS6 */
	uint nv_pdisp_audio_fs7;				/* _NV_PDISP_AUDIO_FS7 0x88 */

	uint nv_pdisp_audio_pulse_width;			/* _NV_PDISP_AUDIO_PULSE_WIDTH */
	uint nv_pdisp_audio_threshold;				/* _NV_PDISP_AUDIO_THRESHOLD */
	uint nv_pdisp_audio_cntrl0;				/* _NV_PDISP_AUDIO_CNTRL0 */
	uint nv_pdisp_audio_n;					/* _NV_PDISP_AUDIO_N */
	uint nv_pdisp_audio_nval[7];				/* _NV_PDISP_AUDIO_NVAL */

	uint nv_pdisp_hdcprif_rom_timing;			/* _NV_PDISP_HDCPRIF_ROM_TIMING */
	uint nv_pdisp_sor_refclk;				/* _NV_PDISP_SOR_REFCLK */
	uint nv_pdisp_crc_control;				/* _NV_PDISP_CRC_CONTROL */
	uint nv_pdisp_input_control;				/* _NV_PDISP_INPUT_CONTROL */
	uint nv_pdisp_scratch;					/* _NV_PDISP_SCRATCH */
	uint nv_pdisp_pe_current;				/* _NV_PDISP_PE_CURRENT */

	uint nv_pdisp_key_ctrl;					/* _NV_PDISP_KEY_CTRL */
	uint nv_pdisp_key_debug0;				/* _NV_PDISP_KEY_DEBUG0 */
	uint nv_pdisp_key_debug1;				/* _NV_PDISP_KEY_DEBUG1 */
	uint nv_pdisp_key_debug2;				/* _NV_PDISP_KEY_DEBUG2 */
	uint nv_pdisp_key_hdcp_key_0;				/* _NV_PDISP_KEY_HDCP_KEY_0 */
	uint nv_pdisp_key_hdcp_key_1;				/* _NV_PDISP_KEY_HDCP_KEY_1 */
	uint nv_pdisp_key_hdcp_key_2;				/* _NV_PDISP_KEY_HDCP_KEY_2 */
	uint nv_pdisp_key_hdcp_key_3;				/* _NV_PDISP_KEY_HDCP_KEY_3 */
	uint nv_pdisp_key_hdcp_key_trig;			/* _NV_PDISP_KEY_HDCP_KEY_3 */
	uint nv_pdisp_key_skey_index;				/* _NV_PDISP_KEY_HDCP_KEY_3 */ /* 0xa3 */

	uint unused2[8];

	uint nv_pdisp_sor_audio_cntrl0;				/* _NV_PDISP_SOR_AUDIO_CNTRL0 */ /* 0xac */
	uint nv_pdisp_sor_audio_debug;				/* _NV_PDISP_SOR_AUDIO_DEBUG */
	uint nv_pdisp_sor_audio_spare0;				/* _NV_PDISP_SOR_AUDIO_SPARE0 */
	uint nv_pdisp_sor_audio_nval[7];			/* _NV_PDISP_SOR_AUDIO_NVAL 0xaf ~ 0xb5 */
	uint nv_pdisp_sor_audio_hda_scratch[4];			/* _NV_PDISP_SOR_AUDIO_HDA_SCRATCH 0xb6 ~ 0xb9 */
	uint nv_pdisp_sor_audio_hda_codec_scratch[2];		/* _NV_PDISP_SOR_AUDIO_HDA_CODEC_SCRATCH 0xba ~ 0xbb */

	uint nv_pdisp_sor_audio_hda_eld_bufwr;			/* _NV_PDISP_SOR_AUDIO_HDA_ELD_BUFWR */
	uint nv_pdisp_sor_audio_hda_presense;			/* _NV_PDISP_SOR_AUDIO_HDA_PRESENSE */
	uint nv_pdisp_sor_audio_hda_cp;				/* _NV_PDISP_SOR_AUDIO_HDA_CP */
	uint nv_pdisp_sor_audio_aval[8];			/* _NV_PDISP_SOR_AUDIO_AVAL */
	uint nv_pdisp_sor_audio_gen_ctrl;			/* _NV_PDISP_SOR_AUDIO_GEN_CTRL */

	uint unused3[4];

	uint nv_pdisp_int_status;				/* _NV_PDISP_INT_STATUS */
	uint nv_pdisp_int_mask;					/* _NV_PDISP_INT_MASK */
	uint nv_pdisp_int_enable;				/* _NV_PDISP_INT_ENABLE */

	uint unused4[2];

	uint nv_pdisp_sor_io_peak_current;			/* _NV_PDISP_SOR_IO_PEAK_CURRENT */
	uint nv_pdisp_sor_pad_ctls0;				/* _NV_PDISP_SOR_PAD_CTLS0 */
};

/* HDMI_NV_PDISP_SOR_STATE0	0x01 */
#define SOR_STATE_UPDATE			BIT(0)

/* HDMI_NV_PDISP_SOR_STATE1	0x02 */
#define SOR_STATE_ASY_HEAD_OPMODE_AWAKE		BIT(1)
#define SOR_STATE_ASY_ORMODE_NORMAL		BIT(2)
#define SOR_STATE_ATTACHED			BIT(3)

/* HDMI_NV_PDISP_SOR_STATE2	0x03 */
#define SOR_STATE_ASY_OWNER_NONE		(0 << 0)
#define SOR_STATE_ASY_OWNER_HEAD0		(1 << 0)
#define SOR_STATE_ASY_SUBOWNER_NONE		(0 << 4)
#define SOR_STATE_ASY_SUBOWNER_SUBHEAD0		(1 << 4)
#define SOR_STATE_ASY_SUBOWNER_SUBHEAD1		(2 << 4)
#define SOR_STATE_ASY_SUBOWNER_BOTH		(3 << 4)
#define SOR_STATE_ASY_CRCMODE_ACTIVE		(0 << 6)
#define SOR_STATE_ASY_CRCMODE_COMPLETE		(1 << 6)
#define SOR_STATE_ASY_CRCMODE_NON_ACTIVE	(2 << 6)
#define SOR_STATE_ASY_PROTOCOL_SINGLE_TMDS_A	(1 << 8)
#define SOR_STATE_ASY_PROTOCOL_CUSTOM		(15 << 8)
#define SOR_STATE_ASY_HSYNCPOL_POS		(0 << 12)
#define SOR_STATE_ASY_HSYNCPOL_NEG		(1 << 12)
#define SOR_STATE_ASY_VSYNCPOL_POS		(0 << 13)
#define SOR_STATE_ASY_VSYNCPOL_NEG		(1 << 13)
#define SOR_STATE_ASY_DEPOL_POS			(0 << 14)
#define SOR_STATE_ASY_DEPOL_NEG			(1 << 14)

#define INFOFRAME_CTRL_ENABLE			BIT(0)
#define INFOFRAME_HEADER_TYPE(x)		(((x) & 0xff) <<  0)
#define INFOFRAME_HEADER_VERSION(x)		(((x) & 0xff) <<  8)
#define INFOFRAME_HEADER_LEN(x)			(((x) & 0x0f) << 16)

/* HDMI_NV_PDISP_HDMI_GENERIC_CTRL	0x2a */
#define GENERIC_CTRL_ENABLE			BIT(0)
#define GENERIC_CTRL_OTHER			BIT(4)
#define GENERIC_CTRL_SINGLE			BIT(8)
#define GENERIC_CTRL_HBLANK			BIT(12)
#define GENERIC_CTRL_AUDIO			BIT(16)

/* HDMI_NV_PDISP_HDMI_ACR_* */
#define ACR_SUBPACK_CTS(x)			(((x) & 0xffffff) << 8)
#define ACR_SUBPACK_N(x)			(((x) & 0xffffff) << 0)
#define ACR_ENABLE				BIT(31)

/* HDMI_NV_PDISP_HDMI_CTRL	0x44 */
#define HDMI_CTRL_REKEY(x)			(((x) & 0x7f) <<  0)
#define HDMI_CTRL_MAX_AC_PACKET(x)		(((x) & 0x1f) << 16)
#define HDMI_CTRL_ENABLE			BIT(30)

/* HDMI_NV_PDISP_HDMI_VSYNC_* */
#define VSYNC_WINDOW_END(x)			(((x) & 0x3ff) <<  0)
#define VSYNC_WINDOW_START(x)			(((x) & 0x3ff) << 16)
#define VSYNC_WINDOW_ENABLE			BIT(31)

/* HDMI_NV_PDISP_HDMI_SPARE	0x4f */
#define SPARE_HW_CTS				BIT(0)
#define SPARE_FORCE_SW_CTS			BIT(1)
#define SPARE_CTS_RESET_VAL(x)			(((x) & 0x7) << 16)

/* HDMI_NV_PDISP_SOR_PWR	0x55 */
#define SOR_PWR_NORMAL_STATE_PD			(0 <<  0)
#define SOR_PWR_NORMAL_STATE_PU			(1 <<  0)
#define SOR_PWR_NORMAL_START_NORMAL		(0 <<  1)
#define SOR_PWR_NORMAL_START_ALT		(1 <<  1)
#define SOR_PWR_SAFE_STATE_PD			(0 << 16)
#define SOR_PWR_SAFE_STATE_PU			(1 << 16)
#define SOR_PWR_SETTING_NEW_DONE		(0 << 31)
#define SOR_PWR_SETTING_NEW_PENDING		(1 << 31)
#define SOR_PWR_SETTING_NEW_TRIGGER		(1 << 31)

/* HDMI_NV_PDISP_SOR_PLL0	0x57 */
#define SOR_PLL_PWR				BIT(0)
#define SOR_PLL_PDBG				BIT(1)
#define SOR_PLL_VCAPD				BIT(2)
#define SOR_PLL_PDPORT				BIT(3)
#define SOR_PLL_RESISTORSEL			BIT(4)
#define SOR_PLL_PULLDOWN			BIT(5)
#define SOR_PLL_VCOCAP(x)			(((x) & 0xf) <<  8)
#define SOR_PLL_BG_V17_S(x)			(((x) & 0xf) << 12)
#define SOR_PLL_FILTER(x)			(((x) & 0xf) << 16)
#define SOR_PLL_ICHPMP(x)			(((x) & 0xf) << 24)
#define SOR_PLL_TX_REG_LOAD(x)			(((x) & 0xf) << 28)

/* HDMI_NV_PDISP_SOR_PLL1	0x58 */
#define SOR_PLL_TMDS_TERM_ENABLE		BIT(8)
#define SOR_PLL_TMDS_TERMADJ(x)			(((x) & 0xf) <<  9)
#define SOR_PLL_LOADADJ(x)			(((x) & 0xf) << 20)
#define SOR_PLL_PE_EN				BIT(28)
#define SOR_PLL_HALF_FULL_PE			BIT(29)
#define SOR_PLL_S_D_PIN_PE			BIT(30)

/* HDMI_NV_PDISP_SOR_CSTM	0x5a */
#define SOR_CSTM_ROTCLK(x)			(((x) & 0xf) << 24)
#define SOR_CSTM_PLLDIV				BIT(21)
#define SOR_CSTM_LVDS_ENABLE			BIT(16)
#define SOR_CSTM_MODE_LVDS			(0 << 12)
#define SOR_CSTM_MODE_TMDS			(1 << 12)
#define SOR_CSTM_MODE_MASK			(3 << 12)

/* HDMI_NV_PDISP_SOR_SEQ_CTL	0x5f */
#define SOR_SEQ_PU_PC(x)			(((x) & 0xf) <<  0)
#define SOR_SEQ_PU_PC_ALT(x)			(((x) & 0xf) <<  4)
#define SOR_SEQ_PD_PC(x)			(((x) & 0xf) <<  8)
#define SOR_SEQ_PD_PC_ALT(x)			(((x) & 0xf) << 12)
#define SOR_SEQ_PC(x)				(((x) & 0xf) << 16)
#define SOR_SEQ_STATUS				BIT(28)
#define SOR_SEQ_SWITCH				BIT(30)

/* HDMI_NV_PDISP_SOR_SEQ_INST(x)	(0x60 + (x)) */
#define SOR_SEQ_INST_WAIT_TIME(x)		(((x) & 0x3ff) << 0)
#define SOR_SEQ_INST_WAIT_UNITS_VSYNC		(2 << 12)
#define SOR_SEQ_INST_HALT			(1 << 15)
#define SOR_SEQ_INST_PIN_A_LOW			(0 << 21)
#define SOR_SEQ_INST_PIN_A_HIGH			(1 << 21)
#define SOR_SEQ_INST_PIN_B_LOW			(0 << 22)
#define SOR_SEQ_INST_PIN_B_HIGH			(1 << 22)
#define SOR_SEQ_INST_DRIVE_PWM_OUT_LO		(1 << 23)

/* HDMI_NV_PDISP_SOR_LANE_DRIVE_CURRENT	0x7e */
#define DRIVE_CURRENT_LANE0(x)			(((x) & 0x3f) <<  0)
#define DRIVE_CURRENT_LANE1(x)			(((x) & 0x3f) <<  8)
#define DRIVE_CURRENT_LANE2(x)			(((x) & 0x3f) << 16)
#define DRIVE_CURRENT_LANE3(x)			(((x) & 0x3f) << 24)
#define DRIVE_CURRENT_LANE0_T114(x)		(((x) & 0x7f) <<  0)
#define DRIVE_CURRENT_LANE1_T114(x)		(((x) & 0x7f) <<  8)
#define DRIVE_CURRENT_LANE2_T114(x)		(((x) & 0x7f) << 16)
#define DRIVE_CURRENT_LANE3_T114(x)		(((x) & 0x7f) << 24)

/* Drive current list */
enum {
	DRIVE_CURRENT_1_500_mA,
	DRIVE_CURRENT_1_875_mA,
	DRIVE_CURRENT_2_250_mA,
	DRIVE_CURRENT_2_625_mA,
	DRIVE_CURRENT_3_000_mA,
	DRIVE_CURRENT_3_375_mA,
	DRIVE_CURRENT_3_750_mA,
	DRIVE_CURRENT_4_125_mA,
	DRIVE_CURRENT_4_500_mA,
	DRIVE_CURRENT_4_875_mA,
	DRIVE_CURRENT_5_250_mA,
	DRIVE_CURRENT_5_625_mA,
	DRIVE_CURRENT_6_000_mA,
	DRIVE_CURRENT_6_375_mA,
	DRIVE_CURRENT_6_750_mA,
	DRIVE_CURRENT_7_125_mA,
	DRIVE_CURRENT_7_500_mA,
	DRIVE_CURRENT_7_875_mA,
	DRIVE_CURRENT_8_250_mA,
	DRIVE_CURRENT_8_625_mA,
	DRIVE_CURRENT_9_000_mA,
	DRIVE_CURRENT_9_375_mA,
	DRIVE_CURRENT_9_750_mA,
	DRIVE_CURRENT_10_125_mA,
	DRIVE_CURRENT_10_500_mA,
	DRIVE_CURRENT_10_875_mA,
	DRIVE_CURRENT_11_250_mA,
	DRIVE_CURRENT_11_625_mA,
	DRIVE_CURRENT_12_000_mA,
	DRIVE_CURRENT_12_375_mA,
	DRIVE_CURRENT_12_750_mA,
	DRIVE_CURRENT_13_125_mA,
	DRIVE_CURRENT_13_500_mA,
	DRIVE_CURRENT_13_875_mA,
	DRIVE_CURRENT_14_250_mA,
	DRIVE_CURRENT_14_625_mA,
	DRIVE_CURRENT_15_000_mA,
	DRIVE_CURRENT_15_375_mA,
	DRIVE_CURRENT_15_750_mA,
	DRIVE_CURRENT_16_125_mA,
	DRIVE_CURRENT_16_500_mA,
	DRIVE_CURRENT_16_875_mA,
	DRIVE_CURRENT_17_250_mA,
	DRIVE_CURRENT_17_625_mA,
	DRIVE_CURRENT_18_000_mA,
	DRIVE_CURRENT_18_375_mA,
	DRIVE_CURRENT_18_750_mA,
	DRIVE_CURRENT_19_125_mA,
	DRIVE_CURRENT_19_500_mA,
	DRIVE_CURRENT_19_875_mA,
	DRIVE_CURRENT_20_250_mA,
	DRIVE_CURRENT_20_625_mA,
	DRIVE_CURRENT_21_000_mA,
	DRIVE_CURRENT_21_375_mA,
	DRIVE_CURRENT_21_750_mA,
	DRIVE_CURRENT_22_125_mA,
	DRIVE_CURRENT_22_500_mA,
	DRIVE_CURRENT_22_875_mA,
	DRIVE_CURRENT_23_250_mA,
	DRIVE_CURRENT_23_625_mA,
	DRIVE_CURRENT_24_000_mA,
	DRIVE_CURRENT_24_375_mA,
	DRIVE_CURRENT_24_750_mA,
};

/* Drive current list for T114 */
enum {
	DRIVE_CURRENT_0_000_mA_T114,
	DRIVE_CURRENT_0_400_mA_T114,
	DRIVE_CURRENT_0_800_mA_T114,
	DRIVE_CURRENT_1_200_mA_T114,
	DRIVE_CURRENT_1_600_mA_T114,
	DRIVE_CURRENT_2_000_mA_T114,
	DRIVE_CURRENT_2_400_mA_T114,
	DRIVE_CURRENT_2_800_mA_T114,
	DRIVE_CURRENT_3_200_mA_T114,
	DRIVE_CURRENT_3_600_mA_T114,
	DRIVE_CURRENT_4_000_mA_T114,
	DRIVE_CURRENT_4_400_mA_T114,
	DRIVE_CURRENT_4_800_mA_T114,
	DRIVE_CURRENT_5_200_mA_T114,
	DRIVE_CURRENT_5_600_mA_T114,
	DRIVE_CURRENT_6_000_mA_T114,
	DRIVE_CURRENT_6_400_mA_T114,
	DRIVE_CURRENT_6_800_mA_T114,
	DRIVE_CURRENT_7_200_mA_T114,
	DRIVE_CURRENT_7_600_mA_T114,
	DRIVE_CURRENT_8_000_mA_T114,
	DRIVE_CURRENT_8_400_mA_T114,
	DRIVE_CURRENT_8_800_mA_T114,
	DRIVE_CURRENT_9_200_mA_T114,
	DRIVE_CURRENT_9_600_mA_T114,
	DRIVE_CURRENT_10_000_mA_T114,
	DRIVE_CURRENT_10_400_mA_T114,
	DRIVE_CURRENT_10_800_mA_T114,
	DRIVE_CURRENT_11_200_mA_T114,
	DRIVE_CURRENT_11_600_mA_T114,
	DRIVE_CURRENT_12_000_mA_T114,
	DRIVE_CURRENT_12_400_mA_T114,
	DRIVE_CURRENT_12_800_mA_T114,
	DRIVE_CURRENT_13_200_mA_T114,
	DRIVE_CURRENT_13_600_mA_T114,
	DRIVE_CURRENT_14_000_mA_T114,
	DRIVE_CURRENT_14_400_mA_T114,
	DRIVE_CURRENT_14_800_mA_T114,
	DRIVE_CURRENT_15_200_mA_T114,
	DRIVE_CURRENT_15_600_mA_T114,
	DRIVE_CURRENT_16_000_mA_T114,
	DRIVE_CURRENT_16_400_mA_T114,
	DRIVE_CURRENT_16_800_mA_T114,
	DRIVE_CURRENT_17_200_mA_T114,
	DRIVE_CURRENT_17_600_mA_T114,
	DRIVE_CURRENT_18_000_mA_T114,
	DRIVE_CURRENT_18_400_mA_T114,
	DRIVE_CURRENT_18_800_mA_T114,
	DRIVE_CURRENT_19_200_mA_T114,
	DRIVE_CURRENT_19_600_mA_T114,
	DRIVE_CURRENT_20_000_mA_T114,
	DRIVE_CURRENT_20_400_mA_T114,
	DRIVE_CURRENT_20_800_mA_T114,
	DRIVE_CURRENT_21_200_mA_T114,
	DRIVE_CURRENT_21_600_mA_T114,
	DRIVE_CURRENT_22_000_mA_T114,
	DRIVE_CURRENT_22_400_mA_T114,
	DRIVE_CURRENT_22_800_mA_T114,
	DRIVE_CURRENT_23_200_mA_T114,
	DRIVE_CURRENT_23_600_mA_T114,
	DRIVE_CURRENT_24_000_mA_T114,
	DRIVE_CURRENT_24_400_mA_T114,
	DRIVE_CURRENT_24_800_mA_T114,
	DRIVE_CURRENT_25_200_mA_T114,
	DRIVE_CURRENT_25_400_mA_T114,
	DRIVE_CURRENT_25_800_mA_T114,
	DRIVE_CURRENT_26_200_mA_T114,
	DRIVE_CURRENT_26_600_mA_T114,
	DRIVE_CURRENT_27_000_mA_T114,
	DRIVE_CURRENT_27_400_mA_T114,
	DRIVE_CURRENT_27_800_mA_T114,
	DRIVE_CURRENT_28_200_mA_T114,
};

/* HDMI_NV_PDISP_AUDIO_FS */
#define AUDIO_FS_LOW(x)				(((x) & 0xfff) <<  0)
#define AUDIO_FS_HIGH(x)			(((x) & 0xfff) << 16)

/* HDMI_NV_PDISP_AUDIO_CNTRL0	0x8b */
#define AUDIO_CNTRL0_ERROR_TOLERANCE(x)		(((x) & 0xff) << 0)
#define AUDIO_CNTRL0_SOURCE_SELECT_AUTO		(0 << 20)
#define AUDIO_CNTRL0_SOURCE_SELECT_SPDIF	(1 << 20)
#define AUDIO_CNTRL0_SOURCE_SELECT_HDAL		(2 << 20)
#define AUDIO_CNTRL0_FRAMES_PER_BLOCK(x)	(((x) & 0xff) << 24)

/* HDMI_NV_PDISP_AUDIO_N	0x8c */
#define AUDIO_N_VALUE(x)			(((x) & 0xfffff) << 0)
#define AUDIO_N_RESETF				(1 << 20)
#define AUDIO_N_GENERATE_NORMAL			(0 << 24)
#define AUDIO_N_GENERATE_ALTERNATE		(1 << 24)

/* HDMI_NV_PDISP_SOR_REFCLK	0x95 */
#define SOR_REFCLK_DIV_INT(x)			(((x) & 0xff) << 8)
#define SOR_REFCLK_DIV_FRAC(x)			(((x) & 0x03) << 6)

/* HDMI_NV_PDISP_INPUT_CONTROL	0x97 */
#define HDMI_SRC_DISPLAYA			(0 << 0)
#define HDMI_SRC_DISPLAYB			(1 << 0)
#define ARM_VIDEO_RANGE_FULL			(0 << 1)
#define ARM_VIDEO_RANGE_LIMITED			(1 << 1)

/* HDMI_NV_PDISP_PE_CURRENT	0x99 */
#define PE_CURRENT0(x)				(((x) & 0xf) << 0)
#define PE_CURRENT1(x)				(((x) & 0xf) << 8)
#define PE_CURRENT2(x)				(((x) & 0xf) << 16)
#define PE_CURRENT3(x)				(((x) & 0xf) << 24)

enum {
	PE_CURRENT_0_0_mA,
	PE_CURRENT_0_5_mA,
	PE_CURRENT_1_0_mA,
	PE_CURRENT_1_5_mA,
	PE_CURRENT_2_0_mA,
	PE_CURRENT_2_5_mA,
	PE_CURRENT_3_0_mA,
	PE_CURRENT_3_5_mA,
	PE_CURRENT_4_0_mA,
	PE_CURRENT_4_5_mA,
	PE_CURRENT_5_0_mA,
	PE_CURRENT_5_5_mA,
	PE_CURRENT_6_0_mA,
	PE_CURRENT_6_5_mA,
	PE_CURRENT_7_0_mA,
	PE_CURRENT_7_5_mA,
};

enum {
	PE_CURRENT_0_mA_T114,
	PE_CURRENT_1_mA_T114,
	PE_CURRENT_2_mA_T114,
	PE_CURRENT_3_mA_T114,
	PE_CURRENT_4_mA_T114,
	PE_CURRENT_5_mA_T114,
	PE_CURRENT_6_mA_T114,
	PE_CURRENT_7_mA_T114,
	PE_CURRENT_8_mA_T114,
	PE_CURRENT_9_mA_T114,
	PE_CURRENT_10_mA_T114,
	PE_CURRENT_11_mA_T114,
	PE_CURRENT_12_mA_T114,
	PE_CURRENT_13_mA_T114,
	PE_CURRENT_14_mA_T114,
	PE_CURRENT_15_mA_T114,
};

/* HDMI_NV_PDISP_SOR_AUDIO_CNTRL0	0xac */
#define SOR_AUDIO_CNTRL0_SOURCE_SELECT_AUTO	(0 << 20)
#define SOR_AUDIO_CNTRL0_SOURCE_SELECT_SPDIF	(1 << 20)
#define SOR_AUDIO_CNTRL0_SOURCE_SELECT_HDAL	(2 << 20)
#define SOR_AUDIO_CNTRL0_INJECT_NULLSMPL	(1 << 29)

/* HDMI_NV_PDISP_SOR_AUDIO_SPARE0	0xae */
#define SOR_AUDIO_SPARE0_HBR_ENABLE		BIT(27)

/* HDMI_NV_PDISP_SOR_AUDIO_HDA_CODEC_SCRATCH0	0xba */
#define SOR_AUDIO_HDA_CODEC_SCRATCH0_VALID	BIT(30)
#define SOR_AUDIO_HDA_CODEC_SCRATCH0_FMT_MASK	0xffff

/* HDMI_NV_PDISP_SOR_AUDIO_HDA_PRESENSE	0xbd */
#define SOR_AUDIO_HDA_PRESENSE_VALID		BIT(1)
#define SOR_AUDIO_HDA_PRESENSE_PRESENT		BIT(0)

/* HDMI_NV_PDISP_INT_STATUS		0xcc */
#define INT_SCRATCH				BIT(3)
#define INT_CP_REQUEST				BIT(2)
#define INT_CODEC_SCRATCH1			BIT(1)
#define INT_CODEC_SCRATCH0			BIT(0)

/* HDMI_NV_PDISP_SOR_IO_PEAK_CURRENT	0xd1 */
#define PEAK_CURRENT_LANE0(x)			(((x) & 0x7f) <<  0)
#define PEAK_CURRENT_LANE1(x)			(((x) & 0x7f) <<  8)
#define PEAK_CURRENT_LANE2(x)			(((x) & 0x7f) << 16)
#define PEAK_CURRENT_LANE3(x)			(((x) & 0x7f) << 24)

enum {
	PEAK_CURRENT_0_000_mA,
	PEAK_CURRENT_0_200_mA,
	PEAK_CURRENT_0_400_mA,
	PEAK_CURRENT_0_600_mA,
	PEAK_CURRENT_0_800_mA,
	PEAK_CURRENT_1_000_mA,
	PEAK_CURRENT_1_200_mA,
	PEAK_CURRENT_1_400_mA,
	PEAK_CURRENT_1_600_mA,
	PEAK_CURRENT_1_800_mA,
	PEAK_CURRENT_2_000_mA,
	PEAK_CURRENT_2_200_mA,
	PEAK_CURRENT_2_400_mA,
	PEAK_CURRENT_2_600_mA,
	PEAK_CURRENT_2_800_mA,
	PEAK_CURRENT_3_000_mA,
	PEAK_CURRENT_3_200_mA,
	PEAK_CURRENT_3_400_mA,
	PEAK_CURRENT_3_600_mA,
	PEAK_CURRENT_3_800_mA,
	PEAK_CURRENT_4_000_mA,
	PEAK_CURRENT_4_200_mA,
	PEAK_CURRENT_4_400_mA,
	PEAK_CURRENT_4_600_mA,
	PEAK_CURRENT_4_800_mA,
	PEAK_CURRENT_5_000_mA,
	PEAK_CURRENT_5_200_mA,
	PEAK_CURRENT_5_400_mA,
	PEAK_CURRENT_5_600_mA,
	PEAK_CURRENT_5_800_mA,
	PEAK_CURRENT_6_000_mA,
	PEAK_CURRENT_6_200_mA,
	PEAK_CURRENT_6_400_mA,
	PEAK_CURRENT_6_600_mA,
	PEAK_CURRENT_6_800_mA,
	PEAK_CURRENT_7_000_mA,
	PEAK_CURRENT_7_200_mA,
	PEAK_CURRENT_7_400_mA,
	PEAK_CURRENT_7_600_mA,
	PEAK_CURRENT_7_800_mA,
	PEAK_CURRENT_8_000_mA,
	PEAK_CURRENT_8_200_mA,
	PEAK_CURRENT_8_400_mA,
	PEAK_CURRENT_8_600_mA,
	PEAK_CURRENT_8_800_mA,
	PEAK_CURRENT_9_000_mA,
	PEAK_CURRENT_9_200_mA,
	PEAK_CURRENT_9_400_mA,
};

#endif /* _TEGRA_HDMI_H */
