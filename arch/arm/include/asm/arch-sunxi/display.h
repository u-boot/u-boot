/*
 * Sunxi platform display controller register and constant defines
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DISPLAY_H
#define _SUNXI_DISPLAY_H

struct sunxi_de_fe_reg {
	u32 enable;			/* 0x000 */
	u32 frame_ctrl;			/* 0x004 */
	u32 bypass;			/* 0x008 */
	u32 algorithm_sel;		/* 0x00c */
	u32 line_int_ctrl;		/* 0x010 */
	u8 res0[0x0c];			/* 0x014 */
	u32 ch0_addr;			/* 0x020 */
	u32 ch1_addr;			/* 0x024 */
	u32 ch2_addr;			/* 0x028 */
	u32 field_sequence;		/* 0x02c */
	u32 ch0_offset;			/* 0x030 */
	u32 ch1_offset;			/* 0x034 */
	u32 ch2_offset;			/* 0x038 */
	u8 res1[0x04];			/* 0x03c */
	u32 ch0_stride;			/* 0x040 */
	u32 ch1_stride;			/* 0x044 */
	u32 ch2_stride;			/* 0x048 */
	u32 input_fmt;			/* 0x04c */
	u32 ch3_addr;			/* 0x050 */
	u32 ch4_addr;			/* 0x054 */
	u32 ch5_addr;			/* 0x058 */
	u32 output_fmt;			/* 0x05c */
	u32 int_enable;			/* 0x060 */
	u32 int_status;			/* 0x064 */
	u32 status;			/* 0x068 */
	u8 res2[0x04];			/* 0x06c */
	u32 csc_coef00;			/* 0x070 */
	u32 csc_coef01;			/* 0x074 */
	u32 csc_coef02;			/* 0x078 */
	u32 csc_coef03;			/* 0x07c */
	u32 csc_coef10;			/* 0x080 */
	u32 csc_coef11;			/* 0x084 */
	u32 csc_coef12;			/* 0x088 */
	u32 csc_coef13;			/* 0x08c */
	u32 csc_coef20;			/* 0x090 */
	u32 csc_coef21;			/* 0x094 */
	u32 csc_coef22;			/* 0x098 */
	u32 csc_coef23;			/* 0x09c */
	u32 deinterlace_ctrl;		/* 0x0a0 */
	u32 deinterlace_diag;		/* 0x0a4 */
	u32 deinterlace_tempdiff;	/* 0x0a8 */
	u32 deinterlace_sawtooth;	/* 0x0ac */
	u32 deinterlace_spatcomp;	/* 0x0b0 */
	u32 deinterlace_burstlen;	/* 0x0b4 */
	u32 deinterlace_preluma;	/* 0x0b8 */
	u32 deinterlace_tile_addr;	/* 0x0bc */
	u32 deinterlace_tile_stride;	/* 0x0c0 */
	u8 res3[0x0c];			/* 0x0c4 */
	u32 wb_stride_enable;		/* 0x0d0 */
	u32 ch3_stride;			/* 0x0d4 */
	u32 ch4_stride;			/* 0x0d8 */
	u32 ch5_stride;			/* 0x0dc */
	u32 fe_3d_ctrl;			/* 0x0e0 */
	u32 fe_3d_ch0_addr;		/* 0x0e4 */
	u32 fe_3d_ch1_addr;		/* 0x0e8 */
	u32 fe_3d_ch2_addr;		/* 0x0ec */
	u32 fe_3d_ch0_offset;		/* 0x0f0 */
	u32 fe_3d_ch1_offset;		/* 0x0f4 */
	u32 fe_3d_ch2_offset;		/* 0x0f8 */
	u8 res4[0x04];			/* 0x0fc */
	u32 ch0_insize;			/* 0x100 */
	u32 ch0_outsize;		/* 0x104 */
	u32 ch0_horzfact;		/* 0x108 */
	u32 ch0_vertfact;		/* 0x10c */
	u32 ch0_horzphase;		/* 0x110 */
	u32 ch0_vertphase0;		/* 0x114 */
	u32 ch0_vertphase1;		/* 0x118 */
	u8 res5[0x04];			/* 0x11c */
	u32 ch0_horztapoffset0;		/* 0x120 */
	u32 ch0_horztapoffset1;		/* 0x124 */
	u32 ch0_verttapoffset;		/* 0x128 */
	u8 res6[0xd4];			/* 0x12c */
	u32 ch1_insize;			/* 0x200 */
	u32 ch1_outsize;		/* 0x204 */
	u32 ch1_horzfact;		/* 0x208 */
	u32 ch1_vertfact;		/* 0x20c */
	u32 ch1_horzphase;		/* 0x210 */
	u32 ch1_vertphase0;		/* 0x214 */
	u32 ch1_vertphase1;		/* 0x218 */
	u8 res7[0x04];			/* 0x21c */
	u32 ch1_horztapoffset0;		/* 0x220 */
	u32 ch1_horztapoffset1;		/* 0x224 */
	u32 ch1_verttapoffset;		/* 0x228 */
	u8 res8[0x1d4];			/* 0x22c */
	u32 ch0_horzcoef0[32];		/* 0x400 */
	u32 ch0_horzcoef1[32];		/* 0x480 */
	u32 ch0_vertcoef[32];		/* 0x500 */
	u8 res9[0x80];			/* 0x580 */
	u32 ch1_horzcoef0[32];		/* 0x600 */
	u32 ch1_horzcoef1[32];		/* 0x680 */
	u32 ch1_vertcoef[32];		/* 0x700 */
	u8 res10[0x280];		/* 0x780 */
	u32 vpp_enable;			/* 0xa00 */
	u32 vpp_dcti;			/* 0xa04 */
	u32 vpp_lp1;			/* 0xa08 */
	u32 vpp_lp2;			/* 0xa0c */
	u32 vpp_wle;			/* 0xa10 */
	u32 vpp_ble;			/* 0xa14 */
};

struct sunxi_de_be_reg {
	u8 res0[0x800];			/* 0x000 */
	u32 mode;			/* 0x800 */
	u32 backcolor;			/* 0x804 */
	u32 disp_size;			/* 0x808 */
	u8 res1[0x4];			/* 0x80c */
	u32 layer0_size;		/* 0x810 */
	u32 layer1_size;		/* 0x814 */
	u32 layer2_size;		/* 0x818 */
	u32 layer3_size;		/* 0x81c */
	u32 layer0_pos;			/* 0x820 */
	u32 layer1_pos;			/* 0x824 */
	u32 layer2_pos;			/* 0x828 */
	u32 layer3_pos;			/* 0x82c */
	u8 res2[0x10];			/* 0x830 */
	u32 layer0_stride;		/* 0x840 */
	u32 layer1_stride;		/* 0x844 */
	u32 layer2_stride;		/* 0x848 */
	u32 layer3_stride;		/* 0x84c */
	u32 layer0_addr_low32b;		/* 0x850 */
	u32 layer1_addr_low32b;		/* 0x854 */
	u32 layer2_addr_low32b;		/* 0x858 */
	u32 layer3_addr_low32b;		/* 0x85c */
	u32 layer0_addr_high4b;		/* 0x860 */
	u32 layer1_addr_high4b;		/* 0x864 */
	u32 layer2_addr_high4b;		/* 0x868 */
	u32 layer3_addr_high4b;		/* 0x86c */
	u32 reg_ctrl;			/* 0x870 */
	u8 res3[0xc];			/* 0x874 */
	u32 color_key_max;		/* 0x880 */
	u32 color_key_min;		/* 0x884 */
	u32 color_key_config;		/* 0x888 */
	u8 res4[0x4];			/* 0x88c */
	u32 layer0_attr0_ctrl;		/* 0x890 */
	u32 layer1_attr0_ctrl;		/* 0x894 */
	u32 layer2_attr0_ctrl;		/* 0x898 */
	u32 layer3_attr0_ctrl;		/* 0x89c */
	u32 layer0_attr1_ctrl;		/* 0x8a0 */
	u32 layer1_attr1_ctrl;		/* 0x8a4 */
	u32 layer2_attr1_ctrl;		/* 0x8a8 */
	u32 layer3_attr1_ctrl;		/* 0x8ac */
	u8 res5[0x110];			/* 0x8b0 */
	u32 output_color_ctrl;		/* 0x9c0 */
	u8 res6[0xc];			/* 0x9c4 */
	u32 output_color_coef[12];	/* 0x9d0 */
};

struct sunxi_hdmi_reg {
	u32 version_id;			/* 0x000 */
	u32 ctrl;			/* 0x004 */
	u32 irq;			/* 0x008 */
	u32 hpd;			/* 0x00c */
	u32 video_ctrl;			/* 0x010 */
	u32 video_size;			/* 0x014 */
	u32 video_bp;			/* 0x018 */
	u32 video_fp;			/* 0x01c */
	u32 video_spw;			/* 0x020 */
	u32 video_polarity;		/* 0x024 */
	u8 res0[0x58];			/* 0x028 */
	u8 avi_info_frame[0x14];	/* 0x080 */
	u8 res1[0x4c];			/* 0x094 */
	u32 qcp_packet0;		/* 0x0e0 */
	u32 qcp_packet1;		/* 0x0e4 */
	u8 res2[0x118];			/* 0x0e8 */
	u32 pad_ctrl0;			/* 0x200 */
	u32 pad_ctrl1;			/* 0x204 */
	u32 pll_ctrl;			/* 0x208 */
	u32 pll_dbg0;			/* 0x20c */
	u32 pll_dbg1;			/* 0x210 */
	u32 hpd_cec;			/* 0x214 */
	u8 res3[0x28];			/* 0x218 */
	u8 vendor_info_frame[0x14];	/* 0x240 */
	u8 res4[0x9c];			/* 0x254 */
	u32 pkt_ctrl0;			/* 0x2f0 */
	u32 pkt_ctrl1;			/* 0x2f4 */
	u8 res5[0x8];			/* 0x2f8 */
	u32 unknown;			/* 0x300 */
	u8 res6[0xc];			/* 0x304 */
	u32 audio_sample_count;		/* 0x310 */
	u8 res7[0xec];			/* 0x314 */
	u32 audio_tx_fifo;		/* 0x400 */
	u8 res8[0xfc];			/* 0x404 */
#ifndef CONFIG_MACH_SUN6I
	u32 ddc_ctrl;			/* 0x500 */
	u32 ddc_addr;			/* 0x504 */
	u32 ddc_int_mask;		/* 0x508 */
	u32 ddc_int_status;		/* 0x50c */
	u32 ddc_fifo_ctrl;		/* 0x510 */
	u32 ddc_fifo_status;		/* 0x514 */
	u32 ddc_fifo_data;		/* 0x518 */
	u32 ddc_byte_count;		/* 0x51c */
	u32 ddc_cmnd;			/* 0x520 */
	u32 ddc_exreg;			/* 0x524 */
	u32 ddc_clock;			/* 0x528 */
	u8 res9[0x14];			/* 0x52c */
	u32 ddc_line_ctrl;		/* 0x540 */
#else
	u32 ddc_ctrl;			/* 0x500 */
	u32 ddc_exreg;			/* 0x504 */
	u32 ddc_cmnd;			/* 0x508 */
	u32 ddc_addr;			/* 0x50c */
	u32 ddc_int_mask;		/* 0x510 */
	u32 ddc_int_status;		/* 0x514 */
	u32 ddc_fifo_ctrl;		/* 0x518 */
	u32 ddc_fifo_status;		/* 0x51c */
	u32 ddc_clock;			/* 0x520 */
	u32 ddc_timeout;		/* 0x524 */
	u8 res9[0x18];			/* 0x528 */
	u32 ddc_dbg;			/* 0x540 */
	u8 res10[0x3c];			/* 0x544 */
	u32 ddc_fifo_data;		/* 0x580 */
#endif
};

/*
 * This is based on the A10s User Manual, and the A10s only supports
 * composite video and not vga like the A10 / A20 does, still other
 * than the removed vga out capability the tvencoder seems to be the same.
 * "unknown#" registers are registers which are used in the A10 kernel code,
 * but not documented in the A10s User Manual.
 */
struct sunxi_tve_reg {
	u32 gctrl;			/* 0x000 */
	u32 cfg0;			/* 0x004 */
	u32 dac_cfg0;			/* 0x008 */
	u32 filter;			/* 0x00c */
	u32 chroma_freq;		/* 0x010 */
	u32 porch_num;			/* 0x014 */
	u32 unknown0;			/* 0x018 */
	u32 line_num;			/* 0x01c */
	u32 blank_black_level;		/* 0x020 */
	u32 unknown1;			/* 0x024, seems to be 1 byte per dac */
	u8 res0[0x08];			/* 0x028 */
	u32 auto_detect_en;		/* 0x030 */
	u32 auto_detect_int_status;	/* 0x034 */
	u32 auto_detect_status;		/* 0x038 */
	u32 auto_detect_debounce;	/* 0x03c */
	u32 csc_reg0;			/* 0x040 */
	u32 csc_reg1;			/* 0x044 */
	u32 csc_reg2;			/* 0x048 */
	u32 csc_reg3;			/* 0x04c */
	u8 res1[0xb0];			/* 0x050 */
	u32 color_burst;		/* 0x100 */
	u32 vsync_num;			/* 0x104 */
	u32 notch_freq;			/* 0x108 */
	u32 cbr_level;			/* 0x10c */
	u32 burst_phase;		/* 0x110 */
	u32 burst_width;		/* 0x114 */
	u32 unknown2;			/* 0x118 */
	u32 sync_vbi_level;		/* 0x11c */
	u32 white_level;		/* 0x120 */
	u32 active_num;			/* 0x124 */
	u32 chroma_bw_gain;		/* 0x128 */
	u32 notch_width;		/* 0x12c */
	u32 resync_num;			/* 0x130 */
	u32 slave_para;			/* 0x134 */
	u32 cfg1;			/* 0x138 */
	u32 cfg2;			/* 0x13c */
};

/*
 * DE-FE register constants.
 */
#define SUNXI_DE_FE_WIDTH(x)			(((x) - 1) << 0)
#define SUNXI_DE_FE_HEIGHT(y)			(((y) - 1) << 16)
#define SUNXI_DE_FE_FACTOR_INT(n)		((n) << 16)
#define SUNXI_DE_FE_ENABLE_EN			(1 << 0)
#define SUNXI_DE_FE_FRAME_CTRL_REG_RDY		(1 << 0)
#define SUNXI_DE_FE_FRAME_CTRL_COEF_RDY		(1 << 1)
#define SUNXI_DE_FE_FRAME_CTRL_FRM_START	(1 << 16)
#define SUNXI_DE_FE_BYPASS_CSC_BYPASS		(1 << 1)
#define SUNXI_DE_FE_INPUT_FMT_ARGB8888		0x00000151
#define SUNXI_DE_FE_OUTPUT_FMT_ARGB8888		0x00000002

/*
 * DE-BE register constants.
 */
#define SUNXI_DE_BE_WIDTH(x)			(((x) - 1) << 0)
#define SUNXI_DE_BE_HEIGHT(y)			(((y) - 1) << 16)
#define SUNXI_DE_BE_MODE_ENABLE			(1 << 0)
#define SUNXI_DE_BE_MODE_START			(1 << 1)
#define SUNXI_DE_BE_MODE_DEFLICKER_ENABLE	(1 << 4)
#define SUNXI_DE_BE_MODE_LAYER0_ENABLE		(1 << 8)
#define SUNXI_DE_BE_MODE_INTERLACE_ENABLE	(1 << 28)
#define SUNXI_DE_BE_LAYER_STRIDE(x)		((x) << 5)
#define SUNXI_DE_BE_REG_CTRL_LOAD_REGS		(1 << 0)
#define SUNXI_DE_BE_LAYER_ATTR0_SRC_FE0		0x00000002
#define SUNXI_DE_BE_LAYER_ATTR1_FMT_XRGB8888	(0x09 << 8)
#define SUNXI_DE_BE_OUTPUT_COLOR_CTRL_ENABLE	1

/*
 * HDMI register constants.
 */
#define SUNXI_HDMI_X(x)				(((x) - 1) << 0)
#define SUNXI_HDMI_Y(y)				(((y) - 1) << 16)
#define SUNXI_HDMI_CTRL_ENABLE			(1 << 31)
#define SUNXI_HDMI_IRQ_STATUS_FIFO_UF		(1 << 0)
#define SUNXI_HDMI_IRQ_STATUS_FIFO_OF		(1 << 1)
#define SUNXI_HDMI_IRQ_STATUS_BITS		0x73
#define SUNXI_HDMI_HPD_DETECT			(1 << 0)
#define SUNXI_HDMI_VIDEO_CTRL_ENABLE		(1 << 31)
#define SUNXI_HDMI_VIDEO_CTRL_HDMI		(1 << 30)
#define SUNXI_HDMI_VIDEO_POL_HOR		(1 << 0)
#define SUNXI_HDMI_VIDEO_POL_VER		(1 << 1)
#define SUNXI_HDMI_VIDEO_POL_TX_CLK		(0x3e0 << 16)
#define SUNXI_HDMI_QCP_PACKET0			3
#define SUNXI_HDMI_QCP_PACKET1			0

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HDMI_PAD_CTRL0_HDP		0x7e80000f
#define SUNXI_HDMI_PAD_CTRL0_RUN		0x7e8000ff
#else
#define SUNXI_HDMI_PAD_CTRL0_HDP		0xfe800000
#define SUNXI_HDMI_PAD_CTRL0_RUN		0xfe800000
#endif

#ifdef CONFIG_MACH_SUN4I
#define SUNXI_HDMI_PAD_CTRL1			0x00d8c820
#elif defined CONFIG_MACH_SUN6I
#define SUNXI_HDMI_PAD_CTRL1			0x01ded030
#else
#define SUNXI_HDMI_PAD_CTRL1			0x00d8c830
#endif
#define SUNXI_HDMI_PAD_CTRL1_HALVE		(1 << 6)

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HDMI_PLL_CTRL			0xba48a308
#define SUNXI_HDMI_PLL_CTRL_DIV(n)		(((n) - 1) << 4)
#else
#define SUNXI_HDMI_PLL_CTRL			0xfa4ef708
#define SUNXI_HDMI_PLL_CTRL_DIV(n)		((n) << 4)
#endif
#define SUNXI_HDMI_PLL_CTRL_DIV_MASK		(0xf << 4)

#define SUNXI_HDMI_PLL_DBG0_PLL3		(0 << 21)
#define SUNXI_HDMI_PLL_DBG0_PLL7		(1 << 21)

#define SUNXI_HDMI_PKT_CTRL0			0x00000f21
#define SUNXI_HDMI_PKT_CTRL1			0x0000000f
#define SUNXI_HDMI_UNKNOWN_INPUT_SYNC		0x08000000

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HMDI_DDC_CTRL_ENABLE		(1 << 0)
#define SUNXI_HMDI_DDC_CTRL_SCL_ENABLE		(1 << 4)
#define SUNXI_HMDI_DDC_CTRL_SDA_ENABLE		(1 << 6)
#define SUNXI_HMDI_DDC_CTRL_START		(1 << 27)
#define SUNXI_HMDI_DDC_CTRL_RESET		(1 << 31)
#else
#define SUNXI_HMDI_DDC_CTRL_RESET		(1 << 0)
/* sun4i / sun5i / sun7i do not have a separate line_ctrl reg */
#define SUNXI_HMDI_DDC_CTRL_SDA_ENABLE		0
#define SUNXI_HMDI_DDC_CTRL_SCL_ENABLE		0
#define SUNXI_HMDI_DDC_CTRL_START		(1 << 30)
#define SUNXI_HMDI_DDC_CTRL_ENABLE		(1 << 31)
#endif

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HMDI_DDC_ADDR_SLAVE_ADDR		(0xa0 << 0)
#else
#define SUNXI_HMDI_DDC_ADDR_SLAVE_ADDR		(0x50 << 0)
#endif
#define SUNXI_HMDI_DDC_ADDR_OFFSET(n)		(((n) & 0xff) << 8)
#define SUNXI_HMDI_DDC_ADDR_EDDC_ADDR		(0x60 << 16)
#define SUNXI_HMDI_DDC_ADDR_EDDC_SEGMENT(n)	((n) << 24)

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HDMI_DDC_FIFO_CTRL_CLEAR		(1 << 15)
#else
#define SUNXI_HDMI_DDC_FIFO_CTRL_CLEAR		(1 << 31)
#endif

#define SUNXI_HDMI_DDC_CMND_EXPLICIT_EDDC_READ	6
#define SUNXI_HDMI_DDC_CMND_IMPLICIT_EDDC_READ	7

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HDMI_DDC_CLOCK			0x61
#else
/* N = 5,M=1 Fscl= Ftmds/2/10/2^N/(M+1) */
#define SUNXI_HDMI_DDC_CLOCK			0x0d
#endif

#define SUNXI_HMDI_DDC_LINE_CTRL_SCL_ENABLE	(1 << 8)
#define SUNXI_HMDI_DDC_LINE_CTRL_SDA_ENABLE	(1 << 9)

/*
 * TVE register constants.
 */
#define SUNXI_TVE_GCTRL_ENABLE			(1 << 0)
/*
 * Select input 0 to disable dac, 1 - 4 to feed dac from tve0, 5 - 8 to feed
 * dac from tve1. When using tve1 the mux value must be written to both tve0's
 * and tve1's gctrl reg.
 */
#define SUNXI_TVE_GCTRL_DAC_INPUT_MASK(dac)	(0xf << (((dac) + 1) * 4))
#define SUNXI_TVE_GCTRL_DAC_INPUT(dac, sel)	((sel) << (((dac) + 1) * 4))
#define SUNXI_TVE_CFG0_VGA			0x20000000
#define SUNXI_TVE_CFG0_PAL			0x07030001
#define SUNXI_TVE_CFG0_NTSC			0x07030000
#define SUNXI_TVE_DAC_CFG0_VGA			0x403e1ac7
#ifdef CONFIG_MACH_SUN5I
#define SUNXI_TVE_DAC_CFG0_COMPOSITE		0x433f0009
#else
#define SUNXI_TVE_DAC_CFG0_COMPOSITE		0x403f0008
#endif
#define SUNXI_TVE_FILTER_COMPOSITE		0x00000120
#define SUNXI_TVE_CHROMA_FREQ_PAL_M		0x21e6efe3
#define SUNXI_TVE_CHROMA_FREQ_PAL_NC		0x21f69446
#define SUNXI_TVE_PORCH_NUM_PAL			0x008a0018
#define SUNXI_TVE_PORCH_NUM_NTSC		0x00760020
#define SUNXI_TVE_LINE_NUM_PAL			0x00160271
#define SUNXI_TVE_LINE_NUM_NTSC			0x0016020d
#define SUNXI_TVE_BLANK_BLACK_LEVEL_PAL		0x00fc00fc
#define SUNXI_TVE_BLANK_BLACK_LEVEL_NTSC	0x00f0011a
#define SUNXI_TVE_UNKNOWN1_VGA			0x00000000
#define SUNXI_TVE_UNKNOWN1_COMPOSITE		0x18181818
#define SUNXI_TVE_AUTO_DETECT_EN_DET_EN(dac)	(1 << ((dac) + 0))
#define SUNXI_TVE_AUTO_DETECT_EN_INT_EN(dac)	(1 << ((dac) + 16))
#define SUNXI_TVE_AUTO_DETECT_INT_STATUS(dac)	(1 << ((dac) + 0))
#define SUNXI_TVE_AUTO_DETECT_STATUS_SHIFT(dac)	((dac) * 8)
#define SUNXI_TVE_AUTO_DETECT_STATUS_MASK(dac)	(3 << ((dac) * 8))
#define SUNXI_TVE_AUTO_DETECT_STATUS_NONE	0
#define SUNXI_TVE_AUTO_DETECT_STATUS_CONNECTED	1
#define SUNXI_TVE_AUTO_DETECT_STATUS_SHORT_GND	3
#define SUNXI_TVE_AUTO_DETECT_DEBOUNCE_SHIFT(d)	((d) * 8)
#define SUNXI_TVE_AUTO_DETECT_DEBOUNCE_MASK(d)	(0xf << ((d) * 8))
#define SUNXI_TVE_CSC_REG0_ENABLE		(1 << 31)
#define SUNXI_TVE_CSC_REG0			0x08440832
#define SUNXI_TVE_CSC_REG1			0x3b6dace1
#define SUNXI_TVE_CSC_REG2			0x0e1d13dc
#define SUNXI_TVE_CSC_REG3			0x00108080
#define SUNXI_TVE_COLOR_BURST_PAL_M		0x00000000
#define SUNXI_TVE_CBR_LEVEL_PAL			0x00002828
#define SUNXI_TVE_CBR_LEVEL_NTSC		0x0000004f
#define SUNXI_TVE_BURST_PHASE_NTSC		0x00000000
#define SUNXI_TVE_BURST_WIDTH_COMPOSITE		0x0016447e
#define SUNXI_TVE_UNKNOWN2_PAL			0x0000e0e0
#define SUNXI_TVE_UNKNOWN2_NTSC			0x0000a0a0
#define SUNXI_TVE_SYNC_VBI_LEVEL_NTSC		0x001000f0
#define SUNXI_TVE_ACTIVE_NUM_COMPOSITE		0x000005a0
#define SUNXI_TVE_CHROMA_BW_GAIN_COMP		0x00000002
#define SUNXI_TVE_NOTCH_WIDTH_COMPOSITE		0x00000101
#define SUNXI_TVE_RESYNC_NUM_PAL		0x800d000c
#define SUNXI_TVE_RESYNC_NUM_NTSC		0x000e000c
#define SUNXI_TVE_SLAVE_PARA_COMPOSITE		0x00000000

int sunxi_simplefb_setup(void *blob);

#endif /* _SUNXI_DISPLAY_H */
