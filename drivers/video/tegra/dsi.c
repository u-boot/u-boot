// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 NVIDIA Corporation
 * Copyright (c) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <clk.h>
#include <log.h>
#include <misc.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <backlight.h>
#include <video_bridge.h>
#include <panel.h>
#include <reset.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/time.h>
#include <power/regulator.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>

#include "dc.h"
#include "dsi.h"
#include "mipi-phy.h"

/* List of supported DSI bridges */
enum {
	DSI_V0,
	DSI_V1,
};

struct tegra_dsi_priv {
	struct mipi_dsi_host host;
	struct mipi_dsi_device device;
	struct mipi_dphy_timing dphy_timing;

	struct udevice *panel;
	struct udevice *mipi;
	struct display_timing timing;

	struct dsi_ctlr *dsi;
	struct udevice *avdd;

	enum tegra_dsi_format format;

	struct clk *clk;
	struct clk *clk_parent;

	int video_fifo_depth;
	int host_fifo_depth;

	u32 calibration_pads;
	u32 version;

	/* for ganged-mode support */
	struct udevice *master;
	struct udevice *slave;
};

static void tegra_dc_enable_controller(struct udevice *dev)
{
	struct tegra_dc_plat *dc_plat = dev_get_plat(dev);
	struct dc_ctlr *dc = dc_plat->dc;
	u32 value;

	value = readl(&dc->disp.disp_win_opt);
	value |= DSI_ENABLE;
	writel(value, &dc->disp.disp_win_opt);

	writel(GENERAL_UPDATE, &dc->cmd.state_ctrl);
	writel(GENERAL_ACT_REQ, &dc->cmd.state_ctrl);
}

static const char * const error_report[16] = {
	"SoT Error",
	"SoT Sync Error",
	"EoT Sync Error",
	"Escape Mode Entry Command Error",
	"Low-Power Transmit Sync Error",
	"Peripheral Timeout Error",
	"False Control Error",
	"Contention Detected",
	"ECC Error, single-bit",
	"ECC Error, multi-bit",
	"Checksum Error",
	"DSI Data Type Not Recognized",
	"DSI VC ID Invalid",
	"Invalid Transmission Length",
	"Reserved",
	"DSI Protocol Violation",
};

static ssize_t tegra_dsi_read_response(struct dsi_misc_reg *misc,
				       const struct mipi_dsi_msg *msg,
				       size_t count)
{
	u8 *rx = msg->rx_buf;
	unsigned int i, j, k;
	size_t size = 0;
	u16 errors;
	u32 value;

	/* read and parse packet header */
	value = readl(&misc->dsi_rd_data);

	switch (value & 0x3f) {
	case MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT:
		errors = (value >> 8) & 0xffff;
		printf("%s: Acknowledge and error report: %04x\n",
		       __func__, errors);
		for (i = 0; i < ARRAY_SIZE(error_report); i++)
			if (errors & BIT(i))
				printf("%s:  %2u: %s\n", __func__, i,
				       error_report[i]);
		break;

	case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE:
		rx[0] = (value >> 8) & 0xff;
		size = 1;
		break;

	case MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE:
		rx[0] = (value >>  8) & 0xff;
		rx[1] = (value >> 16) & 0xff;
		size = 2;
		break;

	case MIPI_DSI_RX_DCS_LONG_READ_RESPONSE:
		size = ((value >> 8) & 0xff00) | ((value >> 8) & 0xff);
		break;

	case MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE:
		size = ((value >> 8) & 0xff00) | ((value >> 8) & 0xff);
		break;

	default:
		printf("%s: unhandled response type: %02x\n",
		       __func__, value & 0x3f);
		return -EPROTO;
	}

	size = min(size, msg->rx_len);

	if (msg->rx_buf && size > 0) {
		for (i = 0, j = 0; i < count - 1; i++, j += 4) {
			u8 *rx = msg->rx_buf + j;

			value = readl(&misc->dsi_rd_data);

			for (k = 0; k < 4 && (j + k) < msg->rx_len; k++)
				rx[j + k] = (value >> (k << 3)) & 0xff;
		}
	}

	return size;
}

static int tegra_dsi_transmit(struct dsi_misc_reg *misc,
			      unsigned long timeout)
{
	writel(DSI_TRIGGER_HOST, &misc->dsi_trigger);

	while (timeout--) {
		u32 value = readl(&misc->dsi_trigger);

		if ((value & DSI_TRIGGER_HOST) == 0)
			return 0;

		udelay(1000);
	}

	debug("timeout waiting for transmission to complete\n");
	return -ETIMEDOUT;
}

static int tegra_dsi_wait_for_response(struct dsi_misc_reg *misc,
				       unsigned long timeout)
{
	while (timeout--) {
		u32 value = readl(&misc->dsi_status);
		u8 count = value & 0x1f;

		if (count > 0)
			return count;

		udelay(1000);
	}

	debug("peripheral returned no data\n");
	return -ETIMEDOUT;
}

static void tegra_dsi_writesl(struct dsi_misc_reg *misc,
			      const void *buffer, size_t size)
{
	const u8 *buf = buffer;
	size_t i, j;
	u32 value;

	for (j = 0; j < size; j += 4) {
		value = 0;

		for (i = 0; i < 4 && j + i < size; i++)
			value |= buf[j + i] << (i << 3);

		writel(value, &misc->dsi_wr_data);
	}
}

static ssize_t tegra_dsi_host_transfer(struct mipi_dsi_host *host,
				       const struct mipi_dsi_msg *msg)
{
	struct udevice *dev = (struct udevice *)host->dev;
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct dsi_misc_reg *misc = &priv->dsi->misc;
	struct mipi_dsi_packet packet;
	const u8 *header;
	size_t count;
	ssize_t err;
	u32 value;

	err = mipi_dsi_create_packet(&packet, msg);
	if (err < 0)
		return err;

	header = packet.header;

	/* maximum FIFO depth is 1920 words */
	if (packet.size > priv->video_fifo_depth * 4)
		return -ENOSPC;

	/* reset underflow/overflow flags */
	value = readl(&misc->dsi_status);
	if (value & (DSI_STATUS_UNDERFLOW | DSI_STATUS_OVERFLOW)) {
		value = DSI_HOST_CONTROL_FIFO_RESET;
		writel(value, &misc->host_dsi_ctrl);
		udelay(10);
	}

	value = readl(&misc->dsi_pwr_ctrl);
	value |= DSI_POWER_CONTROL_ENABLE;
	writel(value, &misc->dsi_pwr_ctrl);

	mdelay(5);

	value = DSI_HOST_CONTROL_CRC_RESET | DSI_HOST_CONTROL_TX_TRIG_HOST |
		DSI_HOST_CONTROL_CS | DSI_HOST_CONTROL_ECC;

	if ((msg->flags & MIPI_DSI_MSG_USE_LPM) == 0)
		value |= DSI_HOST_CONTROL_HS;

	/*
	 * The host FIFO has a maximum of 64 words, so larger transmissions
	 * need to use the video FIFO.
	 */
	if (packet.size > priv->host_fifo_depth * 4)
		value |= DSI_HOST_CONTROL_FIFO_SEL;

	writel(value, &misc->host_dsi_ctrl);

	/*
	 * For reads and messages with explicitly requested ACK, generate a
	 * BTA sequence after the transmission of the packet.
	 */
	if ((msg->flags & MIPI_DSI_MSG_REQ_ACK) ||
	    (msg->rx_buf && msg->rx_len > 0)) {
		value = readl(&misc->host_dsi_ctrl);
		value |= DSI_HOST_CONTROL_PKT_BTA;
		writel(value, &misc->host_dsi_ctrl);
	}

	value = DSI_CONTROL_LANES(0) | DSI_CONTROL_HOST_ENABLE;
	writel(value, &misc->dsi_ctrl);

	/* write packet header, ECC is generated by hardware */
	value = header[2] << 16 | header[1] << 8 | header[0];
	writel(value, &misc->dsi_wr_data);

	/* write payload (if any) */
	if (packet.payload_length > 0)
		tegra_dsi_writesl(misc, packet.payload,
				  packet.payload_length);

	err = tegra_dsi_transmit(misc, 250);
	if (err < 0)
		return err;

	if ((msg->flags & MIPI_DSI_MSG_REQ_ACK) ||
	    (msg->rx_buf && msg->rx_len > 0)) {
		err = tegra_dsi_wait_for_response(misc, 250);
		if (err < 0)
			return err;

		count = err;

		value = readl(&misc->dsi_rd_data);
		switch (value) {
		case 0x84:
			debug("%s: ACK\n", __func__);
			break;

		case 0x87:
			debug("%s: ESCAPE\n", __func__);
			break;

		default:
			printf("%s: unknown status: %08x\n", __func__, value);
			break;
		}

		if (count > 1) {
			err = tegra_dsi_read_response(misc, msg, count);
			if (err < 0) {
				printf("%s: failed to parse response: %zd\n",
				       __func__, err);
			} else {
				/*
				 * For read commands, return the number of
				 * bytes returned by the peripheral.
				 */
				count = err;
			}
		}
	} else {
		/*
		 * For write commands, we have transmitted the 4-byte header
		 * plus the variable-length payload.
		 */
		count = 4 + packet.payload_length;
	}

	return count;
}

struct mipi_dsi_host_ops tegra_dsi_bridge_host_ops = {
	.transfer	= tegra_dsi_host_transfer,
};

#define PKT_ID0(id)	((((id) & 0x3f) <<  3) | (1 <<  9))
#define PKT_LEN0(len)	(((len) & 0x07) <<  0)
#define PKT_ID1(id)	((((id) & 0x3f) << 13) | (1 << 19))
#define PKT_LEN1(len)	(((len) & 0x07) << 10)
#define PKT_ID2(id)	((((id) & 0x3f) << 23) | (1 << 29))
#define PKT_LEN2(len)	(((len) & 0x07) << 20)

#define PKT_LP		BIT(30)
#define NUM_PKT_SEQ	12

/*
 * non-burst mode with sync pulses
 */
static const u32 pkt_seq_video_non_burst_sync_pulses[NUM_PKT_SEQ] = {
	[ 0] = PKT_ID0(MIPI_DSI_V_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(1) |
	       PKT_ID2(MIPI_DSI_H_SYNC_END) | PKT_LEN2(0) |
	       PKT_LP,
	[ 1] = 0,
	[ 2] = PKT_ID0(MIPI_DSI_V_SYNC_END) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(1) |
	       PKT_ID2(MIPI_DSI_H_SYNC_END) | PKT_LEN2(0) |
	       PKT_LP,
	[ 3] = 0,
	[ 4] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(1) |
	       PKT_ID2(MIPI_DSI_H_SYNC_END) | PKT_LEN2(0) |
	       PKT_LP,
	[ 5] = 0,
	[ 6] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(1) |
	       PKT_ID2(MIPI_DSI_H_SYNC_END) | PKT_LEN2(0),
	[ 7] = PKT_ID0(MIPI_DSI_BLANKING_PACKET) | PKT_LEN0(2) |
	       PKT_ID1(MIPI_DSI_PACKED_PIXEL_STREAM_24) | PKT_LEN1(3) |
	       PKT_ID2(MIPI_DSI_BLANKING_PACKET) | PKT_LEN2(4),
	[ 8] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(1) |
	       PKT_ID2(MIPI_DSI_H_SYNC_END) | PKT_LEN2(0) |
	       PKT_LP,
	[ 9] = 0,
	[10] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(1) |
	       PKT_ID2(MIPI_DSI_H_SYNC_END) | PKT_LEN2(0),
	[11] = PKT_ID0(MIPI_DSI_BLANKING_PACKET) | PKT_LEN0(2) |
	       PKT_ID1(MIPI_DSI_PACKED_PIXEL_STREAM_24) | PKT_LEN1(3) |
	       PKT_ID2(MIPI_DSI_BLANKING_PACKET) | PKT_LEN2(4),
};

/*
 * non-burst mode with sync events
 */
static const u32 pkt_seq_video_non_burst_sync_events[NUM_PKT_SEQ] = {
	[ 0] = PKT_ID0(MIPI_DSI_V_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_END_OF_TRANSMISSION) | PKT_LEN1(7) |
	       PKT_LP,
	[ 1] = 0,
	[ 2] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_END_OF_TRANSMISSION) | PKT_LEN1(7) |
	       PKT_LP,
	[ 3] = 0,
	[ 4] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_END_OF_TRANSMISSION) | PKT_LEN1(7) |
	       PKT_LP,
	[ 5] = 0,
	[ 6] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(2) |
	       PKT_ID2(MIPI_DSI_PACKED_PIXEL_STREAM_24) | PKT_LEN2(3),
	[ 7] = PKT_ID0(MIPI_DSI_BLANKING_PACKET) | PKT_LEN0(4),
	[ 8] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_END_OF_TRANSMISSION) | PKT_LEN1(7) |
	       PKT_LP,
	[ 9] = 0,
	[10] = PKT_ID0(MIPI_DSI_H_SYNC_START) | PKT_LEN0(0) |
	       PKT_ID1(MIPI_DSI_BLANKING_PACKET) | PKT_LEN1(2) |
	       PKT_ID2(MIPI_DSI_PACKED_PIXEL_STREAM_24) | PKT_LEN2(3),
	[11] = PKT_ID0(MIPI_DSI_BLANKING_PACKET) | PKT_LEN0(4),
};

static const u32 pkt_seq_command_mode[NUM_PKT_SEQ] = {
	[ 0] = 0,
	[ 1] = 0,
	[ 2] = 0,
	[ 3] = 0,
	[ 4] = 0,
	[ 5] = 0,
	[ 6] = PKT_ID0(MIPI_DSI_DCS_LONG_WRITE) | PKT_LEN0(3) | PKT_LP,
	[ 7] = 0,
	[ 8] = 0,
	[ 9] = 0,
	[10] = PKT_ID0(MIPI_DSI_DCS_LONG_WRITE) | PKT_LEN0(5) | PKT_LP,
	[11] = 0,
};

static void tegra_dsi_get_muldiv(enum mipi_dsi_pixel_format format,
				 unsigned int *mulp, unsigned int *divp)
{
	switch (format) {
	case MIPI_DSI_FMT_RGB666_PACKED:
	case MIPI_DSI_FMT_RGB888:
		*mulp = 3;
		*divp = 1;
		break;

	case MIPI_DSI_FMT_RGB565:
		*mulp = 2;
		*divp = 1;
		break;

	case MIPI_DSI_FMT_RGB666:
		*mulp = 9;
		*divp = 4;
		break;

	default:
		break;
	}
}

static int tegra_dsi_get_format(enum mipi_dsi_pixel_format format,
				enum tegra_dsi_format *fmt)
{
	switch (format) {
	case MIPI_DSI_FMT_RGB888:
		*fmt = TEGRA_DSI_FORMAT_24P;
		break;

	case MIPI_DSI_FMT_RGB666:
		*fmt = TEGRA_DSI_FORMAT_18NP;
		break;

	case MIPI_DSI_FMT_RGB666_PACKED:
		*fmt = TEGRA_DSI_FORMAT_18P;
		break;

	case MIPI_DSI_FMT_RGB565:
		*fmt = TEGRA_DSI_FORMAT_16P;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static void tegra_dsi_pad_calibrate(struct dsi_pad_ctrl_reg *pad)
{
	u32 value;

	/* start calibration */
	value = DSI_PAD_CONTROL_PAD_LPUPADJ(0x1) |
		DSI_PAD_CONTROL_PAD_LPDNADJ(0x1) |
		DSI_PAD_CONTROL_PAD_PREEMP_EN(0x1) |
		DSI_PAD_CONTROL_PAD_SLEWDNADJ(0x6) |
		DSI_PAD_CONTROL_PAD_SLEWUPADJ(0x6) |
		DSI_PAD_CONTROL_PAD_PDIO(0) |
		DSI_PAD_CONTROL_PAD_PDIO_CLK(0) |
		DSI_PAD_CONTROL_PAD_PULLDN_ENAB(0);
	writel(value, &pad->pad_ctrl);

	clock_enable(PERIPH_ID_VI);
	clock_enable(PERIPH_ID_CSI);
	udelay(2);
	reset_set_enable(PERIPH_ID_VI, 0);
	reset_set_enable(PERIPH_ID_CSI, 0);

	value = MIPI_CAL_TERMOSA(0x4);
	writel(value, TEGRA_VI_BASE + (CSI_CILA_MIPI_CAL_CONFIG_0 << 2));

	value = MIPI_CAL_TERMOSB(0x4);
	writel(value, TEGRA_VI_BASE + (CSI_CILB_MIPI_CAL_CONFIG_0 << 2));

	value = MIPI_CAL_HSPUOSD(0x3) | MIPI_CAL_HSPDOSD(0x4);
	writel(value, TEGRA_VI_BASE + (CSI_DSI_MIPI_CAL_CONFIG << 2));

	value = PAD_DRIV_DN_REF(0x5) | PAD_DRIV_UP_REF(0x7);
	writel(value, TEGRA_VI_BASE + (CSI_MIPIBIAS_PAD_CONFIG << 2));

	value = PAD_CIL_PDVREG(0x0);
	writel(value, TEGRA_VI_BASE + (CSI_CIL_PAD_CONFIG << 2));
}

static void tegra_dsi_mipi_calibrate(struct udevice *dev)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct dsi_pad_ctrl_reg *pad = &priv->dsi->pad;
	u32 value;
	int ret;

	ret = misc_set_enabled(priv->mipi, true);
	if (ret)
		log_debug("%s: failed to enable MIPI calibration: %d\n",
			  __func__, ret);

	writel(0, &pad->pad_ctrl);
	writel(0, &pad->pad_ctrl_1);
	writel(0, &pad->pad_ctrl_2);
	writel(0, &pad->pad_ctrl_3);
	writel(0, &pad->pad_ctrl_4);

	/* DSI pad enable */
	value = DSI_PAD_CONTROL_VS1_PULLDN(0) | DSI_PAD_CONTROL_VS1_PDIO(0);
	writel(value, &pad->pad_ctrl);

	value = DSI_PAD_SLEW_UP(0x7) | DSI_PAD_SLEW_DN(0x7) |
		DSI_PAD_LP_UP(0x1) | DSI_PAD_LP_DN(0x1) |
		DSI_PAD_OUT_CLK(0x0);
	writel(value, &pad->pad_ctrl_2);

	value = DSI_PAD_PREEMP_PD_CLK(0x3) | DSI_PAD_PREEMP_PU_CLK(0x3) |
		DSI_PAD_PREEMP_PD(0x03) | DSI_PAD_PREEMP_PU(0x3);
	writel(value, &pad->pad_ctrl_3);

	ret = misc_write(priv->mipi, priv->calibration_pads, NULL, 0);
	if (ret)
		log_debug("%s: MIPI calibration failed %d\n", __func__, ret);

	if (priv->slave)
		tegra_dsi_mipi_calibrate(priv->slave);
}

static void tegra_dsi_set_timeout(struct udevice *dev,
				  unsigned long bclk,
				  unsigned int vrefresh)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct dsi_timeout_reg *rtimeout = &priv->dsi->timeout;
	unsigned int timeout;
	u32 value;

	/* one frame high-speed transmission timeout */
	timeout = (bclk / vrefresh) / 512;
	value = DSI_TIMEOUT_LRX(0x2000) | DSI_TIMEOUT_HTX(timeout);
	writel(value, &rtimeout->dsi_timeout_0);

	/* 2 ms peripheral timeout for panel */
	timeout = 2 * bclk / 512 * 1000;
	value = DSI_TIMEOUT_PR(timeout) | DSI_TIMEOUT_TA(0x2000);
	writel(value, &rtimeout->dsi_timeout_1);

	value = DSI_TALLY_TA(0) | DSI_TALLY_LRX(0) | DSI_TALLY_HTX(0);
	writel(value, &rtimeout->dsi_to_tally);

	if (priv->slave)
		tegra_dsi_set_timeout(priv->slave, bclk, vrefresh);
}

static void tegra_dsi_set_phy_timing(struct udevice *dev,
				     unsigned long period,
				     const struct mipi_dphy_timing *dphy_timing)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct dsi_timing_reg *ptiming = &priv->dsi->ptiming;
	u32 value;

	value = DSI_TIMING_FIELD(dphy_timing->hsexit, period, 1) << 24 |
		DSI_TIMING_FIELD(dphy_timing->hstrail, period, 0) << 16 |
		DSI_TIMING_FIELD(dphy_timing->hszero, period, 3) << 8 |
		DSI_TIMING_FIELD(dphy_timing->hsprepare, period, 1);
	writel(value, &ptiming->dsi_phy_timing_0);

	value = DSI_TIMING_FIELD(dphy_timing->clktrail, period, 1) << 24 |
		DSI_TIMING_FIELD(dphy_timing->clkpost, period, 1) << 16 |
		DSI_TIMING_FIELD(dphy_timing->clkzero, period, 1) << 8 |
		DSI_TIMING_FIELD(dphy_timing->lpx, period, 1);
	writel(value, &ptiming->dsi_phy_timing_1);

	value = DSI_TIMING_FIELD(dphy_timing->clkprepare, period, 1) << 16 |
		DSI_TIMING_FIELD(dphy_timing->clkpre, period, 1) << 8 |
		DSI_TIMING_FIELD(0xff * period, period, 0) << 0;
	writel(value, &ptiming->dsi_phy_timing_2);

	value = DSI_TIMING_FIELD(dphy_timing->taget, period, 1) << 16 |
		DSI_TIMING_FIELD(dphy_timing->tasure, period, 1) << 8 |
		DSI_TIMING_FIELD(dphy_timing->tago, period, 1);
	writel(value, &ptiming->dsi_bta_timing);

	if (priv->slave)
		tegra_dsi_set_phy_timing(priv->slave, period, dphy_timing);
}

static u32 tegra_dsi_get_lanes(struct udevice *dev)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;

	if (priv->master) {
		struct tegra_dsi_priv *mpriv = dev_get_priv(priv->master);
		struct mipi_dsi_device *mdevice = &mpriv->device;

		return mdevice->lanes + device->lanes;
	}

	if (priv->slave) {
		struct tegra_dsi_priv *spriv = dev_get_priv(priv->slave);
		struct mipi_dsi_device *sdevice = &spriv->device;

		return device->lanes + sdevice->lanes;
	}

	return device->lanes;
}

static void tegra_dsi_ganged_enable(struct udevice *dev, unsigned int start,
				    unsigned int size)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct dsi_ganged_mode_reg *ganged = &priv->dsi->ganged;

	writel(start, &ganged->ganged_mode_start);
	writel(size << 16 | size, &ganged->ganged_mode_size);
	writel(DSI_GANGED_MODE_CONTROL_ENABLE, &ganged->ganged_mode_ctrl);
}

static void tegra_dsi_configure(struct udevice *dev, unsigned int pipe,
				unsigned long mode_flags)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct display_timing *timing = &priv->timing;

	struct dsi_misc_reg *misc = &priv->dsi->misc;
	struct dsi_pkt_seq_reg *pkt = &priv->dsi->pkt;
	struct dsi_pkt_len_reg *len = &priv->dsi->len;

	unsigned int hact, hsw, hbp, hfp, i, mul, div;
	const u32 *pkt_seq;
	u32 value;

	tegra_dsi_get_muldiv(device->format, &mul, &div);

	if (mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) {
		printf("[DSI] Non-burst video mode with sync pulses\n");
		pkt_seq = pkt_seq_video_non_burst_sync_pulses;
	} else if (mode_flags & MIPI_DSI_MODE_VIDEO) {
		printf("[DSI] Non-burst video mode with sync events\n");
		pkt_seq = pkt_seq_video_non_burst_sync_events;
	} else {
		printf("[DSI] Command mode\n");
		pkt_seq = pkt_seq_command_mode;
	}

	value = DSI_CONTROL_CHANNEL(0) |
		DSI_CONTROL_FORMAT(priv->format) |
		DSI_CONTROL_LANES(device->lanes - 1) |
		DSI_CONTROL_SOURCE(pipe);
	writel(value, &misc->dsi_ctrl);

	writel(priv->video_fifo_depth, &misc->dsi_max_threshold);

	value = DSI_HOST_CONTROL_HS;
	writel(value, &misc->host_dsi_ctrl);

	value = readl(&misc->dsi_ctrl);

	if (mode_flags & MIPI_DSI_CLOCK_NON_CONTINUOUS)
		value |= DSI_CONTROL_HS_CLK_CTRL;

	value &= ~DSI_CONTROL_TX_TRIG(3);

	/* enable DCS commands for command mode */
	if (mode_flags & MIPI_DSI_MODE_VIDEO)
		value &= ~DSI_CONTROL_DCS_ENABLE;
	else
		value |= DSI_CONTROL_DCS_ENABLE;

	value |= DSI_CONTROL_VIDEO_ENABLE;
	value &= ~DSI_CONTROL_HOST_ENABLE;
	writel(value, &misc->dsi_ctrl);

	for (i = 0; i < NUM_PKT_SEQ; i++)
		writel(pkt_seq[i], &pkt->dsi_pkt_seq_0_lo + i);

	if (mode_flags & MIPI_DSI_MODE_VIDEO) {
		/* horizontal active pixels */
		hact = timing->hactive.typ * mul / div;

		/* horizontal sync width */
		hsw = timing->hsync_len.typ * mul / div;

		/* horizontal back porch */
		hbp = timing->hback_porch.typ * mul / div;

		/* horizontal front porch */
		hfp = timing->hfront_porch.typ * mul / div;

		if (priv->master || priv->slave) {
			hact /= 2;
			hsw /= 2;
			hbp = hbp / 2 - 1;
			hfp /= 2;
		}

		if ((mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) == 0)
			hbp += hsw;

		/* subtract packet overhead */
		hsw -= 10;
		hbp -= 14;
		hfp -= 8;

		writel(hsw << 16 | 0, &len->dsi_pkt_len_0_1);
		writel(hact << 16 | hbp, &len->dsi_pkt_len_2_3);
		writel(hfp, &len->dsi_pkt_len_4_5);
		writel(0x0f0f << 16, &len->dsi_pkt_len_6_7);
	} else {
		if (priv->master || priv->slave) {
			/*
			 * For ganged mode, assume symmetric left-right mode.
			 */
			value = 1 + (timing->hactive.typ / 2) * mul / div;
		} else {
			/* 1 byte (DCS command) + pixel data */
			value = 1 + timing->hactive.typ * mul / div;
		}

		writel(0, &len->dsi_pkt_len_0_1);
		writel(value << 16, &len->dsi_pkt_len_2_3);
		writel(value << 16, &len->dsi_pkt_len_4_5);
		writel(0, &len->dsi_pkt_len_6_7);

		value = MIPI_DCS_WRITE_MEMORY_START << 8 |
			MIPI_DCS_WRITE_MEMORY_CONTINUE;
		writel(value, &len->dsi_dcs_cmds);
	}

	/* set SOL delay */
	if (priv->master || priv->slave) {
		unsigned long delay, bclk, bclk_ganged;
		unsigned int lanes = tegra_dsi_get_lanes(dev);
		unsigned long htotal = timing->hactive.typ + timing->hfront_porch.typ +
				       timing->hback_porch.typ + timing->hsync_len.typ;

		/* SOL to valid, valid to FIFO and FIFO write delay */
		delay = 4 + 4 + 2;
		delay = DIV_ROUND_UP(delay * mul, div * lanes);
		/* FIFO read delay */
		delay = delay + 6;

		bclk = DIV_ROUND_UP(htotal * mul, div * lanes);
		bclk_ganged = DIV_ROUND_UP(bclk * lanes / 2, lanes);
		value = bclk - bclk_ganged + delay + 20;
	} else {
		/* set SOL delay (for non-burst mode only) */
		value = 8 * mul / div;
	}

	writel(value, &misc->dsi_sol_delay);

	if (priv->slave) {
		tegra_dsi_configure(priv->slave, pipe, mode_flags);
		/*
		 * TODO: Support modes other than symmetrical left-right
		 * split.
		 */
		tegra_dsi_ganged_enable(dev, 0, timing->hactive.typ / 2);
		tegra_dsi_ganged_enable(priv->slave, timing->hactive.typ / 2,
					timing->hactive.typ / 2);
	}
}

static void tegra_dsi_enable(struct udevice *dev)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct dsi_misc_reg *misc = &priv->dsi->misc;
	u32 value;

	/* enable DSI controller */
	value = readl(&misc->dsi_pwr_ctrl);
	value |= DSI_POWER_CONTROL_ENABLE;
	writel(value, &misc->dsi_pwr_ctrl);

	if (priv->slave)
		tegra_dsi_enable(priv->slave);
}

static int tegra_dsi_encoder_enable(struct udevice *dev)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct tegra_dc_plat *dc_plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct display_timing *timing = &priv->timing;
	struct dsi_misc_reg *misc = &priv->dsi->misc;
	unsigned int mul, div;
	unsigned long bclk, plld, period;
	u32 value, lanes;
	int ret;

	/* If for some reasone DSI is enabled then it needs to
	 * be disabled in order for the panel initialization
	 * commands to be properly sent.
	 */
	value = readl(&misc->dsi_pwr_ctrl);

	if (value & DSI_POWER_CONTROL_ENABLE) {
		value = readl(&misc->dsi_pwr_ctrl);
		value &= ~DSI_POWER_CONTROL_ENABLE;
		writel(value, &misc->dsi_pwr_ctrl);
	}

	/* Disable interrupt */
	writel(0, &misc->int_enable);

	if (priv->version)
		tegra_dsi_mipi_calibrate(dev);
	else
		tegra_dsi_pad_calibrate(&priv->dsi->pad);

	tegra_dsi_get_muldiv(device->format, &mul, &div);

	/* compute byte clock */
	lanes = tegra_dsi_get_lanes(dev);
	bclk = (timing->pixelclock.typ * mul) / (div * lanes);

	tegra_dsi_set_timeout(dev, bclk, 60);

	/*
	 * Compute bit clock and round up to the next MHz.
	 */
	plld = DIV_ROUND_UP(bclk * 8, USEC_PER_SEC) * USEC_PER_SEC;
	period = DIV_ROUND_CLOSEST(NSEC_PER_SEC, plld);

	ret = mipi_dphy_timing_get_default(&priv->dphy_timing, period);
	if (ret < 0) {
		printf("%s: failed to get D-PHY timing: %d\n", __func__, ret);
		return ret;
	}

	ret = mipi_dphy_timing_validate(&priv->dphy_timing, period);
	if (ret < 0) {
		printf("%s: failed to validate D-PHY timing: %d\n", __func__, ret);
		return ret;
	}

	/*
	 * The D-PHY timing fields are expressed in byte-clock cycles, so
	 * multiply the period by 8.
	 */
	tegra_dsi_set_phy_timing(dev, period * 8, &priv->dphy_timing);

	/* Perform panel HW setup */
	ret = panel_enable_backlight(priv->panel);
	if (ret)
		return ret;

	tegra_dsi_configure(dev, dc_plat->pipe, device->mode_flags);

	tegra_dc_enable_controller(dev);

	tegra_dsi_enable(dev);

	return 0;
}

static int tegra_dsi_bridge_set_panel(struct udevice *dev, int percent)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);

	/* Turn on/off backlight */
	return panel_set_backlight(priv->panel, percent);
}

static int tegra_dsi_panel_timings(struct udevice *dev,
				   struct display_timing *timing)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

static void tegra_dsi_init_clocks(struct udevice *dev)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct tegra_dc_plat *dc_plat = dev_get_plat(dev);
	struct mipi_dsi_device *device = &priv->device;
	unsigned int mul, div, lanes;
	unsigned long bclk, plld;

	/* Switch parents of DSI clocks in case of not standard parent */
	if (priv->clk->id == PERIPH_ID_DSI &&
	    priv->clk_parent->id == CLOCK_ID_DISPLAY2) {
		/* Change DSIA clock parent to PLLD2 */
		struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;

		/* DSIA_CLK_SRC */
		setbits_le32(&clkrst->crc_pll[CLOCK_ID_DISPLAY].pll_base,
			     BIT(25));
	}

	if (priv->clk->id == PERIPH_ID_DSIB &&
	    priv->clk_parent->id == CLOCK_ID_DISPLAY) {
		/* Change DSIB clock parent to match DSIA */
		struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;

		/* DSIB_CLK_SRC */
		clrbits_le32(&clkrst->plld2.pll_base, BIT(25));
	}

	tegra_dsi_get_muldiv(device->format, &mul, &div);

	lanes = tegra_dsi_get_lanes(dev);
	bclk = (priv->timing.pixelclock.typ * mul) / (div * lanes);

	plld = DIV_ROUND_UP(bclk * 8, USEC_PER_SEC);

	dc_plat->scdiv = ((plld * USEC_PER_SEC +
		priv->timing.pixelclock.typ / 2) /
		priv->timing.pixelclock.typ) - 2;

	/*
	 * BUG: If DISP1 is a PLLD/D2 child, it cannot go over 370MHz. The
	 * cause of this is not quite clear. This can be overcomed by
	 * halving the PLLD/D2 if the target rate is > 800MHz. This way
	 * DISP1 and DSI clocks will be equal.
	 */
	if (plld > 800)
		plld /= 2;

	switch (clock_get_osc_freq()) {
	case CLOCK_OSC_FREQ_12_0: /* OSC is 12Mhz */
	case CLOCK_OSC_FREQ_48_0: /* OSC is 48Mhz */
		clock_set_rate(priv->clk_parent->id, plld, 12, 0, 8);
		break;

	case CLOCK_OSC_FREQ_26_0: /* OSC is 26Mhz */
		clock_set_rate(priv->clk_parent->id, plld, 26, 0, 8);
		break;

	case CLOCK_OSC_FREQ_13_0: /* OSC is 13Mhz */
	case CLOCK_OSC_FREQ_16_8: /* OSC is 16.8Mhz */
		clock_set_rate(priv->clk_parent->id, plld, 13, 0, 8);
		break;

	case CLOCK_OSC_FREQ_19_2:
	case CLOCK_OSC_FREQ_38_4:
	default:
		/*
		 * These are not supported.
		 */
		break;
	}

	clk_enable(priv->clk);
}

static int tegra_dsi_ganged_probe(struct udevice *dev)
{
	struct tegra_dsi_priv *mpriv = dev_get_priv(dev);
	struct udevice *gangster;

	uclass_get_device_by_phandle(UCLASS_VIDEO_BRIDGE, dev,
				     "nvidia,ganged-mode", &gangster);
	if (gangster) {
		/* Ganged mode is set */
		struct tegra_dsi_priv *spriv = dev_get_priv(gangster);

		mpriv->slave = gangster;
		spriv->master = dev;
	}

	return 0;
}

static int tegra_dsi_bridge_probe(struct udevice *dev)
{
	struct tegra_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct mipi_dsi_panel_plat *mipi_plat;
	struct reset_ctl reset_ctl;
	int ret;

	priv->version = dev_get_driver_data(dev);

	priv->dsi = (struct dsi_ctlr *)dev_read_addr_ptr(dev);
	if (!priv->dsi) {
		printf("%s: No display controller address\n", __func__);
		return -EINVAL;
	}

	priv->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clk)) {
		log_debug("%s: Could not get DSI clock: %ld\n",
			  __func__, PTR_ERR(priv->clk));
		return PTR_ERR(priv->clk);
	}

	priv->clk_parent = devm_clk_get(dev, "parent");
	if (IS_ERR(priv->clk_parent)) {
		log_debug("%s: Could not get DSI clock parent: %ld\n",
			  __func__, PTR_ERR(priv->clk_parent));
		return PTR_ERR(priv->clk_parent);
	}

	priv->video_fifo_depth = 1920;
	priv->host_fifo_depth = 64;

	tegra_dsi_ganged_probe(dev);

	ret = reset_get_by_name(dev, "dsi", &reset_ctl);
	if (ret) {
		log_debug("%s: reset_get_by_name() failed: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "avdd-dsi-csi-supply", &priv->avdd);
	if (ret)
		debug("%s: Cannot get avdd-dsi-csi-supply: error %d\n",
		      __func__, ret);

	/* Check all DSI children */
	device_foreach_child(priv->panel, dev) {
		if (device_get_uclass_id(priv->panel) == UCLASS_PANEL)
			break;
	}

	/* if loop exits without panel device return error */
	if (device_get_uclass_id(priv->panel) != UCLASS_PANEL) {
		log_debug("%s: panel not found, ret %d\n", __func__, ret);
		return -EINVAL;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PANEL, dev_ofnode(priv->panel),
					  &priv->panel);
	if (ret) {
		log_debug("%s: Cannot get panel: error %d\n", __func__, ret);
		return log_ret(ret);
	}

	if (priv->version) {
		ret = uclass_get_device_by_phandle(UCLASS_MISC, dev,
						   "nvidia,mipi-calibrate",
						   &priv->mipi);
		if (ret) {
			log_debug("%s: cannot get MIPI: error %d\n", __func__, ret);
			return ret;
		}

		ret = dev_read_u32_index(dev, "nvidia,mipi-calibrate", 1,
					 &priv->calibration_pads);
		if (ret) {
			log_debug("%s: cannot get calibration pads: error %d\n",
				  __func__, ret);
			return ret;
		}
	}

	panel_get_display_timing(priv->panel, &priv->timing);

	mipi_plat = dev_get_plat(priv->panel);
	mipi_plat->device = device;

	priv->host.dev = (struct device *)dev;
	priv->host.ops = &tegra_dsi_bridge_host_ops;

	device->host = &priv->host;
	device->lanes = mipi_plat->lanes;
	device->format = mipi_plat->format;
	device->mode_flags = mipi_plat->mode_flags;

	tegra_dsi_get_format(device->format, &priv->format);

	reset_assert(&reset_ctl);

	ret = regulator_set_enable_if_allowed(priv->avdd, true);
	if (ret && ret != -ENOSYS)
		return ret;

	tegra_dsi_init_clocks(dev);

	mdelay(2);
	reset_deassert(&reset_ctl);

	return 0;
}

static const struct video_bridge_ops tegra_dsi_bridge_ops = {
	.attach			= tegra_dsi_encoder_enable,
	.set_backlight		= tegra_dsi_bridge_set_panel,
	.get_display_timing	= tegra_dsi_panel_timings,
};

static const struct udevice_id tegra_dsi_bridge_ids[] = {
	{ .compatible = "nvidia,tegra20-dsi", .data = DSI_V0 },
	{ .compatible = "nvidia,tegra30-dsi", .data = DSI_V0 },
	{ .compatible = "nvidia,tegra114-dsi", .data = DSI_V1 },
	{ .compatible = "nvidia,tegra124-dsi", .data = DSI_V1 },
	{ }
};

U_BOOT_DRIVER(tegra_dsi) = {
	.name		= "tegra_dsi",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= tegra_dsi_bridge_ids,
	.ops		= &tegra_dsi_bridge_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= tegra_dsi_bridge_probe,
	.plat_auto	= sizeof(struct tegra_dc_plat),
	.priv_auto	= sizeof(struct tegra_dsi_priv),
};
