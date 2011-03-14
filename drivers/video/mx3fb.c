/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <lcd.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/errno.h>

DECLARE_GLOBAL_DATA_PTR;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

short console_col;
short console_row;

void lcd_initcolregs(void)
{
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
}

void lcd_disable(void)
{
}

void lcd_panel_disable(void)
{
}

#define msleep(a) udelay(a * 1000)

#if defined(CONFIG_DISPLAY_VBEST_VGG322403)
#define XRES		320
#define YRES		240
#define PANEL_TYPE	IPU_PANEL_TFT
#define PIXEL_CLK	156000
#define PIXEL_FMT	IPU_PIX_FMT_RGB666
#define H_START_WIDTH	20		/* left_margin */
#define H_SYNC_WIDTH	30		/* hsync_len */
#define H_END_WIDTH	(38 + 30)	/* right_margin + hsync_len */
#define V_START_WIDTH	7		/* upper_margin */
#define V_SYNC_WIDTH	3		/* vsync_len */
#define V_END_WIDTH	(26 + 3)	/* lower_margin + vsync_len */
#define SIG_POL		(DI_D3_DRDY_SHARP_POL | DI_D3_CLK_POL)
#define IF_CONF		0
#define IF_CLK_DIV	0x175
#elif defined(CONFIG_DISPLAY_COM57H5M10XRC)
#define XRES		640
#define YRES		480
#define PANEL_TYPE	IPU_PANEL_TFT
#define PIXEL_CLK	40000
#define PIXEL_FMT	IPU_PIX_FMT_RGB666
#define H_START_WIDTH	120		/* left_margin */
#define H_SYNC_WIDTH	30		/* hsync_len */
#define H_END_WIDTH	(10 + 30)	/* right_margin + hsync_len */
#define V_START_WIDTH	35		/* upper_margin */
#define V_SYNC_WIDTH	3		/* vsync_len */
#define V_END_WIDTH	(7 + 3)	/* lower_margin + vsync_len */
#define SIG_POL		(DI_D3_DRDY_SHARP_POL | DI_D3_CLK_POL)
#define IF_CONF		0
#define IF_CLK_DIV	0x55
#else
#define XRES		240
#define YRES		320
#define PANEL_TYPE	IPU_PANEL_TFT
#define PIXEL_CLK	185925
#define PIXEL_FMT	IPU_PIX_FMT_RGB666
#define H_START_WIDTH	9		/* left_margin */
#define H_SYNC_WIDTH	1		/* hsync_len */
#define H_END_WIDTH	(16 + 1)	/* right_margin + hsync_len */
#define V_START_WIDTH	7		/* upper_margin */
#define V_SYNC_WIDTH	1		/* vsync_len */
#define V_END_WIDTH	(9 + 1)		/* lower_margin + vsync_len */
#define SIG_POL		(DI_D3_DRDY_SHARP_POL | DI_D3_CLK_POL)
#define IF_CONF		0
#define IF_CLK_DIV	0x175
#endif

#define LCD_COLOR_IPU	LCD_COLOR16

static ushort colormap[256];

vidinfo_t panel_info = {
	.vl_col		= XRES,
	.vl_row		= YRES,
	.vl_bpix	= LCD_COLOR_IPU,
	.cmap		= colormap,
};

#define BIT_PER_PIXEL	NBITS(LCD_COLOR_IPU)

/* IPU DMA Controller channel definitions. */
enum ipu_channel {
	IDMAC_IC_0 = 0,		/* IC (encoding task) to memory */
	IDMAC_IC_1 = 1,		/* IC (viewfinder task) to memory */
	IDMAC_ADC_0 = 1,
	IDMAC_IC_2 = 2,
	IDMAC_ADC_1 = 2,
	IDMAC_IC_3 = 3,
	IDMAC_IC_4 = 4,
	IDMAC_IC_5 = 5,
	IDMAC_IC_6 = 6,
	IDMAC_IC_7 = 7,		/* IC (sensor data) to memory */
	IDMAC_IC_8 = 8,
	IDMAC_IC_9 = 9,
	IDMAC_IC_10 = 10,
	IDMAC_IC_11 = 11,
	IDMAC_IC_12 = 12,
	IDMAC_IC_13 = 13,
	IDMAC_SDC_0 = 14,	/* Background synchronous display data */
	IDMAC_SDC_1 = 15,	/* Foreground data (overlay) */
	IDMAC_SDC_2 = 16,
	IDMAC_SDC_3 = 17,
	IDMAC_ADC_2 = 18,
	IDMAC_ADC_3 = 19,
	IDMAC_ADC_4 = 20,
	IDMAC_ADC_5 = 21,
	IDMAC_ADC_6 = 22,
	IDMAC_ADC_7 = 23,
	IDMAC_PF_0 = 24,
	IDMAC_PF_1 = 25,
	IDMAC_PF_2 = 26,
	IDMAC_PF_3 = 27,
	IDMAC_PF_4 = 28,
	IDMAC_PF_5 = 29,
	IDMAC_PF_6 = 30,
	IDMAC_PF_7 = 31,
};

/* More formats can be copied from the Linux driver if needed */
enum pixel_fmt {
	/* 2 bytes */
	IPU_PIX_FMT_RGB565,
	IPU_PIX_FMT_RGB666,
	IPU_PIX_FMT_BGR666,
	/* 3 bytes */
	IPU_PIX_FMT_RGB24,
};

struct pixel_fmt_cfg {
	u32	b0;
	u32	b1;
	u32	b2;
	u32	acc;
};

static struct pixel_fmt_cfg fmt_cfg[] = {
	[IPU_PIX_FMT_RGB24] = {
		0x1600AAAA, 0x00E05555, 0x00070000, 3,
	},
	[IPU_PIX_FMT_RGB666] = {
		0x0005000F, 0x000B000F, 0x0011000F, 1,
	},
	[IPU_PIX_FMT_BGR666] = {
		0x0011000F, 0x000B000F, 0x0005000F, 1,
	},
	[IPU_PIX_FMT_RGB565] = {
		0x0004003F, 0x000A000F, 0x000F003F, 1,
	}
};

enum ipu_panel {
	IPU_PANEL_SHARP_TFT,
	IPU_PANEL_TFT,
};

/* IPU Common registers */
/* IPU_CONF and its bits already defined in imx-regs.h */
#define IPU_CHA_BUF0_RDY	(0x04 + IPU_BASE)
#define IPU_CHA_BUF1_RDY	(0x08 + IPU_BASE)
#define IPU_CHA_DB_MODE_SEL	(0x0C + IPU_BASE)
#define IPU_CHA_CUR_BUF		(0x10 + IPU_BASE)
#define IPU_FS_PROC_FLOW	(0x14 + IPU_BASE)
#define IPU_FS_DISP_FLOW	(0x18 + IPU_BASE)
#define IPU_TASKS_STAT		(0x1C + IPU_BASE)
#define IPU_IMA_ADDR		(0x20 + IPU_BASE)
#define IPU_IMA_DATA		(0x24 + IPU_BASE)
#define IPU_INT_CTRL_1		(0x28 + IPU_BASE)
#define IPU_INT_CTRL_2		(0x2C + IPU_BASE)
#define IPU_INT_CTRL_3		(0x30 + IPU_BASE)
#define IPU_INT_CTRL_4		(0x34 + IPU_BASE)
#define IPU_INT_CTRL_5		(0x38 + IPU_BASE)
#define IPU_INT_STAT_1		(0x3C + IPU_BASE)
#define IPU_INT_STAT_2		(0x40 + IPU_BASE)
#define IPU_INT_STAT_3		(0x44 + IPU_BASE)
#define IPU_INT_STAT_4		(0x48 + IPU_BASE)
#define IPU_INT_STAT_5		(0x4C + IPU_BASE)
#define IPU_BRK_CTRL_1		(0x50 + IPU_BASE)
#define IPU_BRK_CTRL_2		(0x54 + IPU_BASE)
#define IPU_BRK_STAT		(0x58 + IPU_BASE)
#define IPU_DIAGB_CTRL		(0x5C + IPU_BASE)

/* Image Converter Registers */
#define IC_CONF			(0x88 + IPU_BASE)
#define IC_PRP_ENC_RSC		(0x8C + IPU_BASE)
#define IC_PRP_VF_RSC		(0x90 + IPU_BASE)
#define IC_PP_RSC		(0x94 + IPU_BASE)
#define IC_CMBP_1		(0x98 + IPU_BASE)
#define IC_CMBP_2		(0x9C + IPU_BASE)
#define PF_CONF			(0xA0 + IPU_BASE)
#define IDMAC_CONF		(0xA4 + IPU_BASE)
#define IDMAC_CHA_EN		(0xA8 + IPU_BASE)
#define IDMAC_CHA_PRI		(0xAC + IPU_BASE)
#define IDMAC_CHA_BUSY		(0xB0 + IPU_BASE)

/* Image Converter Register bits */
#define IC_CONF_PRPENC_EN	0x00000001
#define IC_CONF_PRPENC_CSC1	0x00000002
#define IC_CONF_PRPENC_ROT_EN	0x00000004
#define IC_CONF_PRPVF_EN	0x00000100
#define IC_CONF_PRPVF_CSC1	0x00000200
#define IC_CONF_PRPVF_CSC2	0x00000400
#define IC_CONF_PRPVF_CMB	0x00000800
#define IC_CONF_PRPVF_ROT_EN	0x00001000
#define IC_CONF_PP_EN		0x00010000
#define IC_CONF_PP_CSC1		0x00020000
#define IC_CONF_PP_CSC2		0x00040000
#define IC_CONF_PP_CMB		0x00080000
#define IC_CONF_PP_ROT_EN	0x00100000
#define IC_CONF_IC_GLB_LOC_A	0x10000000
#define IC_CONF_KEY_COLOR_EN	0x20000000
#define IC_CONF_RWS_EN		0x40000000
#define IC_CONF_CSI_MEM_WR_EN	0x80000000

/* SDC Registers */
#define SDC_COM_CONF		(0xB4 + IPU_BASE)
#define SDC_GW_CTRL		(0xB8 + IPU_BASE)
#define SDC_FG_POS		(0xBC + IPU_BASE)
#define SDC_BG_POS		(0xC0 + IPU_BASE)
#define SDC_CUR_POS		(0xC4 + IPU_BASE)
#define SDC_PWM_CTRL		(0xC8 + IPU_BASE)
#define SDC_CUR_MAP		(0xCC + IPU_BASE)
#define SDC_HOR_CONF		(0xD0 + IPU_BASE)
#define SDC_VER_CONF		(0xD4 + IPU_BASE)
#define SDC_SHARP_CONF_1	(0xD8 + IPU_BASE)
#define SDC_SHARP_CONF_2	(0xDC + IPU_BASE)

/* Register bits */
#define SDC_COM_TFT_COLOR	0x00000001UL
#define SDC_COM_FG_EN		0x00000010UL
#define SDC_COM_GWSEL		0x00000020UL
#define SDC_COM_GLB_A		0x00000040UL
#define SDC_COM_KEY_COLOR_G	0x00000080UL
#define SDC_COM_BG_EN		0x00000200UL
#define SDC_COM_SHARP		0x00001000UL

#define SDC_V_SYNC_WIDTH_L	0x00000001UL

/* Display Interface registers */
#define DI_DISP_IF_CONF		(0x0124 + IPU_BASE)
#define DI_DISP_SIG_POL		(0x0128 + IPU_BASE)
#define DI_SER_DISP1_CONF	(0x012C + IPU_BASE)
#define DI_SER_DISP2_CONF	(0x0130 + IPU_BASE)
#define DI_HSP_CLK_PER		(0x0134 + IPU_BASE)
#define DI_DISP0_TIME_CONF_1	(0x0138 + IPU_BASE)
#define DI_DISP0_TIME_CONF_2	(0x013C + IPU_BASE)
#define DI_DISP0_TIME_CONF_3	(0x0140 + IPU_BASE)
#define DI_DISP1_TIME_CONF_1	(0x0144 + IPU_BASE)
#define DI_DISP1_TIME_CONF_2	(0x0148 + IPU_BASE)
#define DI_DISP1_TIME_CONF_3	(0x014C + IPU_BASE)
#define DI_DISP2_TIME_CONF_1	(0x0150 + IPU_BASE)
#define DI_DISP2_TIME_CONF_2	(0x0154 + IPU_BASE)
#define DI_DISP2_TIME_CONF_3	(0x0158 + IPU_BASE)
#define DI_DISP3_TIME_CONF	(0x015C + IPU_BASE)
#define DI_DISP0_DB0_MAP	(0x0160 + IPU_BASE)
#define DI_DISP0_DB1_MAP	(0x0164 + IPU_BASE)
#define DI_DISP0_DB2_MAP	(0x0168 + IPU_BASE)
#define DI_DISP0_CB0_MAP	(0x016C + IPU_BASE)
#define DI_DISP0_CB1_MAP	(0x0170 + IPU_BASE)
#define DI_DISP0_CB2_MAP	(0x0174 + IPU_BASE)
#define DI_DISP1_DB0_MAP	(0x0178 + IPU_BASE)
#define DI_DISP1_DB1_MAP	(0x017C + IPU_BASE)
#define DI_DISP1_DB2_MAP	(0x0180 + IPU_BASE)
#define DI_DISP1_CB0_MAP	(0x0184 + IPU_BASE)
#define DI_DISP1_CB1_MAP	(0x0188 + IPU_BASE)
#define DI_DISP1_CB2_MAP	(0x018C + IPU_BASE)
#define DI_DISP2_DB0_MAP	(0x0190 + IPU_BASE)
#define DI_DISP2_DB1_MAP	(0x0194 + IPU_BASE)
#define DI_DISP2_DB2_MAP	(0x0198 + IPU_BASE)
#define DI_DISP2_CB0_MAP	(0x019C + IPU_BASE)
#define DI_DISP2_CB1_MAP	(0x01A0 + IPU_BASE)
#define DI_DISP2_CB2_MAP	(0x01A4 + IPU_BASE)
#define DI_DISP3_B0_MAP		(0x01A8 + IPU_BASE)
#define DI_DISP3_B1_MAP		(0x01AC + IPU_BASE)
#define DI_DISP3_B2_MAP		(0x01B0 + IPU_BASE)
#define DI_DISP_ACC_CC		(0x01B4 + IPU_BASE)
#define DI_DISP_LLA_CONF	(0x01B8 + IPU_BASE)
#define DI_DISP_LLA_DATA	(0x01BC + IPU_BASE)

/* DI_DISP_SIG_POL bits */
#define DI_D3_VSYNC_POL		(1 << 28)
#define DI_D3_HSYNC_POL		(1 << 27)
#define DI_D3_DRDY_SHARP_POL	(1 << 26)
#define DI_D3_CLK_POL		(1 << 25)
#define DI_D3_DATA_POL		(1 << 24)

/* DI_DISP_IF_CONF bits */
#define DI_D3_CLK_IDLE		(1 << 26)
#define DI_D3_CLK_SEL		(1 << 25)
#define DI_D3_DATAMSK		(1 << 24)

#define IOMUX_PADNUM_MASK	0x1ff
#define IOMUX_GPIONUM_SHIFT	9
#define IOMUX_GPIONUM_MASK	(0xff << IOMUX_GPIONUM_SHIFT)

#define IOMUX_PIN(gpionum, padnum) ((padnum) & IOMUX_PADNUM_MASK)

#define IOMUX_MODE_L(pin, mode) IOMUX_MODE(((pin) + 0xc) ^ 3, mode)

struct chan_param_mem_planar {
	/* Word 0 */
	u32	xv:10;
	u32	yv:10;
	u32	xb:12;

	u32	yb:12;
	u32	res1:2;
	u32	nsb:1;
	u32	lnpb:6;
	u32	ubo_l:11;

	u32	ubo_h:15;
	u32	vbo_l:17;

	u32	vbo_h:9;
	u32	res2:3;
	u32	fw:12;
	u32	fh_l:8;

	u32	fh_h:4;
	u32	res3:28;

	/* Word 1 */
	u32	eba0;

	u32	eba1;

	u32	bpp:3;
	u32	sl:14;
	u32	pfs:3;
	u32	bam:3;
	u32	res4:2;
	u32	npb:6;
	u32	res5:1;

	u32	sat:2;
	u32	res6:30;
} __attribute__ ((packed));

struct chan_param_mem_interleaved {
	/* Word 0 */
	u32	xv:10;
	u32	yv:10;
	u32	xb:12;

	u32	yb:12;
	u32	sce:1;
	u32	res1:1;
	u32	nsb:1;
	u32	lnpb:6;
	u32	sx:10;
	u32	sy_l:1;

	u32	sy_h:9;
	u32	ns:10;
	u32	sm:10;
	u32	sdx_l:3;

	u32	sdx_h:2;
	u32	sdy:5;
	u32	sdrx:1;
	u32	sdry:1;
	u32	sdr1:1;
	u32	res2:2;
	u32	fw:12;
	u32	fh_l:8;

	u32	fh_h:4;
	u32	res3:28;

	/* Word 1 */
	u32	eba0;

	u32	eba1;

	u32	bpp:3;
	u32	sl:14;
	u32	pfs:3;
	u32	bam:3;
	u32	res4:2;
	u32	npb:6;
	u32	res5:1;

	u32	sat:2;
	u32	scc:1;
	u32	ofs0:5;
	u32	ofs1:5;
	u32	ofs2:5;
	u32	ofs3:5;
	u32	wid0:3;
	u32	wid1:3;
	u32	wid2:3;

	u32	wid3:3;
	u32	dec_sel:1;
	u32	res6:28;
} __attribute__ ((packed));

union chan_param_mem {
	struct chan_param_mem_planar		pp;
	struct chan_param_mem_interleaved	ip;
};

static inline u32 reg_read(unsigned long reg)
{
	return __REG(reg);
}

static inline void reg_write(u32 value, unsigned long reg)
{
	__REG(reg) = value;
}

/*
 * sdc_init_panel() - initialize a synchronous LCD panel.
 * @width:		width of panel in pixels.
 * @height:		height of panel in pixels.
 * @pixel_fmt:		pixel format of buffer as FOURCC ASCII code.
 * @return:		0 on success or negative error code on failure.
 */
static int sdc_init_panel(u16 width, u16 height, enum pixel_fmt pixel_fmt)
{
	u32 reg;
	uint32_t old_conf;

	/* Init panel size and blanking periods */
	reg = ((H_SYNC_WIDTH - 1) << 26) |
		((u32)(width + H_START_WIDTH + H_END_WIDTH - 1) << 16);
	reg_write(reg, SDC_HOR_CONF);

	reg = ((V_SYNC_WIDTH - 1) << 26) | SDC_V_SYNC_WIDTH_L |
		((u32)(height + V_START_WIDTH + V_END_WIDTH - 1) << 16);
	reg_write(reg, SDC_VER_CONF);

	switch (PANEL_TYPE) {
	case IPU_PANEL_SHARP_TFT:
		reg_write(0x00FD0102L, SDC_SHARP_CONF_1);
		reg_write(0x00F500F4L, SDC_SHARP_CONF_2);
		reg_write(SDC_COM_SHARP | SDC_COM_TFT_COLOR, SDC_COM_CONF);
		break;
	case IPU_PANEL_TFT:
		reg_write(SDC_COM_TFT_COLOR, SDC_COM_CONF);
		break;
	default:
		return -EINVAL;
	}

	/* Init clocking */

	/*
	 * Calculate divider: fractional part is 4 bits so simply multiple by
	 * 2^4 to get fractional part, as long as we stay under ~250MHz and on
	 * i.MX31 it (HSP_CLK) is <= 178MHz. Currently 128.267MHz
	 */

	reg_write((((IF_CLK_DIV / 8) - 1) << 22) |
			IF_CLK_DIV, DI_DISP3_TIME_CONF);

	/* DI settings */
	old_conf = reg_read(DI_DISP_IF_CONF) & 0x78FFFFFF;
	reg_write(old_conf | IF_CONF, DI_DISP_IF_CONF);

	old_conf = reg_read(DI_DISP_SIG_POL) & 0xE0FFFFFF;
	reg_write(old_conf | SIG_POL, DI_DISP_SIG_POL);

	reg_write(fmt_cfg[pixel_fmt].b0, DI_DISP3_B0_MAP);
	reg_write(fmt_cfg[pixel_fmt].b1, DI_DISP3_B1_MAP);
	reg_write(fmt_cfg[pixel_fmt].b2, DI_DISP3_B2_MAP);
	reg_write(reg_read(DI_DISP_ACC_CC) |
		  ((fmt_cfg[pixel_fmt].acc - 1) << 12), DI_DISP_ACC_CC);

	return 0;
}

static void ipu_ch_param_set_size(union chan_param_mem *params,
				  uint32_t pixel_fmt, uint16_t width,
				  uint16_t height, uint16_t stride)
{
	params->pp.fw		= width - 1;
	params->pp.fh_l		= height - 1;
	params->pp.fh_h		= (height - 1) >> 8;
	params->pp.sl		= stride - 1;

	/* See above, for further formats see the Linux driver */
	switch (pixel_fmt) {
	case IPU_PIX_FMT_RGB565:
		params->ip.bpp	= 2;
		params->ip.pfs	= 4;
		params->ip.npb	= 7;
		params->ip.sat	= 2;		/* SAT = 32-bit access */
		params->ip.ofs0	= 0;		/* Red bit offset */
		params->ip.ofs1	= 5;		/* Green bit offset */
		params->ip.ofs2	= 11;		/* Blue bit offset */
		params->ip.ofs3	= 16;		/* Alpha bit offset */
		params->ip.wid0	= 4;		/* Red bit width - 1 */
		params->ip.wid1	= 5;		/* Green bit width - 1 */
		params->ip.wid2	= 4;		/* Blue bit width - 1 */
		break;
	case IPU_PIX_FMT_RGB24:
		params->ip.bpp	= 1;		/* 24 BPP & RGB PFS */
		params->ip.pfs	= 4;
		params->ip.npb	= 7;
		params->ip.sat	= 2;		/* SAT = 32-bit access */
		params->ip.ofs0	= 16;		/* Red bit offset */
		params->ip.ofs1	= 8;		/* Green bit offset */
		params->ip.ofs2	= 0;		/* Blue bit offset */
		params->ip.ofs3	= 24;		/* Alpha bit offset */
		params->ip.wid0	= 7;		/* Red bit width - 1 */
		params->ip.wid1	= 7;		/* Green bit width - 1 */
		params->ip.wid2	= 7;		/* Blue bit width - 1 */
		break;
	default:
		break;
	}

	params->pp.nsb = 1;
}

static void ipu_ch_param_set_buffer(union chan_param_mem *params,
				    void *buf0, void *buf1)
{
	params->pp.eba0 = (u32)buf0;
	params->pp.eba1 = (u32)buf1;
}

static void ipu_write_param_mem(uint32_t addr, uint32_t *data,
				uint32_t num_words)
{
	for (; num_words > 0; num_words--) {
		reg_write(addr, IPU_IMA_ADDR);
		reg_write(*data++, IPU_IMA_DATA);
		addr++;
		if ((addr & 0x7) == 5) {
			addr &= ~0x7;	/* set to word 0 */
			addr += 8;	/* increment to next row */
		}
	}
}

static uint32_t bpp_to_pixfmt(int bpp)
{
	switch (bpp) {
	case 16:
		return IPU_PIX_FMT_RGB565;
	default:
		return 0;
	}
}

static uint32_t dma_param_addr(enum ipu_channel channel)
{
	/* Channel Parameter Memory */
	return 0x10000 | (channel << 4);
}

static void ipu_init_channel_buffer(enum ipu_channel channel, void *fbmem)
{
	union chan_param_mem params = {};
	uint32_t reg;
	uint32_t stride_bytes;

	stride_bytes = (XRES * ((BIT_PER_PIXEL + 7) / 8) + 3) & ~3;

	/* Build parameter memory data for DMA channel */
	ipu_ch_param_set_size(&params, bpp_to_pixfmt(BIT_PER_PIXEL),
			      XRES, YRES, stride_bytes);
	ipu_ch_param_set_buffer(&params, fbmem, NULL);
	params.pp.bam = 0;
	/* Some channels (rotation) have restriction on burst length */

	switch (channel) {
	case IDMAC_SDC_0:
		/* In original code only IPU_PIX_FMT_RGB565 was setting burst */
		params.pp.npb = 16 - 1;
		break;
	default:
		break;
	}

	ipu_write_param_mem(dma_param_addr(channel), (uint32_t *)&params, 10);

	/* Disable double-buffering */
	reg = reg_read(IPU_CHA_DB_MODE_SEL);
	reg &= ~(1UL << channel);
	reg_write(reg, IPU_CHA_DB_MODE_SEL);
}

static void ipu_channel_set_priority(enum ipu_channel channel,
				     int prio)
{
	u32 reg = reg_read(IDMAC_CHA_PRI);

	if (prio)
		reg |= 1UL << channel;
	else
		reg &= ~(1UL << channel);

	reg_write(reg, IDMAC_CHA_PRI);
}

/*
 * ipu_enable_channel() - enable an IPU channel.
 * @channel:	channel ID.
 * @return:	0 on success or negative error code on failure.
 */
static int ipu_enable_channel(enum ipu_channel channel)
{
	uint32_t reg;

	/* Reset to buffer 0 */
	reg_write(1UL << channel, IPU_CHA_CUR_BUF);

	switch (channel) {
	case IDMAC_SDC_0:
		ipu_channel_set_priority(channel, 1);
		break;
	default:
		break;
	}

	reg = reg_read(IDMAC_CHA_EN);
	reg_write(reg | (1UL << channel), IDMAC_CHA_EN);

	return 0;
}

static int ipu_update_channel_buffer(enum ipu_channel channel, void *buf)
{
	uint32_t reg;

	reg = reg_read(IPU_CHA_BUF0_RDY);
	if (reg & (1UL << channel))
		return -EACCES;

	/* 44.3.3.1.9 - Row Number 1 (WORD1, offset 0) */
	reg_write(dma_param_addr(channel) + 0x0008UL, IPU_IMA_ADDR);
	reg_write((u32)buf, IPU_IMA_DATA);

	return 0;
}

static int idmac_tx_submit(enum ipu_channel channel, void *buf)
{
	int ret;

	ipu_init_channel_buffer(channel, buf);


	/* ipu_idmac.c::ipu_submit_channel_buffers() */
	ret = ipu_update_channel_buffer(channel, buf);
	if (ret < 0)
		return ret;

	/* ipu_idmac.c::ipu_select_buffer() */
	/* Mark buffer 0 as ready. */
	reg_write(1UL << channel, IPU_CHA_BUF0_RDY);


	ret = ipu_enable_channel(channel);
	return ret;
}

static void sdc_enable_channel(void *fbmem)
{
	int ret;
	u32 reg;

	ret = idmac_tx_submit(IDMAC_SDC_0, fbmem);

	/* mx3fb.c::sdc_fb_init() */
	if (ret >= 0) {
		reg = reg_read(SDC_COM_CONF);
		reg_write(reg | SDC_COM_BG_EN, SDC_COM_CONF);
	}

	/*
	 * Attention! Without this msleep the channel keeps generating
	 * interrupts. Next sdc_set_brightness() is going to be called
	 * from mx3fb_blank().
	 */
	msleep(2);
}

/*
 * mx3fb_set_par() - set framebuffer parameters and change the operating mode.
 * @return:	0 on success or negative error code on failure.
 */
static int mx3fb_set_par(void)
{
	int ret;

	ret = sdc_init_panel(XRES, YRES, PIXEL_FMT);
	if (ret < 0)
		return ret;

	reg_write((H_START_WIDTH << 16) | V_START_WIDTH, SDC_BG_POS);

	return 0;
}

/* References in this function refer to respective Linux kernel sources */
void lcd_enable(void)
{
	u32 reg;

	/* pcm037.c::mxc_board_init() */

	/* Display Interface #3 */
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD0, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD1, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD2, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD3, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD4, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD5, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD6, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD7, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD8, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD9, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD10, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD11, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD12, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD13, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD14, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD15, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD16, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_LD17, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_VSYNC3, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_HSYNC, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_FPSHIFT, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_DRDY0, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_D3_REV, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_CONTRAST, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_D3_SPL, MUX_CTL_FUNC));
	mx31_gpio_mux(IOMUX_MODE_L(MX31_PIN_D3_CLS, MUX_CTL_FUNC));


	/* ipu_idmac.c::ipu_probe() */

	/* Start the clock */
	__REG(CCM_CGR1) = __REG(CCM_CGR1) | (3 << 22);


	/* ipu_idmac.c::ipu_idmac_init() */

	/* Service request counter to maximum - shouldn't be needed */
	reg_write(0x00000070, IDMAC_CONF);


	/* ipu_idmac.c::ipu_init_channel() */

	/* Enable IPU sub modules */
	reg = reg_read(IPU_CONF) | IPU_CONF_SDC_EN | IPU_CONF_DI_EN;
	reg_write(reg, IPU_CONF);


	/* mx3fb.c::init_fb_chan() */

	/* set Display Interface clock period */
	reg_write(0x00100010L, DI_HSP_CLK_PER);
	/* Might need to trigger HSP clock change - see 44.3.3.8.5 */


	/* mx3fb.c::sdc_set_brightness() */

	/* This might be board-specific */
	reg_write(0x03000000UL | 255 << 16, SDC_PWM_CTRL);


	/* mx3fb.c::sdc_set_global_alpha() */

	/* Use global - not per-pixel - Alpha-blending */
	reg = reg_read(SDC_GW_CTRL) & 0x00FFFFFFL;
	reg_write(reg | ((uint32_t) 0xff << 24), SDC_GW_CTRL);

	reg = reg_read(SDC_COM_CONF);
	reg_write(reg | SDC_COM_GLB_A, SDC_COM_CONF);


	/* mx3fb.c::sdc_set_color_key() */

	/* Disable colour-keying for background */
	reg = reg_read(SDC_COM_CONF) &
		~(SDC_COM_GWSEL | SDC_COM_KEY_COLOR_G);
	reg_write(reg, SDC_COM_CONF);


	mx3fb_set_par();

	sdc_enable_channel(lcd_base);

	/*
	 * Linux driver calls sdc_set_brightness() here again,
	 * once is enough for us
	 */
}

void lcd_ctrl_init(void *lcdbase)
{
	u32 mem_len = XRES * YRES * BIT_PER_PIXEL / 8;
	/*
	 * We rely on lcdbase being a physical address, i.e., either MMU off,
	 * or 1-to-1 mapping. Might want to add some virt2phys here.
	 */
	if (!lcdbase)
		return;

	memset(lcdbase, 0, mem_len);
}

ulong calc_fbsize(void)
{
	return ((panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8) + PAGE_SIZE;
}

int overwrite_console(void)
{
	/* Keep stdout / stderr on serial, our LCD is for splashscreen only */
	return 1;
}
