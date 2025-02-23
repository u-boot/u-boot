// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Texas Instruments Incorporated
 * Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/ofnode_graph.h>
#include <i2c.h>
#include <log.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <backlight.h>
#include <panel.h>
#include <video_bridge.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <power/regulator.h>

#include <asm/gpio.h>

/* Global (16-bit addressable) */
#define TC358768_CHIPID			0x0000
#define TC358768_SYSCTL			0x0002
#define TC358768_CONFCTL		0x0004
#define TC358768_VSDLY			0x0006
#define TC358768_DATAFMT		0x0008
#define TC358768_GPIOEN			0x000E
#define TC358768_GPIODIR		0x0010
#define TC358768_GPIOIN			0x0012
#define TC358768_GPIOOUT		0x0014
#define TC358768_PLLCTL0		0x0016
#define TC358768_PLLCTL1		0x0018
#define TC358768_CMDBYTE		0x0022
#define TC358768_PP_MISC		0x0032
#define TC358768_DSITX_DT		0x0050
#define TC358768_FIFOSTATUS		0x00F8

/* Debug (16-bit addressable) */
#define TC358768_VBUFCTRL		0x00E0
#define TC358768_DBG_WIDTH		0x00E2
#define TC358768_DBG_VBLANK		0x00E4
#define TC358768_DBG_DATA		0x00E8

/* TX PHY (32-bit addressable) */
#define TC358768_CLW_DPHYCONTTX		0x0100
#define TC358768_D0W_DPHYCONTTX		0x0104
#define TC358768_D1W_DPHYCONTTX		0x0108
#define TC358768_D2W_DPHYCONTTX		0x010C
#define TC358768_D3W_DPHYCONTTX		0x0110
#define TC358768_CLW_CNTRL		0x0140
#define TC358768_D0W_CNTRL		0x0144
#define TC358768_D1W_CNTRL		0x0148
#define TC358768_D2W_CNTRL		0x014C
#define TC358768_D3W_CNTRL		0x0150

/* TX PPI (32-bit addressable) */
#define TC358768_STARTCNTRL		0x0204
#define TC358768_DSITXSTATUS		0x0208
#define TC358768_LINEINITCNT		0x0210
#define TC358768_LPTXTIMECNT		0x0214
#define TC358768_TCLK_HEADERCNT		0x0218
#define TC358768_TCLK_TRAILCNT		0x021C
#define TC358768_THS_HEADERCNT		0x0220
#define TC358768_TWAKEUP		0x0224
#define TC358768_TCLK_POSTCNT		0x0228
#define TC358768_THS_TRAILCNT		0x022C
#define TC358768_HSTXVREGCNT		0x0230
#define TC358768_HSTXVREGEN		0x0234
#define TC358768_TXOPTIONCNTRL		0x0238
#define TC358768_BTACNTRL1		0x023C

/* TX CTRL (32-bit addressable) */
#define TC358768_DSI_CONTROL		0x040C
#define TC358768_DSI_STATUS		0x0410
#define TC358768_DSI_INT		0x0414
#define TC358768_DSI_INT_ENA		0x0418
#define TC358768_DSICMD_RDFIFO		0x0430
#define TC358768_DSI_ACKERR		0x0434
#define TC358768_DSI_ACKERR_INTENA	0x0438
#define TC358768_DSI_ACKERR_HALT	0x043c
#define TC358768_DSI_RXERR		0x0440
#define TC358768_DSI_RXERR_INTENA	0x0444
#define TC358768_DSI_RXERR_HALT		0x0448
#define TC358768_DSI_ERR		0x044C
#define TC358768_DSI_ERR_INTENA		0x0450
#define TC358768_DSI_ERR_HALT		0x0454
#define TC358768_DSI_CONFW		0x0500
#define TC358768_DSI_LPCMD		0x0500
#define TC358768_DSI_RESET		0x0504
#define TC358768_DSI_INT_CLR		0x050C
#define TC358768_DSI_START		0x0518

/* DSITX CTRL (16-bit addressable) */
#define TC358768_DSICMD_TX		0x0600
#define TC358768_DSICMD_TYPE		0x0602
#define TC358768_DSICMD_WC		0x0604
#define TC358768_DSICMD_WD0		0x0610
#define TC358768_DSICMD_WD1		0x0612
#define TC358768_DSICMD_WD2		0x0614
#define TC358768_DSICMD_WD3		0x0616
#define TC358768_DSI_EVENT		0x0620
#define TC358768_DSI_VSW		0x0622
#define TC358768_DSI_VBPR		0x0624
#define TC358768_DSI_VACT		0x0626
#define TC358768_DSI_HSW		0x0628
#define TC358768_DSI_HBPR		0x062A
#define TC358768_DSI_HACT		0x062C

/* TC358768_DSI_CONTROL (0x040C) register */
#define TC358768_DSI_CONTROL_DIS_MODE		BIT(15)
#define TC358768_DSI_CONTROL_TXMD		BIT(7)
#define TC358768_DSI_CONTROL_HSCKMD		BIT(5)
#define TC358768_DSI_CONTROL_EOTDIS		BIT(0)

/* TC358768_DSI_CONFW (0x0500) register */
#define TC358768_DSI_CONFW_MODE_SET		(5 << 29)
#define TC358768_DSI_CONFW_MODE_CLR		(6 << 29)
#define TC358768_DSI_CONFW_ADDR_DSI_CONTROL	(3 << 24)

#define NANO	1000000000UL
#define PICO	1000000000000ULL

static const char * const tc358768_supplies[] = {
	"vddc-supply", "vddmipi-supply", "vddio-supply"
};

struct tc358768_priv {
	struct mipi_dsi_host host;
	struct mipi_dsi_device device;

	struct udevice *panel;
	struct display_timing timing;

	struct udevice *supplies[ARRAY_SIZE(tc358768_supplies)];

	struct clk *refclk;

	struct gpio_desc reset_gpio;

	u32 pd_lines;	/* number of Parallel Port Input Data Lines */
	u32 dsi_lanes;	/* number of DSI Lanes */

	/* Parameters for PLL programming */
	u32 fbd;	/* PLL feedback divider */
	u32 prd;	/* PLL input divider */
	u32 frs;	/* PLL Freqency range for HSCK (post divider) */

	u32 dsiclk;	/* pll_clk / 2 */
};

static void tc358768_read(struct udevice *dev, u32 reg, u32 *val)
{
	int count;
	u8 buf[4] = { 0, 0, 0, 0 };

	/* 16-bit register? */
	if (reg < 0x100 || reg >= 0x600)
		count = 2;
	else
		count = 4;

	dm_i2c_read(dev, reg, buf, count);
	*val = (buf[0] <<  8) | (buf[1] & 0xff) |
	       (buf[2] << 24) | (buf[3] << 16);

	log_debug("%s 0x%04x >> 0x%08x\n",
		  __func__, reg, *val);
}

static void tc358768_write(struct udevice *dev, u32 reg, u32 val)
{
	int count;
	u8 buf[4];

	/* 16-bit register? */
	if (reg < 0x100 || reg >= 0x600)
		count = 2;
	else
		count = 4;

	buf[0] = val >> 8;
	buf[1] = val & 0xff;
	buf[2] = val >> 24;
	buf[3] = val >> 16;

	log_debug("%s 0x%04x << 0x%08x\n",
		  __func__, reg, val);

	dm_i2c_write(dev, reg, buf, count);
}

static void tc358768_update_bits(struct udevice *dev, u32 reg, u32 mask,
				 u32 val)
{
	u32 tmp, orig;

	tc358768_read(dev, reg, &orig);

	tmp = orig & ~mask;
	tmp |= val & mask;
	if (tmp != orig)
		tc358768_write(dev, reg, tmp);
}

static ssize_t tc358768_dsi_host_transfer(struct mipi_dsi_host *host,
					  const struct mipi_dsi_msg *msg)
{
	struct udevice *dev = (struct udevice *)host->dev;
	struct mipi_dsi_packet packet;
	int ret;

	if (msg->rx_len) {
		log_debug("%s: MIPI rx is not supported\n", __func__);
		return -EOPNOTSUPP;
	}

	if (msg->tx_len > 8) {
		log_debug("%s: Maximum 8 byte MIPI tx is supported\n", __func__);
		return -EOPNOTSUPP;
	}

	ret = mipi_dsi_create_packet(&packet, msg);
	if (ret)
		return ret;

	if (mipi_dsi_packet_format_is_short(msg->type)) {
		tc358768_write(dev, TC358768_DSICMD_TYPE,
			       (0x10 << 8) | (packet.header[0] & 0x3f));
		tc358768_write(dev, TC358768_DSICMD_WC, 0);
		tc358768_write(dev, TC358768_DSICMD_WD0,
			       (packet.header[2] << 8) | packet.header[1]);
	} else {
		int i;

		tc358768_write(dev, TC358768_DSICMD_TYPE,
			       (0x40 << 8) | (packet.header[0] & 0x3f));
		tc358768_write(dev, TC358768_DSICMD_WC, packet.payload_length);
		for (i = 0; i < packet.payload_length; i += 2) {
			u16 val = packet.payload[i];

			if (i + 1 < packet.payload_length)
				val |= packet.payload[i + 1] << 8;

			tc358768_write(dev, TC358768_DSICMD_WD0 + i, val);
		}
	}

	/* start transfer */
	tc358768_write(dev, TC358768_DSICMD_TX, 1);

	return packet.size;
}

static const struct mipi_dsi_host_ops tc358768_dsi_host_ops = {
	.transfer = tc358768_dsi_host_transfer,
};

static void tc358768_sw_reset(struct udevice *dev)
{
	/* Assert Reset */
	tc358768_write(dev, TC358768_SYSCTL, 1);
	mdelay(5);

	/* Release Reset, Exit Sleep */
	tc358768_write(dev, TC358768_SYSCTL, 0);
}

static void tc358768_hw_enable(struct udevice *dev)
{
	struct tc358768_priv *priv = dev_get_priv(dev);
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	ret = clk_prepare_enable(priv->refclk);
	if (ret)
		log_debug("%s: error enabling refclk (%d)\n", __func__, ret);

	ret = regulator_set_enable_if_allowed(priv->supplies[0], true);
	if (ret)
		log_debug("%s: error enabling vddc (%d)\n", __func__, ret);

	ret = regulator_set_enable_if_allowed(priv->supplies[1], true);
	if (ret)
		log_debug("%s: error enabling vddmipi (%d)\n", __func__, ret);

	mdelay(10);

	ret = regulator_set_enable_if_allowed(priv->supplies[2], true);
	if (ret)
		log_debug("%s: error enabling vddio (%d)\n", __func__, ret);

	mdelay(2);

	/*
	 * The RESX is active low (GPIO_ACTIVE_LOW).
	 * DEASSERT (value = 0) the reset_gpio to enable the chip
	 */
	ret = dm_gpio_set_value(&uc_priv->reset, 0);
	if (ret)
		log_debug("%s: error changing reset-gpio (%d)\n", __func__, ret);

	/* wait for encoder clocks to stabilize */
	mdelay(2);
}

static u32 tc358768_pclk_to_pll(struct tc358768_priv *priv, u32 pclk)
{
	return (u32)div_u64((u64)pclk * priv->pd_lines, priv->dsi_lanes);
}

static int tc358768_calc_pll(struct tc358768_priv *priv,
			     struct display_timing *dt)
{
	static const u32 frs_limits[] = {
		1000000000,
		500000000,
		250000000,
		125000000,
		62500000
	};
	unsigned long refclk;
	u32 prd, target_pll, i, max_pll, min_pll;
	u32 frs, best_diff, best_pll, best_prd, best_fbd;

	target_pll = tc358768_pclk_to_pll(priv, dt->pixelclock.typ);

	/* pll_clk = RefClk * FBD / PRD * (1 / (2^FRS)) */

	for (i = 0; i < ARRAY_SIZE(frs_limits); i++)
		if (target_pll >= frs_limits[i])
			break;

	if (i == ARRAY_SIZE(frs_limits) || i == 0)
		return -EINVAL;

	frs = i - 1;
	max_pll = frs_limits[i - 1];
	min_pll = frs_limits[i];

	refclk = clk_get_rate(priv->refclk);

	best_diff = UINT_MAX;
	best_pll = 0;
	best_prd = 0;
	best_fbd = 0;

	for (prd = 1; prd <= 16; ++prd) {
		u32 divisor = prd * (1 << frs);
		u32 fbd;

		for (fbd = 1; fbd <= 512; ++fbd) {
			u32 pll, diff, pll_in;

			pll = (u32)div_u64((u64)refclk * fbd, divisor);

			if (pll >= max_pll || pll < min_pll)
				continue;

			pll_in = (u32)div_u64((u64)refclk, prd);
			if (pll_in < 4000000)
				continue;

			diff = max(pll, target_pll) - min(pll, target_pll);

			if (diff < best_diff) {
				best_diff = diff;
				best_pll = pll;
				best_prd = prd;
				best_fbd = fbd;

				if (best_diff == 0)
					goto found;
			}
		}
	}

	if (best_diff == UINT_MAX) {
		log_debug("%s: could not find suitable PLL setup\n", __func__);
		return -EINVAL;
	}

found:
	priv->fbd = best_fbd;
	priv->prd = best_prd;
	priv->frs = frs;
	priv->dsiclk = best_pll / 2;

	return 0;
}

static void tc358768_setup_pll(struct udevice *dev)
{
	struct tc358768_priv *priv = dev_get_priv(dev);
	u32 fbd, prd, frs;
	int ret;

	ret = tc358768_calc_pll(priv, &priv->timing);
	if (ret)
		log_debug("%s: PLL calculation failed: %d\n", __func__, ret);

	fbd = priv->fbd;
	prd = priv->prd;
	frs = priv->frs;

	log_debug("%s: PLL: refclk %lu, fbd %u, prd %u, frs %u\n", __func__,
		  clk_get_rate(priv->refclk), fbd, prd, frs);
	log_debug("%s: PLL: pll_clk: %u, DSIClk %u, HSByteClk %u\n", __func__,
		  priv->dsiclk * 2, priv->dsiclk, priv->dsiclk / 4);

	/* PRD[15:12] FBD[8:0] */
	tc358768_write(dev, TC358768_PLLCTL0, ((prd - 1) << 12) | (fbd - 1));

	/* FRS[11:10] LBWS[9:8] CKEN[4] RESETB[1] EN[0] */
	tc358768_write(dev, TC358768_PLLCTL1,
		       (frs << 10) | (0x2 << 8) | BIT(1) | BIT(0));

	/* wait for lock */
	mdelay(5);

	/* FRS[11:10] LBWS[9:8] CKEN[4] PLL_CKEN[4] RESETB[1] EN[0] */
	tc358768_write(dev, TC358768_PLLCTL1,
		       (frs << 10) | (0x2 << 8) | BIT(4) | BIT(1) | BIT(0));
}

static u32 tc358768_ns_to_cnt(u32 ns, u32 period_ps)
{
	return DIV_ROUND_UP(ns * 1000, period_ps);
}

static u32 tc358768_ps_to_ns(u32 ps)
{
	return ps / 1000;
}

static u32 tc358768_dpi_to_ns(u32 val, u32 pclk)
{
	return (u32)div_u64((u64)val * NANO, pclk);
}

/* Convert value in DPI pixel clock units to DSI byte count */
static u32 tc358768_dpi_to_dsi_bytes(struct tc358768_priv *priv, u32 val)
{
	u64 m = (u64)val * priv->dsiclk / 4 * priv->dsi_lanes;
	u64 n = priv->timing.pixelclock.typ;

	return (u32)div_u64(m + n - 1, n);
}

static u32 tc358768_dsi_bytes_to_ns(struct tc358768_priv *priv, u32 val)
{
	u64 m = (u64)val * NANO;
	u64 n = priv->dsiclk / 4 * priv->dsi_lanes;

	return (u32)div_u64(m, n);
}

static int tc358768_attach(struct udevice *dev)
{
	struct tc358768_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct display_timing *dt = &priv->timing;
	u32 val, val2, lptxcnt, hact, data_type;
	s32 raw_val;
	u32 hsbyteclk_ps, dsiclk_ps, ui_ps;
	u32 dsiclk, hsbyteclk;
	int i;
	/* In pixelclock units */
	u32 dpi_htot, dpi_data_start;
	/* In byte units */
	u32 dsi_dpi_htot, dsi_dpi_data_start;
	u32 dsi_hsw, dsi_hbp, dsi_hact, dsi_hfp;
	const u32 dsi_hss = 4; /* HSS is a short packet (4 bytes) */
	/* In hsbyteclk units */
	u32 dsi_vsdly;
	const u32 internal_dly = 40;

	if (device->mode_flags & MIPI_DSI_CLOCK_NON_CONTINUOUS) {
		debug("%s: Non-continuous mode unimplemented, falling back to continuous\n", __func__);
		device->mode_flags &= ~MIPI_DSI_CLOCK_NON_CONTINUOUS;
	}

	tc358768_hw_enable(dev);
	tc358768_sw_reset(dev);

	tc358768_setup_pll(dev);

	dsiclk = priv->dsiclk;
	hsbyteclk = dsiclk / 4;

	/* Data Format Control Register */
	val = BIT(2) | BIT(1) | BIT(0); /* rdswap_en | dsitx_en | txdt_en */
	switch (device->format) {
	case MIPI_DSI_FMT_RGB888:
		val |= (0x3 << 4);
		hact = dt->hactive.typ * 3;
		data_type = MIPI_DSI_PACKED_PIXEL_STREAM_24;
		break;
	case MIPI_DSI_FMT_RGB666:
		val |= (0x4 << 4);
		hact = dt->hactive.typ * 3;
		data_type = MIPI_DSI_PACKED_PIXEL_STREAM_18;
		break;
	case MIPI_DSI_FMT_RGB666_PACKED:
		val |= (0x4 << 4) | BIT(3);
		hact = dt->hactive.typ * 18 / 8;
		data_type = MIPI_DSI_PIXEL_STREAM_3BYTE_18;
		break;
	case MIPI_DSI_FMT_RGB565:
		val |= (0x5 << 4);
		hact = dt->hactive.typ * 2;
		data_type = MIPI_DSI_PACKED_PIXEL_STREAM_16;
		break;
	default:
		log_debug("%s: Invalid data format (%u)\n",
			  __func__, device->format);
		return -EINVAL;
	}

	/*
	 * There are three important things to make TC358768 work correctly,
	 * which are not trivial to manage:
	 *
	 * 1. Keep the DPI line-time and the DSI line-time as close to each
	 *    other as possible.
	 * 2. TC358768 goes to LP mode after each line's active area. The DSI
	 *    HFP period has to be long enough for entering and exiting LP mode.
	 *    But it is not clear how to calculate this.
	 * 3. VSDly (video start delay) has to be long enough to ensure that the
	 *    DSI TX does not start transmitting until we have started receiving
	 *    pixel data from the DPI input. It is not clear how to calculate
	 *    this either.
	 */

	dpi_htot = dt->hactive.typ + dt->hfront_porch.typ +
		   dt->hsync_len.typ + dt->hback_porch.typ;
	dpi_data_start = dt->hsync_len.typ + dt->hback_porch.typ;

	log_debug("%s: dpi horiz timing (pclk): %u + %u + %u + %u = %u\n", __func__,
		  dt->hsync_len.typ, dt->hback_porch.typ, dt->hactive.typ,
		  dt->hfront_porch.typ, dpi_htot);

	log_debug("%s: dpi horiz timing (ns): %u + %u + %u + %u = %u\n", __func__,
		  tc358768_dpi_to_ns(dt->hsync_len.typ, dt->pixelclock.typ),
		  tc358768_dpi_to_ns(dt->hback_porch.typ, dt->pixelclock.typ),
		  tc358768_dpi_to_ns(dt->hactive.typ, dt->pixelclock.typ),
		  tc358768_dpi_to_ns(dt->hfront_porch.typ, dt->pixelclock.typ),
		  tc358768_dpi_to_ns(dpi_htot, dt->pixelclock.typ));

	log_debug("%s: dpi data start (ns): %u + %u = %u\n", __func__,
		  tc358768_dpi_to_ns(dt->hsync_len.typ, dt->pixelclock.typ),
		  tc358768_dpi_to_ns(dt->hback_porch.typ, dt->pixelclock.typ),
		  tc358768_dpi_to_ns(dpi_data_start, dt->pixelclock.typ));

	dsi_dpi_htot = tc358768_dpi_to_dsi_bytes(priv, dpi_htot);
	dsi_dpi_data_start = tc358768_dpi_to_dsi_bytes(priv, dpi_data_start);

	if (device->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) {
		dsi_hsw = tc358768_dpi_to_dsi_bytes(priv, dt->hsync_len.typ);
		dsi_hbp = tc358768_dpi_to_dsi_bytes(priv, dt->hback_porch.typ);
	} else {
		/* HBP is included in HSW in event mode */
		dsi_hbp = 0;
		dsi_hsw = tc358768_dpi_to_dsi_bytes(priv,
						    dt->hsync_len.typ +
						    dt->hback_porch.typ);

		/*
		 * The pixel packet includes the actual pixel data, and:
		 * DSI packet header = 4 bytes
		 * DCS code = 1 byte
		 * DSI packet footer = 2 bytes
		 */
		dsi_hact = hact + 4 + 1 + 2;

		dsi_hfp = dsi_dpi_htot - dsi_hact - dsi_hsw - dsi_hss;

		/*
		 * Here we should check if HFP is long enough for entering LP
		 * and exiting LP, but it's not clear how to calculate that.
		 * Instead, this is a naive algorithm that just adjusts the HFP
		 * and HSW so that HFP is (at least) roughly 2/3 of the total
		 * blanking time.
		 */
		if (dsi_hfp < (dsi_hfp + dsi_hsw + dsi_hss) * 2 / 3) {
			u32 old_hfp = dsi_hfp;
			u32 old_hsw = dsi_hsw;
			u32 tot = dsi_hfp + dsi_hsw + dsi_hss;

			dsi_hsw = tot / 3;

			/*
			 * Seems like sometimes HSW has to be divisible by num-lanes, but
			 * not always...
			 */
			dsi_hsw = roundup(dsi_hsw, priv->dsi_lanes);

			dsi_hfp = dsi_dpi_htot - dsi_hact - dsi_hsw - dsi_hss;

			log_debug("%s: hfp too short, adjusting dsi hfp and dsi hsw from %u, %u to %u, %u\n",
				  __func__, old_hfp, old_hsw, dsi_hfp, dsi_hsw);
		}

		log_debug("%s: dsi horiz timing (bytes): %u, %u + %u + %u + %u = %u\n", __func__,
			  dsi_hss, dsi_hsw, dsi_hbp, dsi_hact, dsi_hfp,
			  dsi_hss + dsi_hsw + dsi_hbp + dsi_hact + dsi_hfp);

		log_debug("%s: dsi horiz timing (ns): %u + %u + %u + %u + %u = %u\n", __func__,
			  tc358768_dsi_bytes_to_ns(priv, dsi_hss),
			  tc358768_dsi_bytes_to_ns(priv, dsi_hsw),
			  tc358768_dsi_bytes_to_ns(priv, dsi_hbp),
			  tc358768_dsi_bytes_to_ns(priv, dsi_hact),
			  tc358768_dsi_bytes_to_ns(priv, dsi_hfp),
			  tc358768_dsi_bytes_to_ns(priv, dsi_hss + dsi_hsw +
					       dsi_hbp + dsi_hact + dsi_hfp));
	}

	/* VSDly calculation */

	/* Start with the HW internal delay */
	dsi_vsdly = internal_dly;

	/* Convert to byte units as the other variables are in byte units */
	dsi_vsdly *= priv->dsi_lanes;

	/* Do we need more delay, in addition to the internal? */
	if (dsi_dpi_data_start > dsi_vsdly + dsi_hss + dsi_hsw + dsi_hbp) {
		dsi_vsdly = dsi_dpi_data_start - dsi_hss - dsi_hsw - dsi_hbp;
		dsi_vsdly = roundup(dsi_vsdly, priv->dsi_lanes);
	}

	log_debug("%s: dsi data start (bytes) %u + %u + %u + %u = %u\n", __func__,
		  dsi_vsdly, dsi_hss, dsi_hsw, dsi_hbp,
		  dsi_vsdly + dsi_hss + dsi_hsw + dsi_hbp);

	log_debug("%s: dsi data start (ns) %u + %u + %u + %u = %u\n", __func__,
		  tc358768_dsi_bytes_to_ns(priv, dsi_vsdly),
		  tc358768_dsi_bytes_to_ns(priv, dsi_hss),
		  tc358768_dsi_bytes_to_ns(priv, dsi_hsw),
		  tc358768_dsi_bytes_to_ns(priv, dsi_hbp),
		  tc358768_dsi_bytes_to_ns(priv, dsi_vsdly + dsi_hss + dsi_hsw + dsi_hbp));

	/* Convert back to hsbyteclk */
	dsi_vsdly /= priv->dsi_lanes;

	/*
	 * The docs say that there is an internal delay of 40 cycles.
	 * However, we get underflows if we follow that rule. If we
	 * instead ignore the internal delay, things work. So either
	 * the docs are wrong or the calculations are wrong.
	 *
	 * As a temporary fix, add the internal delay here, to counter
	 * the subtraction when writing the register.
	 */
	dsi_vsdly += internal_dly;

	/* Clamp to the register max */
	if (dsi_vsdly - internal_dly > 0x3ff) {
		log_warning("%s: VSDly too high, underflows likely\n", __func__);
		dsi_vsdly = 0x3ff + internal_dly;
	}

	/* VSDly[9:0] */
	tc358768_write(dev, TC358768_VSDLY, dsi_vsdly - internal_dly);

	tc358768_write(dev, TC358768_DATAFMT, val);
	tc358768_write(dev, TC358768_DSITX_DT, data_type);

	/* Enable D-PHY (HiZ->LP11) */
	tc358768_write(dev, TC358768_CLW_CNTRL, 0x0000);
	/* Enable lanes */
	for (i = 0; i < device->lanes; i++)
		tc358768_write(dev, TC358768_D0W_CNTRL + i * 4, 0x0000);

	/* Set up D-PHY CONTTX */
	tc358768_write(dev, TC358768_CLW_DPHYCONTTX, 0x0203);
	/* Adjust lanes */
	for (i = 0; i < device->lanes; i++)
		tc358768_write(dev, TC358768_D0W_DPHYCONTTX + i * 4, 0x0203);

	/* DSI Timings */
	hsbyteclk_ps = (u32)div_u64(PICO, hsbyteclk);
	dsiclk_ps = (u32)div_u64(PICO, dsiclk);
	ui_ps = dsiclk_ps / 2;
	log_debug("%s: dsiclk: %u ps, ui %u ps, hsbyteclk %u ps\n",
		  __func__, dsiclk_ps, ui_ps, hsbyteclk_ps);

	/* LP11 > 100us for D-PHY Rx Init */
	val = tc358768_ns_to_cnt(100 * 1000, hsbyteclk_ps) - 1;
	log_debug("%s: LINEINITCNT: 0x%x\n", __func__, val);
	tc358768_write(dev, TC358768_LINEINITCNT, val);

	/* LPTimeCnt > 50ns */
	val = tc358768_ns_to_cnt(50, hsbyteclk_ps) - 1;
	lptxcnt = val;
	log_debug("%s: LPTXTIMECNT: 0x%x\n", __func__, val);
	tc358768_write(dev, TC358768_LPTXTIMECNT, val);

	/* 38ns < TCLK_PREPARE < 95ns */
	val = tc358768_ns_to_cnt(65, hsbyteclk_ps) - 1;
	log_debug("%s: TCLK_PREPARECNT: 0x%x\n", __func__, val);
	/* TCLK_PREPARE + TCLK_ZERO > 300ns */
	val2 = tc358768_ns_to_cnt(300 - tc358768_ps_to_ns(2 * ui_ps),
				  hsbyteclk_ps) - 2;
	log_debug("%s: TCLK_ZEROCNT: 0x%x\n", __func__, val2);
	val |= val2 << 8;
	tc358768_write(dev, TC358768_TCLK_HEADERCNT, val);

	/* TCLK_TRAIL > 60ns AND TEOT <= 105 ns + 12*UI */
	raw_val = tc358768_ns_to_cnt(60 + tc358768_ps_to_ns(2 * ui_ps),
				     hsbyteclk_ps) - 5;
	val = clamp(raw_val, 0, 127);
	log_debug("%s: TCLK_TRAILCNT: 0x%x\n", __func__, val);
	tc358768_write(dev, TC358768_TCLK_TRAILCNT, val);

	/* 40ns + 4*UI < THS_PREPARE < 85ns + 6*UI */
	val = 50 + tc358768_ps_to_ns(4 * ui_ps);
	val = tc358768_ns_to_cnt(val, hsbyteclk_ps) - 1;
	log_debug("%s: THS_PREPARECNT: 0x%x\n", __func__, val);
	/* THS_PREPARE + THS_ZERO > 145ns + 10*UI */
	raw_val = tc358768_ns_to_cnt(145 - tc358768_ps_to_ns(3 * ui_ps),
				     hsbyteclk_ps) - 10;
	val2 = clamp(raw_val, 0, 127);
	log_debug("%s: THS_ZEROCNT: 0x%x\n", __func__, val2);
	val |= val2 << 8;
	tc358768_write(dev, TC358768_THS_HEADERCNT, val);

	/* TWAKEUP > 1ms in lptxcnt steps */
	val = tc358768_ns_to_cnt(1020000, hsbyteclk_ps);
	val = val / (lptxcnt + 1) - 1;
	log_debug("%s: TWAKEUP: 0x%x\n", __func__, val);
	tc358768_write(dev, TC358768_TWAKEUP, val);

	/* TCLK_POSTCNT > 60ns + 52*UI */
	val = tc358768_ns_to_cnt(60 + tc358768_ps_to_ns(52 * ui_ps),
				 hsbyteclk_ps) - 3;
	log_debug("%s: TCLK_POSTCNT: 0x%x\n", __func__, val);
	tc358768_write(dev, TC358768_TCLK_POSTCNT, val);

	/* max(60ns + 4*UI, 8*UI) < THS_TRAILCNT < 105ns + 12*UI */
	raw_val = tc358768_ns_to_cnt(60 + tc358768_ps_to_ns(18 * ui_ps),
				     hsbyteclk_ps) - 4;
	val = clamp(raw_val, 0, 15);
	log_debug("%s: THS_TRAILCNT: 0x%x\n", __func__, val);
	tc358768_write(dev, TC358768_THS_TRAILCNT, val);

	val = BIT(0);
	for (i = 0; i < device->lanes; i++)
		val |= BIT(i + 1);
	tc358768_write(dev, TC358768_HSTXVREGEN, val);

	tc358768_write(dev, TC358768_TXOPTIONCNTRL,
		       (device->mode_flags & MIPI_DSI_CLOCK_NON_CONTINUOUS) ? 0 : BIT(0));

	/* TXTAGOCNT[26:16] RXTASURECNT[10:0] */
	val = tc358768_ps_to_ns((lptxcnt + 1) * hsbyteclk_ps * 4);
	val = tc358768_ns_to_cnt(val, hsbyteclk_ps) / 4 - 1;
	log_debug("%s: TXTAGOCNT: 0x%x\n", __func__, val);
	val2 = tc358768_ns_to_cnt(tc358768_ps_to_ns((lptxcnt + 1) * hsbyteclk_ps),
				  hsbyteclk_ps) - 2;
	log_debug("%s: RXTASURECNT: 0x%x\n", __func__, val2);
	val = val << 16 | val2;
	tc358768_write(dev, TC358768_BTACNTRL1, val);

	/* START[0] */
	tc358768_write(dev, TC358768_STARTCNTRL, 1);

	if (device->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) {
		/* Set pulse mode */
		tc358768_write(dev, TC358768_DSI_EVENT, 0);

		/* vact */
		tc358768_write(dev, TC358768_DSI_VACT, dt->vactive.typ);
		/* vsw */
		tc358768_write(dev, TC358768_DSI_VSW, dt->vsync_len.typ);
		/* vbp */
		tc358768_write(dev, TC358768_DSI_VBPR, dt->vback_porch.typ);
	} else {
		/* Set event mode */
		tc358768_write(dev, TC358768_DSI_EVENT, 1);

		/* vact */
		tc358768_write(dev, TC358768_DSI_VACT, dt->vactive.typ);

		/* vsw (+ vbp) */
		tc358768_write(dev, TC358768_DSI_VSW,
			       dt->vsync_len.typ + dt->vback_porch.typ);
		/* vbp (not used in event mode) */
		tc358768_write(dev, TC358768_DSI_VBPR, 0);
	}

	/* hsw (bytes) */
	tc358768_write(dev, TC358768_DSI_HSW, dsi_hsw);

	/* hbp (bytes) */
	tc358768_write(dev, TC358768_DSI_HBPR, dsi_hbp);

	/* hact (bytes) */
	tc358768_write(dev, TC358768_DSI_HACT, hact);

	/* VSYNC polarity */
	tc358768_update_bits(dev, TC358768_CONFCTL, BIT(5),
			     (dt->flags & DISPLAY_FLAGS_VSYNC_HIGH) ? BIT(5) : 0);

	/* HSYNC polarity */
	tc358768_update_bits(dev, TC358768_PP_MISC, BIT(0),
			     (dt->flags & DISPLAY_FLAGS_HSYNC_LOW) ? BIT(0) : 0);

	/* Start DSI Tx */
	tc358768_write(dev, TC358768_DSI_START, 0x1);

	/* Configure DSI_Control register */
	val = TC358768_DSI_CONFW_MODE_CLR | TC358768_DSI_CONFW_ADDR_DSI_CONTROL;
	val |= TC358768_DSI_CONTROL_TXMD | TC358768_DSI_CONTROL_HSCKMD |
	       0x3 << 1 | TC358768_DSI_CONTROL_EOTDIS;
	tc358768_write(dev, TC358768_DSI_CONFW, val);

	val = TC358768_DSI_CONFW_MODE_SET | TC358768_DSI_CONFW_ADDR_DSI_CONTROL;
	val |= (device->lanes - 1) << 1;

	val |= TC358768_DSI_CONTROL_TXMD;

	if (!(device->mode_flags & MIPI_DSI_CLOCK_NON_CONTINUOUS))
		val |= TC358768_DSI_CONTROL_HSCKMD;

	/*
	 * TODO: Actually MIPI_DSI_MODE_NO_EOT_PACKET
	 *
	 * Many of the DSI flags have names opposite to their
	 * actual effects, e.g. MIPI_DSI_MODE_EOT_PACKET means
	 * that EoT packets will actually be disabled.
	 */
	if (device->mode_flags & MIPI_DSI_MODE_EOT_PACKET)
		val |= TC358768_DSI_CONTROL_EOTDIS;

	tc358768_write(dev, TC358768_DSI_CONFW, val);

	val = TC358768_DSI_CONFW_MODE_CLR |
	      TC358768_DSI_CONFW_ADDR_DSI_CONTROL |
	      TC358768_DSI_CONTROL_DIS_MODE; /* DSI mode */
	tc358768_write(dev, TC358768_DSI_CONFW, val);

	/* clear FrmStop and RstPtr */
	tc358768_update_bits(dev, TC358768_PP_MISC, 0x3 << 14, 0);

	/* set PP_en */
	tc358768_update_bits(dev, TC358768_CONFCTL, BIT(6), BIT(6));

	/* Set up panel configuration */
	return panel_enable_backlight(priv->panel);
}

static int tc358768_set_backlight(struct udevice *dev, int percent)
{
	struct tc358768_priv *priv = dev_get_priv(dev);

	return panel_set_backlight(priv->panel, percent);
}

static int tc358768_panel_timings(struct udevice *dev,
				  struct display_timing *timing)
{
	struct tc358768_priv *priv = dev_get_priv(dev);

	/* Default to positive sync */

	if (!(priv->timing.flags &
	      (DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_HSYNC_HIGH)))
		priv->timing.flags |= DISPLAY_FLAGS_HSYNC_HIGH;

	if (!(priv->timing.flags &
	      (DISPLAY_FLAGS_VSYNC_LOW | DISPLAY_FLAGS_VSYNC_HIGH)))
		priv->timing.flags |= DISPLAY_FLAGS_VSYNC_HIGH;

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

static int tc358768_get_panel(struct udevice *dev)
{
	struct tc358768_priv *priv = dev_get_priv(dev);
	int i, ret;

	u32 num = ofnode_graph_get_port_count(dev_ofnode(dev));

	for (i = 0; i < num; i++) {
		ofnode remote = ofnode_graph_get_remote_node(dev_ofnode(dev), i, -1);

		ret = uclass_get_device_by_ofnode(UCLASS_PANEL, remote,
						  &priv->panel);
		if (!ret)
			return 0;
	}

	/* If this point is reached, no panels were found */
	return -ENODEV;
}

static int tc358768_setup(struct udevice *dev)
{
	struct tc358768_priv *priv = dev_get_priv(dev);
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct mipi_dsi_panel_plat *mipi_plat;
	int i, ret;

	/* The bridge uses 16 bit registers */
	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret) {
		log_debug("%s: set_chip_offset_len failed: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = tc358768_get_panel(dev);
	if (ret) {
		log_debug("%s: panel not found, ret %d\n", __func__, ret);
		return ret;
	}

	panel_get_display_timing(priv->panel, &priv->timing);

	mipi_plat = dev_get_plat(priv->panel);
	mipi_plat->device = device;

	priv->host.dev = (struct device *)dev;
	priv->host.ops = &tc358768_dsi_host_ops;

	device->host = &priv->host;
	device->lanes = mipi_plat->lanes;
	device->format = mipi_plat->format;
	device->mode_flags = mipi_plat->mode_flags;

	priv->pd_lines = mipi_dsi_pixel_format_to_bpp(device->format);
	priv->dsi_lanes = device->lanes;

	/* get regulators */
	for (i = 0; i < ARRAY_SIZE(tc358768_supplies); i++) {
		ret = device_get_supply_regulator(dev, tc358768_supplies[i],
						  &priv->supplies[i]);
		if (ret) {
			log_debug("%s: cannot get %s %d\n", __func__,
				  tc358768_supplies[i], ret);
			if (ret != -ENOENT)
				return log_ret(ret);
		}
	}

	/* get clk */
	priv->refclk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->refclk)) {
		log_debug("%s: Could not get refclk: %ld\n",
			  __func__, PTR_ERR(priv->refclk));
		return PTR_ERR(priv->refclk);
	}

	dm_gpio_set_value(&uc_priv->reset, 1);

	return 0;
}

static int tc358768_probe(struct udevice *dev)
{
	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	return tc358768_setup(dev);
}

static const struct video_bridge_ops tc358768_ops = {
	.attach			= tc358768_attach,
	.set_backlight		= tc358768_set_backlight,
	.get_display_timing	= tc358768_panel_timings,
};

static const struct udevice_id tc358768_ids[] = {
	{ .compatible = "toshiba,tc358768" },
	{ .compatible = "toshiba,tc358778" },
	{ }
};

U_BOOT_DRIVER(tc358768) = {
	.name		= "tc358768",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= tc358768_ids,
	.ops		= &tc358768_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= tc358768_probe,
	.priv_auto	= sizeof(struct tc358768_priv),
};
