/*
 * Sunxi platform display controller register and constant defines
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DISPLAY_H
#define _SUNXI_DISPLAY_H

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
};

struct sunxi_lcdc_reg {
	u32 ctrl;			/* 0x00 */
	u32 int0;			/* 0x04 */
	u32 int1;			/* 0x08 */
	u8 res0[0x04];			/* 0x0c */
	u32 tcon0_frm_ctrl;		/* 0x10 */
	u32 tcon0_frm_seed[6];		/* 0x14 */
	u32 tcon0_frm_table[4];		/* 0x2c */
	u8 res1[4];			/* 0x3c */
	u32 tcon0_ctrl;			/* 0x40 */
	u32 tcon0_dclk;			/* 0x44 */
	u32 tcon0_timing_active;	/* 0x48 */
	u32 tcon0_timing_h;		/* 0x4c */
	u32 tcon0_timing_v;		/* 0x50 */
	u32 tcon0_timing_sync;		/* 0x54 */
	u32 tcon0_hv_intf;		/* 0x58 */
	u8 res2[0x04];			/* 0x5c */
	u32 tcon0_cpu_intf;		/* 0x60 */
	u32 tcon0_cpu_wr_dat;		/* 0x64 */
	u32 tcon0_cpu_rd_dat0;		/* 0x68 */
	u32 tcon0_cpu_rd_dat1;		/* 0x6c */
	u32 tcon0_ttl_timing0;		/* 0x70 */
	u32 tcon0_ttl_timing1;		/* 0x74 */
	u32 tcon0_ttl_timing2;		/* 0x78 */
	u32 tcon0_ttl_timing3;		/* 0x7c */
	u32 tcon0_ttl_timing4;		/* 0x80 */
	u32 tcon0_lvds_intf;		/* 0x84 */
	u32 tcon0_io_polarity;		/* 0x88 */
	u32 tcon0_io_tristate;		/* 0x8c */
	u32 tcon1_ctrl;			/* 0x90 */
	u32 tcon1_timing_source;	/* 0x94 */
	u32 tcon1_timing_scale;		/* 0x98 */
	u32 tcon1_timing_out;		/* 0x9c */
	u32 tcon1_timing_h;		/* 0xa0 */
	u32 tcon1_timing_v;		/* 0xa4 */
	u32 tcon1_timing_sync;		/* 0xa8 */
	u8 res3[0x44];			/* 0xac */
	u32 tcon1_io_polarity;		/* 0xf0 */
	u32 tcon1_io_tristate;		/* 0xf4 */
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
 * DE-BE register constants.
 */
#define SUNXI_DE_BE_WIDTH(x)			(((x) - 1) << 0)
#define SUNXI_DE_BE_HEIGHT(y)			(((y) - 1) << 16)
#define SUNXI_DE_BE_MODE_ENABLE			(1 << 0)
#define SUNXI_DE_BE_MODE_START			(1 << 1)
#define SUNXI_DE_BE_MODE_LAYER0_ENABLE		(1 << 8)
#define SUNXI_DE_BE_LAYER_STRIDE(x)		((x) << 5)
#define SUNXI_DE_BE_REG_CTRL_LOAD_REGS		(1 << 0)
#define SUNXI_DE_BE_LAYER_ATTR1_FMT_XRGB8888	(0x09 << 8)

/*
 * LCDC register constants.
 */
#define SUNXI_LCDC_X(x)				(((x) - 1) << 16)
#define SUNXI_LCDC_Y(y)				(((y) - 1) << 0)
#define SUNXI_LCDC_TCON_VSYNC_MASK		(1 << 24)
#define SUNXI_LCDC_TCON_HSYNC_MASK		(1 << 25)
#define SUNXI_LCDC_CTRL_IO_MAP_MASK		(1 << 0)
#define SUNXI_LCDC_CTRL_IO_MAP_TCON0		(0 << 0)
#define SUNXI_LCDC_CTRL_IO_MAP_TCON1		(1 << 0)
#define SUNXI_LCDC_CTRL_TCON_ENABLE		(1 << 31)
#define SUNXI_LCDC_TCON0_FRM_CTRL_RGB666	((1 << 31) | (0 << 4))
#define SUNXI_LCDC_TCON0_FRM_CTRL_RGB565	((1 << 31) | (5 << 4))
#define SUNXI_LCDC_TCON0_FRM_SEED		0x11111111
#define SUNXI_LCDC_TCON0_FRM_TAB0		0x01010000
#define SUNXI_LCDC_TCON0_FRM_TAB1		0x15151111
#define SUNXI_LCDC_TCON0_FRM_TAB2		0x57575555
#define SUNXI_LCDC_TCON0_FRM_TAB3		0x7f7f7777
#define SUNXI_LCDC_TCON0_CTRL_CLK_DELAY(n)	(((n) & 0x1f) << 4)
#define SUNXI_LCDC_TCON0_CTRL_ENABLE		(1 << 31)
#define SUNXI_LCDC_TCON0_DCLK_DIV(n)		((n) << 0)
#define SUNXI_LCDC_TCON0_DCLK_ENABLE		(0xf << 28)
#define SUNXI_LCDC_TCON0_TIMING_H_BP(n)		(((n) - 1) << 0)
#define SUNXI_LCDC_TCON0_TIMING_H_TOTAL(n)	(((n) - 1) << 16)
#define SUNXI_LCDC_TCON0_TIMING_V_BP(n)		(((n) - 1) << 0)
#define SUNXI_LCDC_TCON0_TIMING_V_TOTAL(n)	(((n) * 2) << 16)
#define SUNXI_LCDC_TCON1_CTRL_CLK_DELAY(n)	(((n) & 0x1f) << 4)
#define SUNXI_LCDC_TCON1_CTRL_ENABLE		(1 << 31)
#define SUNXI_LCDC_TCON1_TIMING_H_BP(n)		(((n) - 1) << 0)
#define SUNXI_LCDC_TCON1_TIMING_H_TOTAL(n)	(((n) - 1) << 16)
#define SUNXI_LCDC_TCON1_TIMING_V_BP(n)		(((n) - 1) << 0)
#define SUNXI_LCDC_TCON1_TIMING_V_TOTAL(n)	(((n) * 2) << 16)

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

int sunxi_simplefb_setup(void *blob);

#endif /* _SUNXI_DISPLAY_H */
