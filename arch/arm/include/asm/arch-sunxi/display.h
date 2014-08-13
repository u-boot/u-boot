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
	u32 frame_ctrl;			/* 0x10 */
	u8 res1[0x2c];			/* 0x14 */
	u32 tcon0_ctrl;			/* 0x40 */
	u32 tcon0_dclk;			/* 0x44 */
	u32 tcon0_basic_timing0;	/* 0x48 */
	u32 tcon0_basic_timing1;	/* 0x4c */
	u32 tcon0_basic_timing2;	/* 0x50 */
	u32 tcon0_basic_timing3;	/* 0x54 */
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
	u8 res0[0x1d8];			/* 0x028 */
	u32 pad_ctrl0;			/* 0x200 */
	u32 pad_ctrl1;			/* 0x204 */
	u32 pll_ctrl;			/* 0x208 */
	u32 pll_dbg0;			/* 0x20c */
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
#define SUNXI_LCDC_CTRL_IO_MAP_MASK		(1 << 0)
#define SUNXI_LCDC_CTRL_IO_MAP_TCON0		(0 << 0)
#define SUNXI_LCDC_CTRL_IO_MAP_TCON1		(1 << 0)
#define SUNXI_LCDC_CTRL_TCON_ENABLE		(1 << 31)
#define SUNXI_LCDC_TCON0_DCLK_ENABLE		(0xf << 28)
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
#define SUNXI_HDMI_VIDEO_POL_HOR		(1 << 0)
#define SUNXI_HDMI_VIDEO_POL_VER		(1 << 1)
#define SUNXI_HDMI_VIDEO_POL_TX_CLK		(0x3e0 << 16)

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

int sunxi_simplefb_setup(void *blob);

#endif /* _SUNXI_DISPLAY_H */
