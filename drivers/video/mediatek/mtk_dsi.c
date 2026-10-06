// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video DSI support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <backlight.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <phy-mipi-dphy.h>
#include <regmap.h>
#include <syscon.h>
#include <video.h>

#include "mtk_ddp.h"
#include "mtk_disp_aal.h"
#include "mtk_disp_ccorr.h"
#include "mtk_disp_color.h"
#include "mtk_disp_comp.h"
#include "mtk_disp_dither.h"
#include "mtk_disp_gamma.h"
#include "mtk_disp_mutex.h"
#include "mtk_disp_ovl.h"
#include "mtk_disp_rdma.h"

DECLARE_GLOBAL_DATA_PTR;

#define MTK_DSI_MAX_WIDTH  3840
#define MTK_DSI_MAX_HEIGHT 2160

/* the longest of the DSI paths below */
#define MTK_DSI_MAX_COMPS  7

/* one microsecond apart, as many as the engine may need to take a RACK */
#define MTK_DSI_RACK_RETRIES 1000

#define DSI_START		0x00

#define DSI_INTEN		0x08
#define LPRX_RD_RDY_INT_FLAG	      BIT(0)
#define CMD_DONE_INT_FLAG	      BIT(1)
#define VM_DONE_INT_FLAG	      BIT(3)

/* Same flag layout as DSI_INTEN, plus the engine busy bit */
#define DSI_INTSTA		0x0c
#define DSI_BUSY		      BIT(31)

#define DSI_CON_CTRL		0x10
#define DSI_RESET		      BIT(0)
#define DSI_EN			      BIT(1)
#define DPHY_RESET		      BIT(2)

#define DSI_MODE_CTRL		0x14
#define MODE_CON		      GENMASK(1, 0)
#define CMD_MODE		      0
#define SYNC_PULSE_MODE		      1
#define SYNC_EVENT_MODE		      2
#define BURST_MODE		      3

#define DSI_TXRX_CTRL		0x18
#define LANE_NUM		      GENMASK(5, 2)
#define DIS_EOT			      BIT(6)
#define HSTX_CKLP_EN		      BIT(16)

#define DSI_PSCTRL		0x1c
#define DSI_PS_WC		      GENMASK(13, 0)
#define DSI_PS_SEL		      GENMASK(17, 16)
#define PACKED_PS_16BIT_RGB565	      0
#define PACKED_PS_18BIT_RGB666	      1
#define LOOSELY_PS_24BIT_RGB666	      2
#define PACKED_PS_24BIT_RGB888	      3

#define DSI_VSA_NL		0x20
#define DSI_VBP_NL		0x24
#define DSI_VFP_NL		0x28
#define DSI_VACT_NL		0x2C
#define DSI_SIZE_CON		0x38
#define DSI_SIZE_CON_WIDTH	      GENMASK(15, 0)
#define DSI_SIZE_CON_HEIGHT	      GENMASK(31, 16)
#define DSI_HSA_WC		0x50
#define DSI_HBP_WC		0x54
#define DSI_HFP_WC		0x58

#define DSI_CMDQ_SIZE		0x60
#define CMDQ_SIZE		      GENMASK(5, 0)
#define CMDQ_SIZE_SEL		      BIT(15)

#define DSI_HSTX_CKL_WC		0x64

#define DSI_RX_DATA0		0x74

#define DSI_RACK		0x84
#define RACK			      BIT(0)

#define VM_CMD_EN		       BIT(0)
#define TS_VFP_EN		       BIT(5)

#define DSI_PHY_LCCON		0x104
#define LC_HS_TX_EN		       BIT(0)
#define LC_ULPM_EN		       BIT(1)
#define LC_WAKEUP_EN		       BIT(2)

#define DSI_PHY_LD0CON		0x108
#define LD0_HS_TX_EN		       BIT(0)
#define LD0_ULPM_EN		       BIT(1)
#define LD0_WAKEUP_EN		       BIT(2)

#define DSI_PHY_TIMECON0	0x110
#define LPX			       GENMASK(7, 0)
#define HS_PREP			       GENMASK(15, 8)
#define HS_ZERO			       GENMASK(23, 16)
#define HS_TRAIL		       GENMASK(31, 24)

#define DSI_PHY_TIMECON1	0x114
#define TA_GO			       GENMASK(7, 0)
#define TA_SURE			       GENMASK(15, 8)
#define TA_GET			       GENMASK(23, 16)
#define DA_HS_EXIT		       GENMASK(31, 24)

#define DSI_PHY_TIMECON2	0x118
#define DA_HS_SYNC		       GENMASK(15, 8)
#define CLK_ZERO		       GENMASK(23, 16)
#define CLK_TRAIL		       GENMASK(31, 24)

#define DSI_PHY_TIMECON3	0x11c
#define CLK_HS_PREP		       GENMASK(7, 0)
#define CLK_HS_POST		       GENMASK(15, 8)
#define CLK_HS_EXIT		       GENMASK(23, 16)

/*
 * Shadow control, at 0x190 on the mt8183 and at 0xc00 on the mt8188 and
 * mt8189, as in Linux. It is not used on the MT8366.
 */
#define DSI_SHADOW_DEBUG	0x190
#define DSI_SHADOW_DEBUG_MT8188	0xc00
#define FORCE_COMMIT		       BIT(0)
#define BYPASS_SHADOW		       BIT(1)

/* Layout of the first word of the command queue (at data->host_cmdq_off) */
#define CMDQ_CONFIG		       GENMASK(7, 0)
#define SHORT_PACKET			     0
#define LONG_PACKET			     2
#define BTA				     BIT(2)
#define HSTX				     BIT(3)
#define CMDQ_DATA_ID		       GENMASK(15, 8)
#define CMDQ_DATA_0		       GENMASK(23, 16)
#define CMDQ_DATA_1		       GENMASK(31, 24)
/* For a long packet DATA_0/DATA_1 together hold the 16-bit word count */
#define CMDQ_WC			       GENMASK(31, 16)

#define MTK_DSI_HOST_IS_READ(type)				\
	((type == MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM) ||	\
	 (type == MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM) ||	\
	 (type == MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM) ||	\
	 (type == MIPI_DSI_DCS_READ))

/* D-PHY timings, in HS byte-clock cycles, as computed by the Linux driver */
struct mtk_dsi_phy_timing {
	u32 lpx;
	u32 da_hs_prepare;
	u32 da_hs_zero;
	u32 da_hs_trail;
	u32 ta_go;
	u32 ta_sure;
	u32 ta_get;
	u32 da_hs_exit;
	u32 clk_hs_prepare;
	u32 clk_hs_post;
	u32 clk_hs_trail;
	u32 clk_hs_zero;
	u32 clk_hs_exit;
};

struct mtk_dsi_priv {
	void __iomem *base;
	struct udevice *dev;
	const struct mtk_dsi_data *data;
	struct udevice *comps[MTK_DSI_MAX_COMPS];
	struct udevice *mutex;
	void __iomem *mmsys_base;
	struct phy phy;
	struct udevice *panel;
	struct mipi_dsi_host dsi_host;
	struct mipi_dsi_device mipi;
	struct clk *engine_clk;
	struct clk *digital_clk;
	struct mtk_dsi_phy_timing phy_timing;
	u64 data_rate;
};

/**
 * struct mtk_dsi_comp - a component of the path feeding the DSI
 * @alias: device tree alias of the component
 * @config: set the frame size of the component
 * @start: start the component
 */
struct mtk_dsi_comp {
	const char *alias;
	void (*config)(struct udevice *dev, const struct display_timing *timing);
	void (*start)(struct udevice *dev);
};

/**
 * struct mtk_dsi_data - per-SoC DSI data
 * @path: components feeding the DSI, from the overlay, which must come first
 * @path_len: number of components in @path, MTK_DSI_MAX_COMPS at most
 * @ddp_connect: route the mmsys muxes along @path
 * @cmdq_long_packet_ctl: the command queue size is not set automatically for
 *			  long packets, CMDQ_SIZE_SEL must be set
 * @shadow_dbg_off: offset of the shadow control, zero on the SoCs without it
 * @vm_cmdq_off: offset of the video mode command register
 * @host_cmdq_off: offset of the host command queue
 */
struct mtk_dsi_data {
	const struct mtk_dsi_comp *path;
	unsigned int path_len;
	void (*ddp_connect)(void __iomem *mmsys_base);
	bool cmdq_long_packet_ctl;
	u32 shadow_dbg_off;
	u32 vm_cmdq_off;
	u32 host_cmdq_off;
};

/* the paths are the main ones of the Linux mtk_drm_drv driver data */
static const struct mtk_dsi_comp mt8365_dsi_path[] = {
	{ "ovl0",	mtk_ovl_config,		mtk_ovl_start },
	{ "rdma0",	mtk_rdma_config,	mtk_rdma_start },
	{ "color0",	mtk_color_config,	mtk_color_start },
	{ "ccorr0",	mtk_ccorr_config,	mtk_ccorr_start },
	{ "aal0",	mtk_aal_config,		mtk_aal_start },
	{ "gamma0",	mtk_gamma_config,	mtk_gamma_start },
	{ "dither0",	mtk_dither_config,	mtk_dither_start },
};

static const struct mtk_dsi_comp mt8366_dsi_path[] = {
	{ "ovl0",	mtk_ovl_config,		mtk_ovl_start },
	{ "rdma0",	mtk_rdma_config,	mtk_rdma_start },
};

static const struct mtk_dsi_data mt8365_dsi_data = {
	.path = mt8365_dsi_path,
	.path_len = ARRAY_SIZE(mt8365_dsi_path),
	.ddp_connect = mt8365_ddp_ovl_to_dsi,
	.shadow_dbg_off = DSI_SHADOW_DEBUG,
	.vm_cmdq_off = 0x130,
	.host_cmdq_off = 0x200,
};

static const struct mtk_dsi_data mt8366_dsi_data = {
	.path = mt8366_dsi_path,
	.path_len = ARRAY_SIZE(mt8366_dsi_path),
	.ddp_connect = mt8366_ddp_ovl_to_dsi,
	.cmdq_long_packet_ctl = true,
	.vm_cmdq_off = 0x200,
	.host_cmdq_off = 0xd00,
};

/* the MT8189 path and mmsys muxes are the MT8366 ones */
static const struct mtk_dsi_data mt8189_dsi_data = {
	.path = mt8366_dsi_path,
	.path_len = ARRAY_SIZE(mt8366_dsi_path),
	.ddp_connect = mt8366_ddp_ovl_to_dsi,
	.cmdq_long_packet_ctl = true,
	.shadow_dbg_off = DSI_SHADOW_DEBUG_MT8188,
	.vm_cmdq_off = 0x200,
	.host_cmdq_off = 0xd00,
};

static inline struct mtk_dsi_priv *host_to_dsi(struct mipi_dsi_host *host)
{
	return container_of(host, struct mtk_dsi_priv, dsi_host);
}

static void mtk_dsi_mask(struct mtk_dsi_priv *dsi, u32 offset, u32 val, u32 mask)
{
	void __iomem *addr = dsi->base + offset;
	u32 tmp = readl(addr) & ~mask;

	tmp |= (val & mask);
	writel(tmp, addr);
}

static void mtk_dsi_shadow_debug(struct mtk_dsi_priv *dsi)
{
	writel(FORCE_COMMIT | BYPASS_SHADOW, dsi->base + dsi->data->shadow_dbg_off);
}

static void mtk_dsi_enable(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_CON_CTRL, DSI_EN, DSI_EN);
}

static void mtk_dsi_disable(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_CON_CTRL, 0, DSI_EN);
}

static void mtk_dsi_reset_engine(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_CON_CTRL, DSI_RESET, DSI_RESET);
	mtk_dsi_mask(dsi, DSI_CON_CTRL, 0, DSI_RESET);
}

static void mtk_dsi_reset_dphy(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_CON_CTRL, DPHY_RESET, DPHY_RESET);
	mtk_dsi_mask(dsi, DSI_CON_CTRL, 0, DPHY_RESET);
}

static void mtk_dsi_clk_ulp_mode_leave(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_PHY_LCCON, 0, LC_ULPM_EN);
	mtk_dsi_mask(dsi, DSI_PHY_LCCON, LC_WAKEUP_EN, LC_WAKEUP_EN);
	mtk_dsi_mask(dsi, DSI_PHY_LCCON, 0, LC_WAKEUP_EN);
}

static void mtk_dsi_clk_ulp_mode_enter(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_PHY_LCCON, 0, LC_HS_TX_EN);
	mtk_dsi_mask(dsi, DSI_PHY_LCCON, 0, LC_ULPM_EN);
}

static void mtk_dsi_lane0_ulp_mode_enter(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_PHY_LD0CON, 0, LD0_HS_TX_EN);
	mtk_dsi_mask(dsi, DSI_PHY_LD0CON, 0, LD0_ULPM_EN);
}

static void mtk_dsi_lane0_ulp_mode_leave(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, DSI_PHY_LD0CON, 0, LD0_ULPM_EN);
	mtk_dsi_mask(dsi, DSI_PHY_LD0CON, LD0_WAKEUP_EN, LD0_WAKEUP_EN);
	mtk_dsi_mask(dsi, DSI_PHY_LD0CON, 0, LD0_WAKEUP_EN);
}

static bool mtk_dsi_clk_hs_state(struct mtk_dsi_priv *dsi)
{
	return readl(dsi->base + DSI_PHY_LCCON) & LC_HS_TX_EN;
}

static void mtk_dsi_clk_hs_mode(struct mtk_dsi_priv *dsi, bool enter)
{
	if (enter && !mtk_dsi_clk_hs_state(dsi))
		mtk_dsi_mask(dsi, DSI_PHY_LCCON, LC_HS_TX_EN, LC_HS_TX_EN);
	else if (!enter && mtk_dsi_clk_hs_state(dsi))
		mtk_dsi_mask(dsi, DSI_PHY_LCCON, 0, LC_HS_TX_EN);
}

static void mtk_dsi_phy_timconfig(struct mtk_dsi_priv *dsi)
{
	struct mtk_dsi_phy_timing *timing = &dsi->phy_timing;
	u32 data_rate_mhz = DIV_ROUND_UP_ULL(dsi->data_rate, 1000000);
	u32 timcon0, timcon1, timcon2, timcon3;

	timing->lpx = (60 * data_rate_mhz / (8 * 1000)) + 1;
	timing->da_hs_prepare = (80 * data_rate_mhz + 4 * 1000) / 8000;
	timing->da_hs_zero = (170 * data_rate_mhz + 10 * 1000) / 8000 + 1 -
			     timing->da_hs_prepare;
	timing->da_hs_trail = timing->da_hs_prepare + 1;

	timing->ta_go = 4 * timing->lpx - 2;
	timing->ta_sure = timing->lpx + 2;
	timing->ta_get = 4 * timing->lpx;
	timing->da_hs_exit = 2 * timing->lpx + 1;

	timing->clk_hs_prepare = 70 * data_rate_mhz / (8 * 1000);
	timing->clk_hs_post = timing->clk_hs_prepare + 8;
	timing->clk_hs_trail = timing->clk_hs_prepare;
	timing->clk_hs_zero = timing->clk_hs_trail * 4;
	timing->clk_hs_exit = 2 * timing->clk_hs_trail;

	timcon0 = FIELD_PREP(LPX, timing->lpx) |
		  FIELD_PREP(HS_PREP, timing->da_hs_prepare) |
		  FIELD_PREP(HS_ZERO, timing->da_hs_zero) |
		  FIELD_PREP(HS_TRAIL, timing->da_hs_trail);

	timcon1 = FIELD_PREP(TA_GO, timing->ta_go) |
		  FIELD_PREP(TA_SURE, timing->ta_sure) |
		  FIELD_PREP(TA_GET, timing->ta_get) |
		  FIELD_PREP(DA_HS_EXIT, timing->da_hs_exit);

	timcon2 = FIELD_PREP(DA_HS_SYNC, 1) |
		  FIELD_PREP(CLK_ZERO, timing->clk_hs_zero) |
		  FIELD_PREP(CLK_TRAIL, timing->clk_hs_trail);

	timcon3 = FIELD_PREP(CLK_HS_PREP, timing->clk_hs_prepare) |
		  FIELD_PREP(CLK_HS_POST, timing->clk_hs_post) |
		  FIELD_PREP(CLK_HS_EXIT, timing->clk_hs_exit);

	writel(timcon0, dsi->base + DSI_PHY_TIMECON0);
	writel(timcon1, dsi->base + DSI_PHY_TIMECON1);
	writel(timcon2, dsi->base + DSI_PHY_TIMECON2);
	writel(timcon3, dsi->base + DSI_PHY_TIMECON3);
}

static void mtk_dsi_set_interrupt_enable(struct mtk_dsi_priv *dsi)
{
	u32 inten = LPRX_RD_RDY_INT_FLAG | CMD_DONE_INT_FLAG | VM_DONE_INT_FLAG;

	writel(inten, dsi->base + DSI_INTEN);
}

static void mtk_dsi_set_vm_cmd(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_mask(dsi, dsi->data->vm_cmdq_off, VM_CMD_EN, VM_CMD_EN);
	mtk_dsi_mask(dsi, dsi->data->vm_cmdq_off, TS_VFP_EN, TS_VFP_EN);
}

static void mtk_dsi_rxtx_control(struct mtk_dsi_priv *dsi)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dsi->panel);
	u32 lane_bits = 0, reg;
	u8 i;

	/* one bit per enabled data lane, four lanes at most */
	for (i = 0; i < plat->lanes; i++)
		lane_bits |= BIT(i);

	reg = FIELD_PREP(LANE_NUM, lane_bits);

	if (plat->mode_flags & MIPI_DSI_CLOCK_NON_CONTINUOUS)
		reg |= HSTX_CKLP_EN;

	if (plat->mode_flags & MIPI_DSI_MODE_EOT_PACKET)
		reg |= DIS_EOT;

	writel(reg, dsi->base + DSI_TXRX_CTRL);
}

static void mtk_dsi_lane_ready(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_rxtx_control(dsi);
	udelay(100);
	mtk_dsi_reset_dphy(dsi);
	mtk_dsi_clk_ulp_mode_leave(dsi);
	mtk_dsi_lane0_ulp_mode_leave(dsi);
	mtk_dsi_clk_hs_mode(dsi, false);
	udelay(3000);
}

static void mtk_dsi_ps_control(struct mtk_dsi_priv *dsi,
			       const struct display_timing *timing)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dsi->panel);
	u32 bpp, ps_sel, ps_wc;

	switch (plat->format) {
	case MIPI_DSI_FMT_RGB666:
		ps_sel = LOOSELY_PS_24BIT_RGB666;
		bpp = 3;
		break;
	case MIPI_DSI_FMT_RGB666_PACKED:
		ps_sel = PACKED_PS_18BIT_RGB666;
		bpp = 3;
		break;
	case MIPI_DSI_FMT_RGB565:
		ps_sel = PACKED_PS_16BIT_RGB565;
		bpp = 2;
		break;
	case MIPI_DSI_FMT_RGB888:
	default:
		ps_sel = PACKED_PS_24BIT_RGB888;
		bpp = 3;
		break;
	}

	ps_wc = FIELD_PREP(DSI_PS_WC, timing->hactive.typ * bpp);

	/*
	 * The clock lane needs the same word count as the pixel stream, or it
	 * never sustains HS for the whole data burst and no video frame ever
	 * completes.
	 */
	writel(ps_wc, dsi->base + DSI_HSTX_CKL_WC);

	writel(FIELD_PREP(DSI_PS_SEL, ps_sel) | ps_wc, dsi->base + DSI_PSCTRL);
}

static void mtk_dsi_config_vdo_timing(struct mtk_dsi_priv *dsi,
				      const struct display_timing *timing)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dsi->panel);
	const struct mtk_dsi_phy_timing *phy = &dsi->phy_timing;
	u32 hsa, hbp, hfp, bpp;
	u32 delta, front_back, data_phy_cycles, data_phy_cycles_byte;

	bpp = (plat->format == MIPI_DSI_FMT_RGB565) ? 2 : 3;

	writel(timing->vsync_len.typ, dsi->base + DSI_VSA_NL);
	writel(timing->vback_porch.typ, dsi->base + DSI_VBP_NL);
	writel(timing->vfront_porch.typ, dsi->base + DSI_VFP_NL);
	writel(timing->vactive.typ, dsi->base + DSI_VACT_NL);

	writel(FIELD_PREP(DSI_SIZE_CON_WIDTH, timing->hactive.typ) |
	       FIELD_PREP(DSI_SIZE_CON_HEIGHT, timing->vactive.typ),
	       dsi->base + DSI_SIZE_CON);

	hsa = (timing->hsync_len.typ * bpp - 10);

	if (plat->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE)
		hbp = timing->hback_porch.typ * bpp - 10;
	else
		hbp = (timing->hback_porch.typ + timing->hsync_len.typ) * bpp - 10;

	/*
	 * Number of D-PHY HS byte-clock cycles the panel needs to switch the
	 * data lanes from LP to HS and back. It must be subtracted out of the
	 * HFP/HBP word counts or the horizontal blanking period ends up far
	 * too long.
	 */
	data_phy_cycles = phy->lpx + phy->da_hs_prepare + phy->da_hs_zero +
			  phy->da_hs_exit + 3;

	/* burst/sync-pulse base overhead, plus 2 when the EOT packet is sent */
	delta = (plat->mode_flags & MIPI_DSI_MODE_VIDEO_BURST) ? 18 : 12;
	delta += (plat->mode_flags & MIPI_DSI_MODE_EOT_PACKET) ? 0 : 2;

	/*
	 * The d-phy allowance is shared between the two porches in proportion
	 * to their word counts, so scale by the byte values rather than by the
	 * pixel counts they were derived from.
	 */
	hfp = timing->hfront_porch.typ * bpp;
	front_back = hfp + hbp;
	data_phy_cycles_byte = data_phy_cycles * plat->lanes + delta;

	if (front_back > data_phy_cycles_byte) {
		hfp -= data_phy_cycles_byte * hfp / front_back;
		hbp -= data_phy_cycles_byte * hbp / front_back;
	} else {
		dev_warn(dsi->dev, "HFP + HBP less than d-phy, FPS will under 60Hz\n");
	}

	writel(hsa, dsi->base + DSI_HSA_WC);
	writel(hbp, dsi->base + DSI_HBP_WC);
	writel(hfp, dsi->base + DSI_HFP_WC);

	mtk_dsi_ps_control(dsi, timing);
}

static void mtk_dsi_set_display_mode(struct mtk_dsi_priv *dsi,
				     const struct display_timing *timing)
{
	struct video_priv *priv = dev_get_uclass_priv(dsi->dev);

	priv->xsize = timing->hactive.typ;
	priv->ysize = timing->vactive.typ;
	priv->bpix = VIDEO_BPP32;

	mtk_dsi_config_vdo_timing(dsi, timing);
}

static void mtk_dsi_set_mode(struct mtk_dsi_priv *dsi)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dsi->panel);
	u32 vid_mode = CMD_MODE;

	if (plat->mode_flags & MIPI_DSI_MODE_VIDEO) {
		if (plat->mode_flags & MIPI_DSI_MODE_VIDEO_BURST)
			vid_mode = BURST_MODE;
		else if (plat->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE)
			vid_mode = SYNC_PULSE_MODE;
		else
			vid_mode = SYNC_EVENT_MODE;
	}

	writel(vid_mode, dsi->base + DSI_MODE_CTRL);
}

static void mtk_dsi_start(struct mtk_dsi_priv *dsi)
{
	writel(0, dsi->base + DSI_START);
	writel(1, dsi->base + DSI_START);
}

static void mtk_dsi_stop(struct mtk_dsi_priv *dsi)
{
	writel(0, dsi->base + DSI_START);
}

static void mtk_dsi_output_enable(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_set_mode(dsi);

	/* The panel's DCS init goes out in LP, so raise the clock lane only now. */
	mtk_dsi_clk_hs_mode(dsi, true);

	mtk_dsi_start(dsi);
}

static int mtk_dsi_power_on(struct mtk_dsi_priv *dsi,
			    const struct display_timing *timing)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dsi->panel);
	struct phy_configure_opts_mipi_dphy opts = { };
	int ret, bit_per_pixel;
	unsigned int i;

	bit_per_pixel = mipi_dsi_pixel_format_to_bpp(plat->format);
	if (bit_per_pixel < 0) {
		dev_err(dsi->dev, "unknown MIPI DSI format %d\n", plat->format);
		return bit_per_pixel;
	}

	dsi->data_rate = DIV_ROUND_UP_ULL((u64)timing->pixelclock.typ *
					  bit_per_pixel, plat->lanes);

	opts.hs_clk_rate = dsi->data_rate;
	opts.lanes = plat->lanes;

	ret = generic_phy_configure(&dsi->phy, &opts);
	if (ret) {
		dev_err(dsi->dev, "failed to set the PHY to %llu Hz: %d\n",
			dsi->data_rate, ret);
		return ret;
	}

	ret = generic_phy_power_on(&dsi->phy);
	if (ret) {
		dev_err(dsi->dev, "failed to power on the PHY: %d\n", ret);
		return ret;
	}

	ret = clk_enable(dsi->engine_clk);
	if (ret) {
		dev_err(dsi->dev, "failed to enable engine clk: %d\n", ret);
		goto err_phy_power_off;
	}

	ret = clk_enable(dsi->digital_clk);
	if (ret) {
		dev_err(dsi->dev, "failed to enable digital clk: %d\n", ret);
		goto err_disable_engine_clk;
	}

	ret = mtk_disp_comp_enable(dsi->mutex);
	if (ret) {
		dev_err(dsi->dev, "failed to enable mutex: %d\n", ret);
		goto err_disable_digital_clk;
	}

	for (i = 0; i < dsi->data->path_len; i++) {
		ret = mtk_disp_comp_enable(dsi->comps[i]);
		if (ret) {
			dev_err(dsi->dev, "failed to enable %s: %d\n",
				dsi->data->path[i].alias, ret);
			goto err_disable_comps;
		}
	}

	mtk_dsi_enable(dsi);

	if (dsi->data->shadow_dbg_off)
		mtk_dsi_shadow_debug(dsi);

	mtk_dsi_reset_engine(dsi);

	mtk_dsi_phy_timconfig(dsi);

	mtk_dsi_set_vm_cmd(dsi);

	mtk_dsi_set_interrupt_enable(dsi);

	return 0;

err_disable_comps:
	while (i--)
		mtk_disp_comp_disable(dsi->comps[i]);
	mtk_disp_comp_disable(dsi->mutex);
err_disable_digital_clk:
	clk_disable(dsi->digital_clk);
err_disable_engine_clk:
	clk_disable(dsi->engine_clk);
err_phy_power_off:
	generic_phy_power_off(&dsi->phy);

	return ret;
}

/*
 * Linux re-enables and resets the engine whenever one of its waits times
 * out; do the same, so that a command left unfinished does not wedge all
 * the ones after it.
 */
static void mtk_dsi_reset_after_timeout(struct mtk_dsi_priv *dsi)
{
	mtk_dsi_enable(dsi);
	mtk_dsi_reset_engine(dsi);
}

static int mtk_dsi_wait_for_idle(struct mtk_dsi_priv *dsi)
{
	u32 val;
	int ret;

	ret = readl_poll_timeout(dsi->base + DSI_INTSTA, val, !(val & DSI_BUSY),
				 2000000);
	if (ret)
		mtk_dsi_reset_after_timeout(dsi);

	return ret;
}

/*
 * Acknowledge the packet the engine has just flagged: it keeps BUSY set
 * until the acknowledge is taken. Linux spins here in its interrupt
 * handler; U-Boot has no way out of a wedged engine, so give up eventually.
 */
static int mtk_dsi_ack_packet(struct mtk_dsi_priv *dsi)
{
	u32 tmp;
	int i;

	for (i = 0; i < MTK_DSI_RACK_RETRIES; i++) {
		mtk_dsi_mask(dsi, DSI_RACK, RACK, RACK);

		tmp = readl(dsi->base + DSI_INTSTA);
		if (!(tmp & DSI_BUSY))
			return 0;

		udelay(1);
	}

	return -ETIME;
}

/*
 * Wait for one of the @flag interrupt flags, 2 s at most as in Linux, then
 * acknowledge and clear every flag raised, as the Linux interrupt handler
 * does.
 */
static int mtk_dsi_wait_for_done(struct mtk_dsi_priv *dsi, u32 flag)
{
	u32 all = LPRX_RD_RDY_INT_FLAG | CMD_DONE_INT_FLAG | VM_DONE_INT_FLAG;
	u32 status;
	int ret;

	ret = readl_poll_timeout(dsi->base + DSI_INTSTA, status, status & flag,
				 2000000);
	if (ret) {
		dev_err(dsi->dev, "wait_for_done timeout: INTSTA=0x%08x\n",
			status);
		goto reset;
	}

	ret = mtk_dsi_ack_packet(dsi);
	mtk_dsi_mask(dsi, DSI_INTSTA, 0, status & all);
	if (ret) {
		dev_err(dsi->dev, "engine still busy after %d acknowledges\n",
			MTK_DSI_RACK_RETRIES);
		goto reset;
	}

	return 0;

reset:
	mtk_dsi_reset_after_timeout(dsi);

	return ret;
}

static void mtk_dsi_switch_to_cmd_mode(struct mtk_dsi_priv *dsi)
{
	if (!(readl(dsi->base + DSI_MODE_CTRL) & MODE_CON))
		return;

	writel(CMD_MODE, dsi->base + DSI_MODE_CTRL);

	/* the engine leaves the video mode at the end of the current frame */
	if (mtk_dsi_wait_for_done(dsi, VM_DONE_INT_FLAG))
		dev_warn(dsi->dev, "failed to switch to command mode\n");
}

static void mtk_dsi_power_off(struct mtk_dsi_priv *dsi)
{
	unsigned int i;

	/* undo mtk_dsi_power_on() and the lane setup, in the order Linux does */
	mtk_dsi_stop(dsi);
	mtk_dsi_switch_to_cmd_mode(dsi);
	mtk_dsi_reset_engine(dsi);
	mtk_dsi_lane0_ulp_mode_enter(dsi);
	mtk_dsi_clk_ulp_mode_enter(dsi);
	/* no lane enabled pulls the MIPI lines down */
	writel(0, dsi->base + DSI_TXRX_CTRL);
	mtk_dsi_disable(dsi);

	for (i = dsi->data->path_len; i--; )
		mtk_disp_comp_disable(dsi->comps[i]);

	mtk_disp_comp_disable(dsi->mutex);
	clk_disable(dsi->digital_clk);
	clk_disable(dsi->engine_clk);
	generic_phy_power_off(&dsi->phy);
}

static void mtk_dsi_cmdq(struct mtk_dsi_priv *dsi, const struct mipi_dsi_msg *msg)
{
	const char *tx_buf = msg->tx_buf;
	u8 config, cmdq_size, cmdq_off, type = msg->type;
	u32 reg_val, cmdq_mask, i;
	u32 reg_cmdq_off = dsi->data->host_cmdq_off;

	if (MTK_DSI_HOST_IS_READ(type))
		config = BTA;
	else
		config = (msg->tx_len > 2) ? LONG_PACKET : SHORT_PACKET;

	if (!(msg->flags & MIPI_DSI_MSG_USE_LPM))
		config |= HSTX;

	if (msg->tx_len > 2) {
		cmdq_size = 1 + (msg->tx_len + 3) / 4;
		cmdq_off = 4;
		cmdq_mask = CMDQ_CONFIG | CMDQ_DATA_ID | CMDQ_DATA_0 | CMDQ_DATA_1;
		reg_val = FIELD_PREP(CMDQ_WC, msg->tx_len) |
			  FIELD_PREP(CMDQ_DATA_ID, type) |
			  FIELD_PREP(CMDQ_CONFIG, config);
	} else {
		cmdq_size = 1;
		cmdq_off = 2;
		cmdq_mask = CMDQ_CONFIG | CMDQ_DATA_ID;
		reg_val = FIELD_PREP(CMDQ_DATA_ID, type) |
			  FIELD_PREP(CMDQ_CONFIG, config);
	}

	for (i = 0; i < msg->tx_len; i++)
		mtk_dsi_mask(dsi, (reg_cmdq_off + cmdq_off + i) & (~0x3U),
			     tx_buf[i] << (((i + cmdq_off) & 3U) * 8U),
			     (0xffUL << (((i + cmdq_off) & 3U) * 8U)));

	mtk_dsi_mask(dsi, reg_cmdq_off, reg_val, cmdq_mask);
	mtk_dsi_mask(dsi, DSI_CMDQ_SIZE, cmdq_size, CMDQ_SIZE);

	/* do not let the engine size the queue itself for long packets */
	if (dsi->data->cmdq_long_packet_ctl)
		mtk_dsi_mask(dsi, DSI_CMDQ_SIZE, CMDQ_SIZE_SEL, CMDQ_SIZE_SEL);
}

static u32 mtk_dsi_recv_cnt(struct mtk_dsi_priv *dsi, u8 type, u8 *read_data)
{
	switch (type) {
	case MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_1BYTE:
	case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE:
		return 1;
	case MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_2BYTE:
	case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE:
		return 2;
	case MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE:
	case MIPI_DSI_RX_DCS_LONG_READ_RESPONSE:
		return read_data[1] + read_data[2] * 16;
	case MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT:
		dev_info(dsi->dev, "type is 0x02, try again\n");
		break;
	default:
		dev_warn(dsi->dev, "type(0x%x) not recognized\n", type);
		break;
	}

	return 0;
}

static int mtk_dsi_host_send_cmd(struct mtk_dsi_priv *dsi,
				 const struct mipi_dsi_msg *msg, u32 flag)
{
	int ret;

	ret = mtk_dsi_wait_for_idle(dsi);
	if (ret) {
		dev_err(dsi->dev, "polling dsi wait not busy timeout!\n");
		return ret;
	}

	/* drop any flag left over, so that only this command completes it */
	mtk_dsi_mask(dsi, DSI_INTSTA, 0, flag);
	mtk_dsi_cmdq(dsi, msg);
	mtk_dsi_start(dsi);

	return mtk_dsi_wait_for_done(dsi, flag);
}

static ssize_t mtk_dsi_host_transfer(struct mipi_dsi_host *host,
				     const struct mipi_dsi_msg *msg)
{
	struct mtk_dsi_priv *dsi = host_to_dsi(host);
	ssize_t recv_cnt;
	u8 read_data[16];
	u32 flag = CMD_DONE_INT_FLAG;
	void *src_addr;
	int ret;
	u8 i;

	if (readl(dsi->base + DSI_MODE_CTRL) & MODE_CON) {
		dev_err(dsi->dev, "dsi engine is not command mode\n");
		return -EINVAL;
	}

	if (MTK_DSI_HOST_IS_READ(msg->type))
		flag |= LPRX_RD_RDY_INT_FLAG;

	ret = mtk_dsi_host_send_cmd(dsi, msg, flag);
	if (ret)
		return ret;

	if (!MTK_DSI_HOST_IS_READ(msg->type))
		return 0;

	if (!msg->rx_buf) {
		dev_err(dsi->dev, "dsi receive buffer size may be NULL\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(read_data); i++)
		read_data[i] = readb(dsi->base + DSI_RX_DATA0 + i);

	recv_cnt = mtk_dsi_recv_cnt(dsi, read_data[0], read_data);

	if (recv_cnt > 2)
		src_addr = &read_data[4];
	else
		src_addr = &read_data[1];

	if (recv_cnt > 10)
		recv_cnt = 10;

	if (recv_cnt > msg->rx_len)
		recv_cnt = msg->rx_len;

	if (recv_cnt)
		memcpy(msg->rx_buf, src_addr, recv_cnt);

	dev_dbg(dsi->dev, "dsi get %zd byte data from the panel address(0x%x)\n",
		recv_cnt, *((const u8 *)msg->tx_buf));

	return recv_cnt;
}

static int mtk_dsi_host_attach(struct mipi_dsi_host *host,
			       struct mipi_dsi_device *device)
{
	return 0;
}

static const struct mipi_dsi_host_ops mtk_dsi_host_ops = {
	.attach	  = mtk_dsi_host_attach,
	.transfer = mtk_dsi_host_transfer,
};

static void mtk_dsi_host_init(struct mtk_dsi_priv *dsi)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dsi->panel);
	struct mipi_dsi_device *device = &dsi->mipi;

	/*
	 * Populate the mipi_dsi_device the panel will use. Without ->dev the
	 * panel's dev_err()/logging prints "(NULL udevice *)", and the lane/
	 * format/mode_flags are needed by mipi_dsi helpers.
	 */
	device->dev = dsi->dev;
	device->lanes = plat->lanes;
	device->format = plat->format;
	device->mode_flags = plat->mode_flags;

	plat->device = device;

	dsi->dsi_host.dev = (struct device *)dsi->dev;
	dsi->dsi_host.ops = &mtk_dsi_host_ops;
	device->host = &dsi->dsi_host;
}

static int mtk_dsi_probe(struct udevice *dev)
{
	struct mtk_dsi_priv *dsi = dev_get_priv(dev);
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct display_timing timing;
	struct udevice *mmsys_dev;
	struct regmap *regmap;
	const char *alias;
	unsigned int i;
	int ret;

	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	dsi->dev = dev;
	dsi->data = (const struct mtk_dsi_data *)dev_get_driver_data(dev);
	if (!dsi->data) {
		dev_err(dev, "missing dsi match data\n");
		return -EINVAL;
	}

	dsi->base = dev_remap_addr(dev);
	if (!dsi->base)
		return -EINVAL;

	dsi->engine_clk = devm_clk_get(dev, "engine");
	if (IS_ERR(dsi->engine_clk))
		return PTR_ERR(dsi->engine_clk);

	/*
	 * The DSI host's "hs" clock is the MIPI TX PLL. U-Boot's MIPI TX driver
	 * does not register it as a clock, so it is not fetched here; its rate
	 * is set through generic_phy_configure() instead.
	 */
	dsi->digital_clk = devm_clk_get(dev, "digital");
	if (IS_ERR(dsi->digital_clk))
		return PTR_ERR(dsi->digital_clk);

	if (dsi->data->path_len > MTK_DSI_MAX_COMPS)
		return -E2BIG;

	for (i = 0; i < dsi->data->path_len; i++) {
		alias = dsi->data->path[i].alias;
		ret = uclass_get_device_by_of_path(UCLASS_MISC, alias,
						   &dsi->comps[i]);
		if (ret) {
			dev_err(dev, "cannot get %s device: %s\n", alias,
				errno_str(ret));
			return ret;
		}
	}

	/*
	 * Unlike the components above, the mutex has no alias in the MT8365
	 * device tree, so take the first device bound to its driver, as Linux
	 * does when the "mutex" alias is missing. All the SoCs supported here
	 * have a single mutex node; a SoC with more than one mmsys would need
	 * the mutex sitting next to this DSI to be singled out.
	 */
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(mtk_disp_mutex),
					  &dsi->mutex);
	if (ret) {
		dev_err(dev, "cannot get mutex device: %s\n", errno_str(ret));
		return ret;
	}

	/*
	 * No standalone "mmsys" phandle exists in the DSI binding; the
	 * "digital" clock already comes from the mmsys config block, so
	 * reuse its clock provider device to reach the same regmap.
	 */
	mmsys_dev = dsi->digital_clk->dev;

	regmap = syscon_node_to_regmap(dev_ofnode(mmsys_dev));
	if (IS_ERR(regmap)) {
		dev_err(dev, "cannot get mmsys regmap: %ld\n", PTR_ERR(regmap));
		return PTR_ERR(regmap);
	}

	dsi->mmsys_base = regmap_get_range(regmap, 0);
	if (!dsi->mmsys_base) {
		dev_err(dev, "cannot get mmsys base\n");
		return -ENOENT;
	}

	ret = generic_phy_get_by_name(dev, "dphy", &dsi->phy);
	if (ret) {
		dev_err(dev, "failed to get dsi PHY: %d\n", ret);
		return ret;
	}

	/* the panel is the DSI node's own child, bound by .bind below */
	ret = uclass_get_device_by_ofnode(UCLASS_PANEL,
					  dev_read_subnode(dev, "panel"),
					  &dsi->panel);
	if (ret) {
		dev_err(dev, "cannot get panel: %s\n", errno_str(ret));
		return ret;
	}

	mtk_dsi_host_init(dsi);

	ret = panel_get_display_timing(dsi->panel, &timing);
	if (ret) {
		dev_err(dev, "cannot get display timing\n");
		return ret;
	}

	/* the components are only programmed once their clocks are on */
	ret = mtk_dsi_power_on(dsi, &timing);
	if (ret) {
		dev_err(dev, "failed to power on\n");
		return ret;
	}

	ret = mtk_disp_mutex_ovl_dsi_enable(dsi->mutex);
	if (ret)
		goto err_power_off;

	dsi->data->ddp_connect(dsi->mmsys_base);

	for (i = 0; i < dsi->data->path_len; i++) {
		dsi->data->path[i].config(dsi->comps[i], &timing);
		dsi->data->path[i].start(dsi->comps[i]);
	}

	mtk_dsi_set_display_mode(dsi, &timing);

	mtk_dsi_lane_ready(dsi);

	/* the overlay comes first in every path */
	ret = mtk_ovl_layer(dsi->comps[0], plat, &timing);
	if (ret)
		goto err_power_off;

	video_set_flush_dcache(dev, true);

	ret = panel_set_backlight(dsi->panel, BACKLIGHT_DEFAULT);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to set backlight\n");
		goto err_power_off;
	}

	ret = panel_enable_backlight(dsi->panel);
	if (ret) {
		dev_err(dev, "failed to enable backlight\n");
		goto err_power_off;
	}

	mtk_dsi_output_enable(dsi);

	return 0;

err_power_off:
	mtk_dsi_power_off(dsi);

	return ret;
}

/*
 * Linux cannot take over a running pipeline: shut it down before booting
 * the OS, or the DMA masters keep scanning out memory the kernel reuses.
 */
static int mtk_dsi_remove(struct udevice *dev)
{
	struct mtk_dsi_priv *dsi = dev_get_priv(dev);

	/* nothing was set up before relocation */
	if (!dsi->dev)
		return 0;

	mtk_dsi_power_off(dsi);

	return 0;
}

static int mtk_dsi_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = MTK_DSI_MAX_WIDTH * MTK_DSI_MAX_HEIGHT * VNBYTES(VIDEO_BPP32);

	return dm_scan_fdt_dev(dev);
}

static const struct udevice_id mtk_dsi_ids[] = {
	{
		.compatible = "mediatek,mt8189-dsi",
		.data = (ulong)&mt8189_dsi_data,
	},
	{
		.compatible = "mediatek,mt8365-dsi",
		.data = (ulong)&mt8365_dsi_data,
	},
	{
		.compatible = "mediatek,mt8366-dsi",
		.data = (ulong)&mt8366_dsi_data,
	},
	{ }
};

U_BOOT_DRIVER(mtk_dsi) = {
	.name	   = "mtk_dsi",
	.id	   = UCLASS_VIDEO,
	.of_match  = mtk_dsi_ids,
	.probe	   = mtk_dsi_probe,
	.remove	   = mtk_dsi_remove,
	.bind	   = mtk_dsi_bind,
	.priv_auto = sizeof(struct mtk_dsi_priv),
	.flags	   = DM_FLAG_PRE_RELOC | DM_FLAG_OS_PREPARE,
};
