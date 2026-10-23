// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek HDMI TX v2 support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <asm/io.h>
#include <asm/unaligned.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/ofnode.h>
#include <edid.h>
#include <errno.h>
#include <fdtdec.h>
#include <generic-phy.h>
#include <i2c.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <mapmem.h>
#include <time.h>
#include <video.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_merge.h"
#include "mtk_disp_mutex.h"
#include "mtk_disp_padding.h"
#include "mtk_dpi.h"
#include "mtk_ethdr.h"
#include "mtk_hdmi_regs_v2.h"
#include "mtk_mdp_rdma.h"

/*
 * MT8188 vdosys1 registers: the display routing (SEL_IN / SOUT mux) and the
 * sizes of the asynchronous clock-domain wrappers between the pipeline
 * blocks. Both the offsets and the mux values below describe the MT8188
 * vdosys1 routing; another SoC would need its own set.
 */
#define SUBDISP_MIXER_IN3_PAD			0xD48
#define MIXER_IN_PAD_MODE_BYPASS			0x0
#define MIXER_IN_PAD_BI_WIDTH				GENMASK(31, 16)
#define SUBDISP_MERGE2_ASYNC_CFG_WD		0xE50
#define SUBDISP_HDRBE_ASYNC_CFG_WD		0xE70
#define SUBDISP_DISP_DPI1_SEL_IN		0xF10
#define SUBDISP_DISP_MERGE4_MOUT_SEL		0xF18
#define SUBDISP_MIXER_IN3_SEL_IN		0xF2C
#define SUBDISP_MIXER_OUT_SOUT_SEL		0xF34
#define SUBDISP_MERGE2_ASYNC_SOUT_SEL		0xF48
#define SUBDISP_MERGE4_ASYNC_SEL_IN		0xF50
#define SUBDISP_MIXER_IN3_SOUT_SEL		0xF60
#define SUBDISP_MIXER_SOUT_SEL_IN		0xF68

#define OUTPUT_TO_MIXER_IN3_SEL			0x1
#define INPUT_FROM_MERGE2_ASYNC_SOUT		0x1
#define OUTPUT_TO_DISP_MIXER			0x0
#define INPUT_FROM_DISP_MIXER			0x0
#define OUTPUT_TO_HDR_VDO_BE0			0x0
#define INPUT_FROM_HDR_VDO_BE0			0x0
#define OUTPUT_TO_DPI1_SEL			0x4
#define INPUT_FROM_VPP_MERGE4_MOUT		0x0

#define MTK_HDMI_MAX_WIDTH			3840
#define MTK_HDMI_MAX_HEIGHT			2160

/* the TMDS character rates the PHY can lock to, see mtk_hdmi_pll_calc() */
#define MTK_HDMI_MIN_PIXEL_CLOCK		25000000
#define MTK_HDMI_MAX_PIXEL_CLOCK		594000000

#define MTK_HDMI_HPD_TIMEOUT_MS			100

/* 4 byte header + 13 byte payload */
#define HDMI_AVI_INFOFRAME_SIZE			17
/* 4 byte header + 25 byte payload */
#define HDMI_SPD_INFOFRAME_SIZE			29

/*
 * HDMI 2.0 requires TMDS clock scrambling and a 1/40 bit clock ratio for
 * TMDS character rates above 340 MHz.
 */
#define HDMI_TMDS_CLOCK_SCRAMBLE_HZ		340000000

/* SCDC (Status and Control Data Channel), HDMI 2.0 section 10.4 */
#define SCDC_I2C_ADDR				0x54
#define SCDC_TMDS_CONFIG			0x20
#define SCDC_SCRAMBLING_ENABLE			BIT(0)
#define SCDC_TMDS_BIT_CLOCK_RATIO_BY_40		BIT(1)

/**
 * struct mtk_hdmi_vic - CTA-861 Video Identification Code of a timing
 * @hactive: horizontal active pixels
 * @vactive: vertical active lines
 * @refresh: refresh rate, in Hz
 * @vic: Video Identification Code
 * @aspect_4_3: 4:3 picture aspect ratio, 16:9 otherwise
 *
 * Video Identification Codes (CTA-861) for the timings this driver can
 * describe in the AVI InfoFrame. Anything else is reported as VIC 0 with no
 * picture aspect ratio rather than as a wrong mode. @refresh is compared
 * against the rounded refresh rate, which lands exactly on these values for
 * every one of the timings below, the 1000/1001 variants included.
 *
 * 720x480p60 and 720x576p50 exist both as 4:3 (VIC 2 / 17) and 16:9
 * (VIC 3 / 18) with identical timings, so they cannot be told apart from the
 * timing alone: 4:3 is assumed.
 */
struct mtk_hdmi_vic {
	u16 hactive;
	u16 vactive;
	u8 refresh;
	u8 vic;
	bool aspect_4_3;
};

static const struct mtk_hdmi_vic mtk_hdmi_vics[] = {
	{  640,  480, 60,  1, true  },
	{  720,  480, 60,  2, true  },
	{  720,  576, 50, 17, true  },
	{ 1280,  720, 50, 19, false },
	{ 1280,  720, 60,  4, false },
	{ 1920, 1080, 50, 31, false },
	{ 1920, 1080, 60, 16, false },
	{ 3840, 2160, 24, 93, false },
	{ 3840, 2160, 25, 94, false },
	{ 3840, 2160, 30, 95, false },
	{ 3840, 2160, 50, 96, false },
	{ 3840, 2160, 60, 97, false },
};

/**
 * struct mtk_hdmi - HDMI transmitter state
 * @dev: HDMI device
 * @ddc_bus: DDC I2C bus, for the EDID and SCDC
 * @dpi1: DPI feeding the transmitter
 * @merge3: merge of the two RDMA halves
 * @merge5: merge buffering the HDR back-end output
 * @ethdr: mixer and HDR video back-end
 * @mutex: vdosys1 mutex
 * @padding4: padding behind @rdma4
 * @padding5: padding behind @rdma5
 * @rdma4: RDMA reading the left half of the framebuffer
 * @rdma5: RDMA reading the right half of the framebuffer
 * @phy: HDMI PHY
 * @regs: transmitter register window
 * @clk_bulk: transmitter clocks
 */
struct mtk_hdmi {
	struct udevice *dev;
	struct udevice *ddc_bus;
	struct udevice *dpi1;
	struct udevice *merge3;
	struct udevice *merge5;
	struct udevice *ethdr;
	struct udevice *mutex;
	struct udevice *padding4;
	struct udevice *padding5;
	struct udevice *rdma4;
	struct udevice *rdma5;
	struct phy phy;
	void __iomem *regs;
	struct clk_bulk clk_bulk;
};

static u32 mtk_hdmi_read(struct mtk_hdmi *hdmi, u32 offset)
{
	return readl(hdmi->regs + offset);
}

static void mtk_hdmi_write(struct mtk_hdmi *hdmi, u32 offset, u32 val)
{
	writel(val, hdmi->regs + offset);
}

static void mtk_hdmi_update(struct mtk_hdmi *hdmi, u32 offset, u32 val, u32 mask)
{
	void __iomem *reg = hdmi->regs + offset;
	u32 tmp;

	tmp = readl(reg);
	tmp = (tmp & ~mask) | (val & mask);
	writel(tmp, reg);
}

static void mtk_hdmi_enable_hdmi_mode(struct mtk_hdmi *hdmi, bool enable)
{
	u32 value = enable ? HDMI_MODE_HDMI : 0;

	mtk_hdmi_update(hdmi, TOP_CFG00, value, HDMI_MODE_HDMI);
}

static void mtk_hdmi_hw_vid_black(struct mtk_hdmi *hdmi, bool black)
{
	u32 value = black ? REG_VMUTE_EN : 0;

	mtk_hdmi_update(hdmi, TOP_VMUTE_CFG1, value, REG_VMUTE_EN);
}

/*
 * Clear the General Control Packet AVMUTE, in case an earlier boot stage
 * left the sink muted.
 */
static void mtk_hdmi_hw_gcp_avmute(struct mtk_hdmi *hdmi)
{
	mtk_hdmi_update(hdmi, TOP_CFG01, CP_CLR_MUTE_EN,
			CP_CLR_MUTE_EN | CP_SET_MUTE_EN);
	mtk_hdmi_update(hdmi, TOP_INFO_RPT, CP_RPT_EN, CP_RPT_EN);
	mtk_hdmi_update(hdmi, TOP_INFO_EN, CP_EN | CP_EN_WR, CP_EN | CP_EN_WR);
}

static void mtk_hdmi_hw_reset(struct mtk_hdmi *hdmi)
{
	mtk_hdmi_update(hdmi, HDMITX_CONFIG_MT8188, 0, HDMITX_SW_RSTB);
	udelay(5);
	mtk_hdmi_update(hdmi, HDMITX_CONFIG_MT8188, HDMITX_SW_RSTB, HDMITX_SW_RSTB);
}

static void mtk_hdmi_set_sw_hpd(struct mtk_hdmi *hdmi, bool high)
{
	u32 value = high ? HDMITX_SW_HPD : 0;

	mtk_hdmi_update(hdmi, HDMITX_CONFIG_MT8188, value, HDMITX_SW_HPD);
}

/*
 * The driver only ever outputs 8 bits per component, so force 8 bpp and
 * disable the deep-color GCP and packets (TMDS_PACK_MODE_8BPP is 0).
 */
static void mtk_hdmi_set_8bit_color(struct mtk_hdmi *hdmi)
{
	mtk_hdmi_update(hdmi, TOP_CFG00, 0, TMDS_PACK_MODE);
	mtk_hdmi_update(hdmi, TOP_CFG00, 0, DEEPCOLOR_PKT_EN);
	mtk_hdmi_update(hdmi, TOP_MISC_CTLR, 0, DEEP_COLOR_ADD);
}

/* the driver only ever outputs RGB444, so keep the downsampler bypassed */
static void mtk_hdmi_bypass_yuv420_downsample(struct mtk_hdmi *hdmi)
{
	mtk_hdmi_update(hdmi, HDMITX_CONFIG_MT8188, 0, HDMI_YUV420_MODE);
	mtk_hdmi_update(hdmi, VID_DOWNSAMPLE_CONFIG, 0, C444_C422_CONFIG_ENABLE);
	mtk_hdmi_update(hdmi, VID_DOWNSAMPLE_CONFIG, 0, C422_C420_CONFIG_ENABLE);
	mtk_hdmi_update(hdmi, VID_DOWNSAMPLE_CONFIG, C422_C420_CONFIG_BYPASS,
			C422_C420_CONFIG_BYPASS);
	mtk_hdmi_update(hdmi, VID_DOWNSAMPLE_CONFIG, 0,
			C422_C420_CONFIG_OUT_CB_OR_CR);
	mtk_hdmi_update(hdmi, VID_OUT_FORMAT, 0, OUTPUT_FORMAT_DEMUX_420_ENABLE);
}

static bool mtk_hdmi_tmds_over_340M(u32 pixelclock)
{
	/* RGB444 8 bpc only, so the TMDS character rate is the pixel clock */
	return pixelclock >= HDMI_TMDS_CLOCK_SCRAMBLE_HZ;
}

/*
 * Above 340 MHz the sink has to scramble and use the 1/40 bit clock ratio
 * too. A sink advertising such a mode in its EDID is required to implement
 * SCDC (HDMI 2.0 section 6.1.3.1), so write TMDS_Config without first
 * reading back the HF-VSDB capability bits.
 */
static int mtk_hdmi_scdc_enable_scrambling(struct mtk_hdmi *hdmi)
{
	u8 val = SCDC_SCRAMBLING_ENABLE | SCDC_TMDS_BIT_CLOCK_RATIO_BY_40;
	struct udevice *chip;
	int ret;

	ret = i2c_get_chip(hdmi->ddc_bus, SCDC_I2C_ADDR, 1, &chip);
	if (ret)
		return ret;

	return dm_i2c_write(chip, SCDC_TMDS_CONFIG, &val, 1);
}

static void mtk_hdmi_enable_scrambling(struct mtk_hdmi *hdmi, bool enable)
{
	u32 value = enable ? SCR_ON | HDMI2_ON : 0;
	int ret;

	mtk_hdmi_update(hdmi, TOP_CFG00, value, SCR_ON | HDMI2_ON);

	/*
	 * Only reach for SCDC when scrambling is required: a sink that never
	 * advertises a mode above 340 MHz does not have to implement SCDC at
	 * all and would simply not acknowledge the write.
	 */
	if (!enable)
		return;

	ret = mtk_hdmi_scdc_enable_scrambling(hdmi);
	if (ret)
		dev_warn(hdmi->dev, "failed to configure sink SCDC: %d\n", ret);
}

static void mtk_hdmi_disable_abist(struct mtk_hdmi *hdmi)
{
	mtk_hdmi_update(hdmi, TOP_CFG00, 0, HDMI_ABIST_ENABLE);
}

static void mtk_hdmi_change_video_resolution(struct mtk_hdmi *hdmi,
					     const struct display_timing *mode)
{
	bool is_over_340M;

	mtk_hdmi_hw_reset(hdmi);
	mtk_hdmi_set_sw_hpd(hdmi, true);
	udelay(5);

	mtk_hdmi_write(hdmi, HDCP_TOP_CTRL, 0x0);

	mtk_hdmi_set_8bit_color(hdmi);
	mtk_hdmi_enable_hdmi_mode(hdmi, mode->hdmi_monitor);

	udelay(10);
	mtk_hdmi_hw_vid_black(hdmi, true);

	/* data islands are an HDMI concept, a DVI sink gets none of them */
	if (mode->hdmi_monitor) {
		mtk_hdmi_hw_gcp_avmute(hdmi);
		mtk_hdmi_update(hdmi, TOP_CFG01, NULL_PKT_VSYNC_HIGH_EN,
				NULL_PKT_VSYNC_HIGH_EN | NULL_PKT_EN);
	}
	udelay(150);

	is_over_340M = mtk_hdmi_tmds_over_340M(mode->pixelclock.typ);
	dev_dbg(hdmi->dev, "is_over_340M: %d\n", is_over_340M);

	mtk_hdmi_enable_scrambling(hdmi, is_over_340M);

	mtk_hdmi_bypass_yuv420_downsample(hdmi);
}

static int mtk_hdmi_output_set_display_mode(struct mtk_hdmi *hdmi,
					    const struct display_timing *mode)
{
	unsigned long link_rate = mode->pixelclock.typ;
	int ret;

	ret = generic_phy_configure(&hdmi->phy, &link_rate);
	if (ret) {
		dev_err(hdmi->dev, "failed to set the PHY to %u Hz: %d\n",
			mode->pixelclock.typ, ret);
		return ret;
	}

	mtk_hdmi_change_video_resolution(hdmi, mode);

	return 0;
}

/*
 * Copy at most @dst_len bytes of @src into @dst and pad the remainder with
 * spaces. Unlike strlcpy() this does not NUL-terminate, so the full field
 * width can be used for the string.
 */
static void mtk_hdmi_pad_string(u8 *dst, size_t dst_len, const char *src)
{
	size_t src_len = strlen(src);

	if (src_len > dst_len)
		src_len = dst_len;

	memcpy(dst, src, src_len);
	memset(dst + src_len, ' ', dst_len - src_len);
}

static void mtk_hdmi_setup_spd_infoframe(u8 *buffer, const char *vendor,
					 const char *product)
{
	u8 checksum;
	int i;

	memset(buffer, 0, HDMI_SPD_INFOFRAME_SIZE);

	/* SPD InfoFrame header */
	buffer[0] = 0x83;  /* SPD InfoFrame type */
	buffer[1] = 0x01;  /* Version */
	buffer[2] = 0x19;  /* Length (25 bytes) */

	/* Vendor name (8 bytes) and product description (16 bytes) */
	mtk_hdmi_pad_string(&buffer[4], 8, vendor);
	mtk_hdmi_pad_string(&buffer[12], 16, product);

	/* checksum over the full frame, so that the total sums to zero */
	for (checksum = 0, i = 0; i < HDMI_SPD_INFOFRAME_SIZE; i++)
		checksum += buffer[i];
	buffer[3] = 0x100 - checksum;
}

/* the refresh rate of @timing, in Hz */
static unsigned int mtk_hdmi_refresh_rate(const struct display_timing *timing)
{
	unsigned int htotal, vtotal;

	htotal = timing->hactive.typ + timing->hfront_porch.typ +
		 timing->hback_porch.typ + timing->hsync_len.typ;
	vtotal = timing->vactive.typ + timing->vfront_porch.typ +
		 timing->vback_porch.typ + timing->vsync_len.typ;

	if (!htotal || !vtotal)
		return 0;

	return DIV_ROUND_CLOSEST(timing->pixelclock.typ, htotal * vtotal);
}

static const struct mtk_hdmi_vic *
mtk_hdmi_get_vic(const struct display_timing *timing)
{
	unsigned int refresh = mtk_hdmi_refresh_rate(timing);
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(mtk_hdmi_vics); i++) {
		const struct mtk_hdmi_vic *vic = &mtk_hdmi_vics[i];

		if (vic->hactive == timing->hactive.typ &&
		    vic->vactive == timing->vactive.typ &&
		    vic->refresh == refresh)
			return vic;
	}

	return NULL;
}

static void mtk_hdmi_setup_avi_infoframe(u8 *buffer,
					 const struct display_timing *mode)
{
	const struct mtk_hdmi_vic *vic;
	u8 checksum;
	int i;

	memset(buffer, 0, HDMI_AVI_INFOFRAME_SIZE);

	/* AVI InfoFrame header */
	buffer[0] = 0x82;  /* AVI InfoFrame type */
	buffer[1] = 0x02;  /* Version */
	buffer[2] = 0x0D;  /* Length (13 bytes) */

	/*
	 * Data byte 1: RGB output, no scan or bar information, and A0 set to
	 * mark the active format information of data byte 2 valid.
	 */
	buffer[4] = 0x10;

	/* Data byte 4: Video Identification Code (VIC) */
	vic = mtk_hdmi_get_vic(mode);
	buffer[7] = vic ? vic->vic : 0;

	/*
	 * Data byte 2: active portion same as picture, the picture aspect
	 * ratio of the VIC, and colorimetry "No Data" since the output is RGB
	 * (CTA-861 only defines the ITU-601/709 selection for YCbCr). An
	 * unknown timing gets no aspect ratio rather than a wrong one.
	 */
	if (!vic)
		buffer[5] = 0x08;
	else
		buffer[5] = vic->aspect_4_3 ? 0x18 : 0x28;

	/* Data byte 3: no content type, default range, no picture scaling */
	buffer[6] = 0x00;

	/* Data byte 5: Pixel repetition */
	buffer[8] = 0x00;  /* No pixel repetition */

	/* checksum over the full frame, so that the total sums to zero */
	for (checksum = 0, i = 0; i < HDMI_AVI_INFOFRAME_SIZE; i++)
		checksum += buffer[i];
	buffer[3] = 0x100 - checksum;
}

static void mtk_hdmi_hw_avi_infoframe(struct mtk_hdmi *hdmi, const u8 *buf)
{
	/* Disable AVI InfoFrame first */
	mtk_hdmi_update(hdmi, TOP_INFO_EN, 0, AVI_EN_WR | AVI_EN);
	mtk_hdmi_update(hdmi, TOP_INFO_RPT, 0, AVI_RPT_EN);

	/* Write AVI InfoFrame header */
	mtk_hdmi_write(hdmi, TOP_AVI_HEADER, get_unaligned_le24(&buf[0]));

	/* Write AVI InfoFrame data packets */
	mtk_hdmi_write(hdmi, TOP_AVI_PKT00, get_unaligned_le32(&buf[3]));
	mtk_hdmi_write(hdmi, TOP_AVI_PKT01, get_unaligned_le24(&buf[7]));
	mtk_hdmi_write(hdmi, TOP_AVI_PKT02, get_unaligned_le32(&buf[10]));
	mtk_hdmi_write(hdmi, TOP_AVI_PKT03, get_unaligned_le24(&buf[14]));

	/* Clear remaining packets */
	mtk_hdmi_write(hdmi, TOP_AVI_PKT04, 0);
	mtk_hdmi_write(hdmi, TOP_AVI_PKT05, 0);

	/* Enable AVI InfoFrame */
	mtk_hdmi_update(hdmi, TOP_INFO_RPT, AVI_RPT_EN, AVI_RPT_EN);
	mtk_hdmi_update(hdmi, TOP_INFO_EN, AVI_EN_WR | AVI_EN,
			AVI_EN_WR | AVI_EN);
}

static void mtk_hdmi_hw_spd_infoframe(struct mtk_hdmi *hdmi, const u8 *buf)
{
	/* Disable SPD InfoFrame first */
	mtk_hdmi_update(hdmi, TOP_INFO_EN, 0, SPD_EN_WR | SPD_EN);
	mtk_hdmi_update(hdmi, TOP_INFO_RPT, 0, SPD_RPT_EN);

	/* Write SPD InfoFrame header */
	mtk_hdmi_write(hdmi, TOP_SPDIF_HEADER, get_unaligned_le24(&buf[0]));

	/* Write SPD InfoFrame data packets */
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT00, get_unaligned_le32(&buf[3]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT01, get_unaligned_le24(&buf[7]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT02, get_unaligned_le32(&buf[10]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT03, get_unaligned_le24(&buf[14]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT04, get_unaligned_le32(&buf[17]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT05, get_unaligned_le24(&buf[21]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT06, get_unaligned_le32(&buf[24]));
	mtk_hdmi_write(hdmi, TOP_SPDIF_PKT07, buf[28]);

	/* Enable SPD InfoFrame */
	mtk_hdmi_update(hdmi, TOP_INFO_RPT, SPD_RPT_EN, SPD_RPT_EN);
	mtk_hdmi_update(hdmi, TOP_INFO_EN, SPD_EN_WR | SPD_EN,
			SPD_EN_WR | SPD_EN);
}

static void mtk_hdmi_setup_infoframes(struct mtk_hdmi *hdmi,
				      const struct display_timing *mode)
{
	u8 buffer_spd[HDMI_SPD_INFOFRAME_SIZE];
	u8 buffer_avi[HDMI_AVI_INFOFRAME_SIZE];

	mtk_hdmi_setup_avi_infoframe(buffer_avi, mode);
	mtk_hdmi_setup_spd_infoframe(buffer_spd, "MediaTek", "On-chip HDMI");

	/* Send the infoframes to hardware */
	mtk_hdmi_hw_avi_infoframe(hdmi, buffer_avi);
	mtk_hdmi_hw_spd_infoframe(hdmi, buffer_spd);
}

static int mtk_hdmi_controller_pre_enable(struct mtk_hdmi *hdmi,
					  const struct display_timing *mode)
{
	int ret;

	ret = mtk_hdmi_output_set_display_mode(hdmi, mode);
	if (ret)
		return ret;

	/* InfoFrames are an HDMI concept, a DVI sink must not be sent any */
	if (mode->hdmi_monitor)
		mtk_hdmi_setup_infoframes(hdmi, mode);

	return 0;
}

static int mtk_hdmi_controller_enable(struct mtk_hdmi *hdmi)
{
	int ret;

	ret = generic_phy_power_on(&hdmi->phy);
	if (ret) {
		dev_err(hdmi->dev, "failed to power on the PHY: %d\n", ret);
		return ret;
	}

	mtk_hdmi_hw_vid_black(hdmi, false);

	return 0;
}

/*
 * Mask every transmitter interrupt. Linux keeps
 * HDCP2X_RX_REAUTH_REQ_DDCM_INT enabled (the hardware uses it internally for
 * the HPD state machine) and drives HDCP2X_CTRL_0's software HPD override,
 * but nothing here re-runs HPD detection once the output is set up.
 */
static void mtk_hdmi_disable_all_int(struct mtk_hdmi *hdmi)
{
	/* disable all tx irq */
	mtk_hdmi_write(hdmi, TOP_INT_ENABLE00, 0);
	mtk_hdmi_write(hdmi, TOP_INT_ENABLE01, 0);
}

static int mtk_hdmi_read_edid(struct mtk_hdmi *hdmi, u8 *buf, int buf_size)
{
	const struct edid1_info *edid = (const struct edid1_info *)buf;
	struct udevice *chip;
	int ret;

	if (!hdmi->ddc_bus || buf_size < EDID_SIZE)
		return -EINVAL;

	ret = i2c_get_chip(hdmi->ddc_bus, EDID_ADDR, 1, &chip);
	if (ret)
		return ret;

	ret = dm_i2c_read(chip, 0, buf, EDID_SIZE);
	if (ret) {
		dev_err(hdmi->dev, "failed to read EDID: %d\n", ret);
		return ret;
	}

	/* read the extension block, if any */
	if (edid->extension_flag && buf_size >= EDID_EXT_SIZE) {
		ret = dm_i2c_read(chip, EDID_SIZE, buf + EDID_SIZE,
				  buf_size - EDID_SIZE);
		if (ret) {
			dev_warn(hdmi->dev,
				 "error reading extended EDID block: %d\n",
				 ret);
			/*
			 * The parser looks at the extension block whenever
			 * the base block announces one, so make sure it
			 * never sees the leftovers of a failed read.
			 */
			memset(buf + EDID_SIZE, 0, buf_size - EDID_SIZE);
		}
	}

	return 0;
}

/* true once a sink is plugged in and powered on */
static bool mtk_hdmi_hpd_asserted(struct mtk_hdmi *hdmi)
{
	u32 mask = HPD_PIN_STA | PORD_PIN_STA;

	return (mtk_hdmi_read(hdmi, HPD_DDC_STATUS) & mask) == mask;
}

static int mtk_hdmi_wait_for_hpd(struct mtk_hdmi *hdmi)
{
	ulong start = get_timer(0);

	do {
		if (mtk_hdmi_hpd_asserted(hdmi))
			return 0;

		udelay(100);
	} while (get_timer(start) < MTK_HDMI_HPD_TIMEOUT_MS);

	return -ETIMEDOUT;
}

/*
 * The pipeline reads the frame as two halves and feeds the DPI two pixels per
 * clock, so it only handles progressive timings of an even width. Rejecting
 * everything else here lets the EDID parser fall back to the next detailed
 * timing instead of scanning out a broken one.
 */
static bool mtk_hdmi_mode_valid(void *priv, const struct display_timing *timing)
{
	if (timing->flags & DISPLAY_FLAGS_INTERLACED)
		return false;

	if (timing->hactive.typ > MTK_HDMI_MAX_WIDTH ||
	    timing->vactive.typ > MTK_HDMI_MAX_HEIGHT ||
	    timing->hactive.typ % 2)
		return false;

	return timing->pixelclock.typ >= MTK_HDMI_MIN_PIXEL_CLOCK &&
	       timing->pixelclock.typ <= MTK_HDMI_MAX_PIXEL_CLOCK;
}

static int mtk_hdmi_get_display_timing(struct mtk_hdmi *hdmi,
				       struct display_timing *timing)
{
	u8 edid[EDID_EXT_SIZE];
	int panel_bits_per_colour;
	int ret;

	ret = mtk_hdmi_read_edid(hdmi, edid, EDID_EXT_SIZE);
	if (ret)
		return ret;

	ret = edid_get_timing_validate(edid, EDID_EXT_SIZE,
				       timing, &panel_bits_per_colour,
				       mtk_hdmi_mode_valid, NULL);
	if (ret)
		return ret;

	debug("Display timing:\n clock %u Hz\n", timing->pixelclock.typ);
	debug(" hactive: %d,\thfront_p: %d,\thback_p: %d hsync:\t%d\n",
	      timing->hactive.typ, timing->hfront_porch.typ,
	      timing->hback_porch.typ, timing->hsync_len.typ);
	debug(" vactive: %d,\tvfront_p: %d,\tvback_p: %d vsync:\t%d\n",
	      timing->vactive.typ, timing->vfront_porch.typ,
	      timing->vback_porch.typ, timing->vsync_len.typ);
	debug(" flags: 0x%x\n", timing->flags);

	return 0;
}

static int mtk_hdmi_controller_initialize(struct mtk_hdmi *hdmi,
					  const struct display_timing *mode)
{
	int ret;

	mtk_hdmi_disable_all_int(hdmi);

	mtk_hdmi_disable_abist(hdmi);

	ret = mtk_hdmi_controller_pre_enable(hdmi, mode);
	if (ret)
		return ret;

	/* let the new configuration settle before unmuting the output */
	mdelay(50);

	return mtk_hdmi_controller_enable(hdmi);
}

static int mtk_hdmi_get_comp_by_alias(struct udevice *dev, const char *alias,
				      struct udevice **compp)
{
	struct udevice *comp;
	ofnode node;
	int ret;

	node = ofnode_get_aliases_node(alias);
	if (!ofnode_valid(node)) {
		dev_err(dev, "cannot find alias %s\n", alias);
		return -ENODEV;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, node, &comp);
	if (ret) {
		dev_err(dev, "cannot get %s: %d\n", alias, ret);
		return ret;
	}

	ret = mtk_disp_comp_enable(comp);
	if (ret)
		return ret;

	/*
	 * Only publish the handle once its clocks are on, so the error
	 * unwind can rely on a non-NULL pointer meaning "enabled".
	 */
	*compp = comp;

	return 0;
}

/* disable the pipeline component clocks, in reverse acquisition order */
static void mtk_hdmi_disable_components(struct mtk_hdmi *hdmi)
{
	struct udevice *comps[] = {
		hdmi->padding5, hdmi->padding4, hdmi->mutex, hdmi->ethdr,
		hdmi->merge5, hdmi->merge3, hdmi->rdma5, hdmi->rdma4,
		hdmi->dpi1,
	};
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(comps); i++)
		if (comps[i])
			mtk_disp_comp_disable(comps[i]);
}

/*
 * Program the vdosys1 muxes to route the mixer output to the DPI1/HDMI path,
 * and size the asynchronous wrappers along it. The mixer and the merges run
 * in a two-pixel-per-clock domain, hence the half widths.
 *
 * Unlike the Linux MT8188 routing table, which muxes the mixer output
 * straight to the merge4 async wrapper (MIXER_OUT_SOUT_SEL and
 * MERGE4_ASYNC_SEL_IN set to 1), the HDR video back-end is deliberately kept
 * in the path, in bypass (see mtk_ethdr_config()).
 *
 * vdosys1 has no power domain of its own in the devicetree: this relies on
 * the VDOSYS1 domain having already been powered on by the probe of a
 * pipeline component, such as dpi1.
 */
static void mtk_hdmi_configure_vdosys1(void __iomem *base, u16 width,
				       u16 height)
{
	/* MERGE2_ASYNC_SOUT --> MIXER_IN3_SEL */
	writel(OUTPUT_TO_MIXER_IN3_SEL, base + SUBDISP_MERGE2_ASYNC_SOUT_SEL);
	writel(INPUT_FROM_MERGE2_ASYNC_SOUT, base + SUBDISP_MIXER_IN3_SEL_IN);

	/* MIXER_IN3_SOUT --> DISP_MIXER */
	writel(OUTPUT_TO_DISP_MIXER, base + SUBDISP_MIXER_IN3_SOUT_SEL);

	/* DISP_MIXER --> MIXER_SOUT_SEL */
	writel(INPUT_FROM_DISP_MIXER, base + SUBDISP_MIXER_SOUT_SEL_IN);

	/* MIXER_SOUT_SEL --> HDR_VDO_BE0 */
	writel(OUTPUT_TO_HDR_VDO_BE0, base + SUBDISP_MIXER_OUT_SOUT_SEL);

	/* HDR_VDO_BE0 --> MERGE4_ASYNC_SEL */
	writel(INPUT_FROM_HDR_VDO_BE0, base + SUBDISP_MERGE4_ASYNC_SEL_IN);

	/* MERGE4_MOUT_SEL --> DPI1_SEL */
	writel(OUTPUT_TO_DPI1_SEL, base + SUBDISP_DISP_MERGE4_MOUT_SEL);
	writel(INPUT_FROM_VPP_MERGE4_MOUT, base + SUBDISP_DISP_DPI1_SEL_IN);

	/* merge3 --> mixer_in3 and mixer_sout --> merge5 async wrapper sizes */
	writel(height << 16 | width / 2, base + SUBDISP_MERGE2_ASYNC_CFG_WD);
	writel(height << 16 | width / 2, base + SUBDISP_HDRBE_ASYNC_CFG_WD);

	/* mixer input 3: bypass mode, no channel swap (bit 4 left clear) */
	writel(FIELD_PREP(MIXER_IN_PAD_BI_WIDTH, width / 2 - 1) |
	       MIXER_IN_PAD_MODE_BYPASS, base + SUBDISP_MIXER_IN3_PAD);
}

static int mtk_hdmi_probe(struct udevice *dev)
{
	struct mtk_hdmi *hdmi = dev_get_priv(dev);
	struct video_priv *priv = dev_get_uclass_priv(dev);
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct display_timing timing;
	struct udevice *vdosys1;
	u32 fb_size;
	void __iomem *vdosys1_base;
	ofnode node;
	int ret;

	hdmi->dev = dev;

	hdmi->regs = dev_remap_addr(dev);
	if (!hdmi->regs)
		return -EINVAL;

	ret = clk_get_bulk(dev, &hdmi->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to get clocks: %d\n", ret);
		return ret;
	}

	ret = clk_enable_bulk(&hdmi->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to enable clocks: %d\n", ret);
		return ret;
	}

	ret = generic_phy_get_by_name(dev, "hdmi", &hdmi->phy);
	if (ret) {
		dev_err(dev, "failed to get HDMI PHY: %d\n", ret);
		goto err_disable_clk;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C,
					  dev_read_subnode(dev, "i2c"),
					  &hdmi->ddc_bus);
	if (ret) {
		dev_err(dev, "failed to get DDC I2C bus: %d\n", ret);
		goto err_disable_clk;
	}

	ret = mtk_hdmi_wait_for_hpd(hdmi);
	if (ret) {
		/*
		 * Booting without a display attached is perfectly normal, so
		 * keep this quiet even though the probe has to fail: there is
		 * nothing for the video uclass to drive.
		 */
		dev_dbg(dev, "display is not connected\n");
		goto err_disable_clk;
	}

	ret = mtk_hdmi_get_display_timing(hdmi, &timing);
	if (ret) {
		dev_err(dev, "failed to get display timing from EDID: %d\n",
			ret);
		goto err_disable_clk;
	}

	/* this powers on the HDMI PHY */
	ret = mtk_hdmi_controller_initialize(hdmi, &timing);
	if (ret)
		goto err_disable_clk;

	/* Set up video private data based on the selected display timing */
	priv->bpix = VIDEO_BPP32;
	priv->format = VIDEO_X8R8G8B8;
	priv->xsize = timing.hactive.typ;
	priv->ysize = timing.vactive.typ;
	priv->line_length = priv->xsize * VNBYTES(VIDEO_BPP32);

	/*
	 * The framebuffer is only cleared by video_post_probe(), after this
	 * function has already started scanning it out: zero the part about
	 * to be displayed first, so that whatever the memory holds is never
	 * shown.
	 */
	fb_size = priv->ysize * priv->line_length;
	memset(map_sysmem(plat->base, fb_size), 0, fb_size);
	flush_dcache_range(plat->base, plat->base + fb_size);

	ret = mtk_hdmi_get_comp_by_alias(dev, "dpi1", &hdmi->dpi1);
	if (ret)
		goto err_disable_comps;

	mtk_dpi_config(hdmi->dpi1, &timing);

	ret = mtk_hdmi_get_comp_by_alias(dev, "vdo1-rdma4", &hdmi->rdma4);
	if (ret)
		goto err_disable_comps;

	ret = mtk_mdp_rdma_config(hdmi->rdma4, plat, &timing,
				  priv->line_length, false);
	if (ret)
		goto err_disable_comps;

	ret = mtk_hdmi_get_comp_by_alias(dev, "vdo1-rdma5", &hdmi->rdma5);
	if (ret)
		goto err_disable_comps;

	ret = mtk_mdp_rdma_config(hdmi->rdma5, plat, &timing,
				  priv->line_length, true);
	if (ret)
		goto err_disable_comps;

	/*
	 * The display routing muxes live in the vdosys1 syscon register block.
	 * It is a clock provider rather than a pipeline component, so resolve
	 * it by compatible and grab the register base directly.
	 */
	node = ofnode_by_compatible(ofnode_null(), "mediatek,mt8188-vdosys1");
	if (!ofnode_valid(node)) {
		dev_err(dev, "cannot find vdosys1 node\n");
		ret = -ENODEV;
		goto err_disable_comps;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_CLK, node, &vdosys1);
	if (ret) {
		dev_err(dev, "cannot get vdosys1 syscon: %d\n", ret);
		goto err_disable_comps;
	}

	vdosys1_base = dev_remap_addr(vdosys1);
	if (!vdosys1_base) {
		ret = -EINVAL;
		goto err_disable_comps;
	}

	mtk_hdmi_configure_vdosys1(vdosys1_base, priv->xsize, priv->ysize);

	ret = mtk_hdmi_get_comp_by_alias(dev, "merge3", &hdmi->merge3);
	if (ret)
		goto err_disable_comps;

	mtk_disp_merge_config(hdmi->merge3, priv->xsize / 2, priv->xsize / 2,
			      priv->ysize);

	ret = mtk_hdmi_get_comp_by_alias(dev, "merge5", &hdmi->merge5);
	if (ret)
		goto err_disable_comps;

	mtk_disp_merge_config(hdmi->merge5, priv->xsize, 0, priv->ysize);

	ret = mtk_hdmi_get_comp_by_alias(dev, "ethdr0", &hdmi->ethdr);
	if (ret)
		goto err_disable_comps;

	mtk_ethdr_config(hdmi->ethdr, priv->xsize, priv->ysize);

	ret = mtk_hdmi_get_comp_by_alias(dev, "mutex1", &hdmi->mutex);
	if (ret)
		goto err_disable_comps;

	mt8188_mutex_config_hdmi(hdmi->mutex);

	ret = mtk_hdmi_get_comp_by_alias(dev, "padding4", &hdmi->padding4);
	if (ret)
		goto err_disable_comps;

	mtk_disp_padding_config(hdmi->padding4);

	ret = mtk_hdmi_get_comp_by_alias(dev, "padding5", &hdmi->padding5);
	if (ret)
		goto err_disable_comps;

	mtk_disp_padding_config(hdmi->padding5);

	/* the timing generator goes last, once the pipeline feeding it is up */
	mtk_dpi_hw_enable(hdmi->dpi1);

	video_set_flush_dcache(dev, true);

	return 0;

err_disable_comps:
	if (hdmi->dpi1)
		mtk_dpi_hw_disable(hdmi->dpi1);
	mtk_hdmi_disable_components(hdmi);
	generic_phy_power_off(&hdmi->phy);
err_disable_clk:
	clk_disable_bulk(&hdmi->clk_bulk);

	return ret;
}

/*
 * The HDMI TX node has a single, SoC-specific compatible, and the vdosys1
 * routing programmed above is MT8188 specific as well.
 */
static const struct udevice_id mtk_hdmi_ids[] = {
	{
		.compatible = "mediatek,mt8188-hdmi-tx",
	},
	{ }
};

/*
 * The framebuffer has to be reserved before relocation, long before the EDID
 * says which mode the sink wants, so ask for the largest one this driver
 * accepts.
 */
static int mtk_hdmi_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = MTK_HDMI_MAX_WIDTH * MTK_HDMI_MAX_HEIGHT *
		     VNBYTES(VIDEO_BPP32);

	return dm_scan_fdt_dev(dev);
}

U_BOOT_DRIVER(mtk_hdmi) = {
	.name		= "mtk_hdmi",
	.id		= UCLASS_VIDEO,
	.of_match	= mtk_hdmi_ids,
	.probe		= mtk_hdmi_probe,
	.bind		= mtk_hdmi_bind,
	.priv_auto	= sizeof(struct mtk_hdmi),
	.flags		= DM_FLAG_PRE_RELOC,
};
