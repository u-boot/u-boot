// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 *
 * Author: Wyon Bi <bivvy.bi@rock-chips.com>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <div64.h>
#include <generic-phy.h>
#include <linux/kernel.h>
#include <linux/iopoll.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/math64.h>
#include <linux/time.h>
#include <phy-mipi-dphy.h>
#include <reset.h>

#define UPDATE(x, h, l)	(((x) << (l)) & GENMASK((h), (l)))

/*
 * The offset address[7:0] is distributed two parts, one from the bit7 to bit5
 * is the first address, the other from the bit4 to bit0 is the second address.
 * when you configure the registers, you must set both of them. The Clock Lane
 * and Data Lane use the same registers with the same second address, but the
 * first address is different.
 */
#define FIRST_ADDRESS(x)		(((x) & 0x7) << 5)
#define SECOND_ADDRESS(x)		(((x) & 0x1f) << 0)
#define PHY_REG(first, second)		(FIRST_ADDRESS(first) | \
					 SECOND_ADDRESS(second))

/* Analog Register Part: reg00 */
#define BANDGAP_POWER_MASK			BIT(7)
#define BANDGAP_POWER_DOWN			BIT(7)
#define BANDGAP_POWER_ON			0
#define LANE_EN_MASK				GENMASK(6, 2)
#define LANE_EN_CK				BIT(6)
#define LANE_EN_3				BIT(5)
#define LANE_EN_2				BIT(4)
#define LANE_EN_1				BIT(3)
#define LANE_EN_0				BIT(2)
#define POWER_WORK_MASK				GENMASK(1, 0)
#define POWER_WORK_ENABLE			UPDATE(1, 1, 0)
#define POWER_WORK_DISABLE			UPDATE(2, 1, 0)
/* Analog Register Part: reg01 */
#define REG_SYNCRST_MASK			BIT(2)
#define REG_SYNCRST_RESET			BIT(2)
#define REG_SYNCRST_NORMAL			0
#define REG_LDOPD_MASK				BIT(1)
#define REG_LDOPD_POWER_DOWN			BIT(1)
#define REG_LDOPD_POWER_ON			0
#define REG_PLLPD_MASK				BIT(0)
#define REG_PLLPD_POWER_DOWN			BIT(0)
#define REG_PLLPD_POWER_ON			0
/* Analog Register Part: reg03 */
#define REG_FBDIV_HI_MASK			BIT(5)
#define REG_FBDIV_HI(x)				UPDATE((x >> 8), 5, 5)
#define REG_PREDIV_MASK				GENMASK(4, 0)
#define REG_PREDIV(x)				UPDATE(x, 4, 0)
/* Analog Register Part: reg04 */
#define REG_FBDIV_LO_MASK			GENMASK(7, 0)
#define REG_FBDIV_LO(x)				UPDATE(x, 7, 0)
/* Analog Register Part: reg05 */
#define SAMPLE_CLOCK_PHASE_MASK			GENMASK(6, 4)
#define SAMPLE_CLOCK_PHASE(x)			UPDATE(x, 6, 4)
#define CLOCK_LANE_SKEW_PHASE_MASK		GENMASK(2, 0)
#define CLOCK_LANE_SKEW_PHASE(x)		UPDATE(x, 2, 0)
/* Analog Register Part: reg06 */
#define DATA_LANE_3_SKEW_PHASE_MASK		GENMASK(6, 4)
#define DATA_LANE_3_SKEW_PHASE(x)		UPDATE(x, 6, 4)
#define DATA_LANE_2_SKEW_PHASE_MASK		GENMASK(2, 0)
#define DATA_LANE_2_SKEW_PHASE(x)		UPDATE(x, 2, 0)
/* Analog Register Part: reg07 */
#define DATA_LANE_1_SKEW_PHASE_MASK		GENMASK(6, 4)
#define DATA_LANE_1_SKEW_PHASE(x)		UPDATE(x, 6, 4)
#define DATA_LANE_0_SKEW_PHASE_MASK		GENMASK(2, 0)
#define DATA_LANE_0_SKEW_PHASE(x)		UPDATE(x, 2, 0)
/* Analog Register Part: reg08 */
#define PLL_POST_DIV_ENABLE_MASK		BIT(5)
#define PLL_POST_DIV_ENABLE			BIT(5)
#define SAMPLE_CLOCK_DIRECTION_MASK		BIT(4)
#define SAMPLE_CLOCK_DIRECTION_REVERSE		BIT(4)
#define SAMPLE_CLOCK_DIRECTION_FORWARD		0
#define LOWFRE_EN_MASK				BIT(5)
#define PLL_OUTPUT_FREQUENCY_DIV_BY_1		0
#define PLL_OUTPUT_FREQUENCY_DIV_BY_2		1
/* Analog Register Part: reg0b */
#define CLOCK_LANE_VOD_RANGE_SET_MASK		GENMASK(3, 0)
#define CLOCK_LANE_VOD_RANGE_SET(x)		UPDATE(x, 3, 0)
#define VOD_MIN_RANGE				0x1
#define VOD_MID_RANGE				0x3
#define VOD_BIG_RANGE				0x7
#define VOD_MAX_RANGE				0xf
/* Analog Register Part: reg1E */
#define PLL_MODE_SEL_MASK			GENMASK(6, 5)
#define PLL_MODE_SEL_LVDS_MODE			0
#define PLL_MODE_SEL_MIPI_MODE			BIT(5)
/* Digital Register Part: reg00 */
#define REG_DIG_RSTN_MASK			BIT(0)
#define REG_DIG_RSTN_NORMAL			BIT(0)
#define REG_DIG_RSTN_RESET			0
/* Digital Register Part: reg01 */
#define INVERT_TXCLKESC_MASK			BIT(1)
#define INVERT_TXCLKESC_ENABLE			BIT(1)
#define INVERT_TXCLKESC_DISABLE			0
#define INVERT_TXBYTECLKHS_MASK			BIT(0)
#define INVERT_TXBYTECLKHS_ENABLE		BIT(0)
#define INVERT_TXBYTECLKHS_DISABLE		0
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg05 */
#define T_LPX_CNT_MASK				GENMASK(5, 0)
#define T_LPX_CNT(x)				UPDATE(x, 5, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg06 */
#define T_HS_ZERO_CNT_HI_MASK			BIT(7)
#define T_HS_ZERO_CNT_HI(x)			UPDATE(x, 7, 7)
#define T_HS_PREPARE_CNT_MASK			GENMASK(6, 0)
#define T_HS_PREPARE_CNT(x)			UPDATE(x, 6, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg07 */
#define T_HS_ZERO_CNT_LO_MASK			GENMASK(5, 0)
#define T_HS_ZERO_CNT_LO(x)			UPDATE(x, 5, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg08 */
#define T_HS_TRAIL_CNT_MASK			GENMASK(6, 0)
#define T_HS_TRAIL_CNT(x)			UPDATE(x, 6, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg09 */
#define T_HS_EXIT_CNT_LO_MASK			GENMASK(4, 0)
#define T_HS_EXIT_CNT_LO(x)			UPDATE(x, 4, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg0a */
#define T_CLK_POST_CNT_LO_MASK			GENMASK(3, 0)
#define T_CLK_POST_CNT_LO(x)			UPDATE(x, 3, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg0c */
#define LPDT_TX_PPI_SYNC_MASK			BIT(2)
#define LPDT_TX_PPI_SYNC_ENABLE			BIT(2)
#define LPDT_TX_PPI_SYNC_DISABLE		0
#define T_WAKEUP_CNT_HI_MASK			GENMASK(1, 0)
#define T_WAKEUP_CNT_HI(x)			UPDATE(x, 1, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg0d */
#define T_WAKEUP_CNT_LO_MASK			GENMASK(7, 0)
#define T_WAKEUP_CNT_LO(x)			UPDATE(x, 7, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg0e */
#define T_CLK_PRE_CNT_MASK			GENMASK(3, 0)
#define T_CLK_PRE_CNT(x)			UPDATE(x, 3, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg10 */
#define T_CLK_POST_CNT_HI_MASK			GENMASK(7, 6)
#define T_CLK_POST_CNT_HI(x)			UPDATE(x, 7, 6)
#define T_TA_GO_CNT_MASK			GENMASK(5, 0)
#define T_TA_GO_CNT(x)				UPDATE(x, 5, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg11 */
#define T_HS_EXIT_CNT_HI_MASK			BIT(6)
#define T_HS_EXIT_CNT_HI(x)			UPDATE(x, 6, 6)
#define T_TA_SURE_CNT_MASK			GENMASK(5, 0)
#define T_TA_SURE_CNT(x)			UPDATE(x, 5, 0)
/* Clock/Data0/Data1/Data2/Data3 Lane Register Part: reg12 */
#define T_TA_WAIT_CNT_MASK			GENMASK(5, 0)
#define T_TA_WAIT_CNT(x)			UPDATE(x, 5, 0)
/* LVDS Register Part: reg00 */
#define LVDS_DIGITAL_INTERNAL_RESET_MASK	BIT(2)
#define LVDS_DIGITAL_INTERNAL_RESET_DISABLE	BIT(2)
#define LVDS_DIGITAL_INTERNAL_RESET_ENABLE	0
/* LVDS Register Part: reg01 */
#define LVDS_DIGITAL_INTERNAL_ENABLE_MASK	BIT(7)
#define LVDS_DIGITAL_INTERNAL_ENABLE		BIT(7)
#define LVDS_DIGITAL_INTERNAL_DISABLE		0
/* LVDS Register Part: reg03 */
#define MODE_ENABLE_MASK			GENMASK(2, 0)
#define TTL_MODE_ENABLE				BIT(2)
#define LVDS_MODE_ENABLE			BIT(1)
#define MIPI_MODE_ENABLE			BIT(0)
/* LVDS Register Part: reg0b */
#define LVDS_LANE_EN_MASK			GENMASK(7, 3)
#define LVDS_DATA_LANE0_EN			BIT(7)
#define LVDS_DATA_LANE1_EN			BIT(6)
#define LVDS_DATA_LANE2_EN			BIT(5)
#define LVDS_DATA_LANE3_EN			BIT(4)
#define LVDS_CLK_LANE_EN			BIT(3)
#define LVDS_PLL_POWER_MASK			BIT(2)
#define LVDS_PLL_POWER_OFF			BIT(2)
#define LVDS_PLL_POWER_ON			0
#define LVDS_BANDGAP_POWER_MASK			BIT(0)
#define LVDS_BANDGAP_POWER_DOWN			BIT(0)
#define LVDS_BANDGAP_POWER_ON			0

#define DSI_PHY_RSTZ				0xa0
#define PHY_ENABLECLK				BIT(2)
#define DSI_PHY_STATUS				0xb0
#define PHY_LOCK				BIT(0)

#define msleep(a)				udelay(a * 1000)

enum phy_max_rate {
	MAX_1GHZ,
	MAX_2_5GHZ,
};

struct clk_hw {
	struct clk_core *core;
	struct clk *clk;
	const struct clk_init_data *init;
};

struct inno_video_phy_plat_data {
	const struct inno_mipi_dphy_timing *inno_mipi_dphy_timing_table;
	const unsigned int num_timings;
	enum phy_max_rate max_rate;
};

struct inno_dsidphy {
	struct udevice *dev;
	struct clk *ref_clk;
	struct clk *pclk_phy;
	struct clk *pclk_host;
	const struct inno_video_phy_plat_data *pdata;
	void __iomem *phy_base;
	void __iomem *host_base;
	struct reset_ctl *rst;
	struct phy_configure_opts_mipi_dphy dphy_cfg;

	struct clk *pll_clk;
	struct {
		struct clk_hw hw;
		u8 prediv;
		u16 fbdiv;
		unsigned long rate;
	} pll;
};

enum {
	REGISTER_PART_ANALOG,
	REGISTER_PART_DIGITAL,
	REGISTER_PART_CLOCK_LANE,
	REGISTER_PART_DATA0_LANE,
	REGISTER_PART_DATA1_LANE,
	REGISTER_PART_DATA2_LANE,
	REGISTER_PART_DATA3_LANE,
	REGISTER_PART_LVDS,
};

struct inno_mipi_dphy_timing {
	unsigned long rate;
	u8 lpx;
	u8 hs_prepare;
	u8 clk_lane_hs_zero;
	u8 data_lane_hs_zero;
	u8 hs_trail;
};

static const
struct inno_mipi_dphy_timing inno_mipi_dphy_timing_table_max_1ghz[] = {
	{ 110000000, 0x0, 0x20, 0x16, 0x02, 0x22},
	{ 150000000, 0x0, 0x06, 0x16, 0x03, 0x45},
	{ 200000000, 0x0, 0x18, 0x17, 0x04, 0x0b},
	{ 250000000, 0x0, 0x05, 0x17, 0x05, 0x16},
	{ 300000000, 0x0, 0x51, 0x18, 0x06, 0x2c},
	{ 400000000, 0x0, 0x64, 0x19, 0x07, 0x33},
	{ 500000000, 0x0, 0x20, 0x1b, 0x07, 0x4e},
	{ 600000000, 0x0, 0x6a, 0x1d, 0x08, 0x3a},
	{ 700000000, 0x0, 0x3e, 0x1e, 0x08, 0x6a},
	{ 800000000, 0x0, 0x21, 0x1f, 0x09, 0x29},
	{1000000000, 0x0, 0x09, 0x20, 0x09, 0x27},
};

static const
struct inno_mipi_dphy_timing inno_mipi_dphy_timing_table_max_2_5ghz[] = {
	{ 110000000, 0x02, 0x7f, 0x16, 0x02, 0x02},
	{ 150000000, 0x02, 0x7f, 0x16, 0x03, 0x02},
	{ 200000000, 0x02, 0x7f, 0x17, 0x04, 0x02},
	{ 250000000, 0x02, 0x7f, 0x17, 0x05, 0x04},
	{ 300000000, 0x02, 0x7f, 0x18, 0x06, 0x04},
	{ 400000000, 0x03, 0x7e, 0x19, 0x07, 0x04},
	{ 500000000, 0x03, 0x7c, 0x1b, 0x07, 0x08},
	{ 600000000, 0x03, 0x70, 0x1d, 0x08, 0x10},
	{ 700000000, 0x05, 0x40, 0x1e, 0x08, 0x30},
	{ 800000000, 0x05, 0x02, 0x1f, 0x09, 0x30},
	{1000000000, 0x05, 0x08, 0x20, 0x09, 0x30},
	{1200000000, 0x06, 0x03, 0x32, 0x14, 0x0f},
	{1400000000, 0x09, 0x03, 0x32, 0x14, 0x0f},
	{1600000000, 0x0d, 0x42, 0x36, 0x0e, 0x0f},
	{1800000000, 0x0e, 0x47, 0x7a, 0x0e, 0x0f},
	{2000000000, 0x11, 0x64, 0x7a, 0x0e, 0x0b},
	{2200000000, 0x13, 0x64, 0x7e, 0x15, 0x0b},
	{2400000000, 0x13, 0x33, 0x7f, 0x15, 0x6a},
	{2500000000, 0x15, 0x54, 0x7f, 0x15, 0x6a},
};

static void phy_update_bits(struct inno_dsidphy *inno,
			    u8 first, u8 second, u8 mask, u8 val)
{
	u32 reg = PHY_REG(first, second) << 2;
	unsigned int tmp, orig;

	orig = readl(inno->phy_base + reg);
	tmp = orig & ~mask;
	tmp |= val & mask;
	writel(tmp, inno->phy_base + reg);
}

static unsigned long inno_dsidphy_pll_calc_rate(struct inno_dsidphy *inno,
						unsigned long rate)
{
	unsigned long prate;
	unsigned long best_freq = 0;
	unsigned long fref, fout;
	u8 min_prediv, max_prediv;
	u8 _prediv, best_prediv = 1;
	u16 _fbdiv, best_fbdiv = 1;
	u32 min_delta = UINT_MAX;

	/*
	 * Upstream Linux tries to read the ref_clk, while the BSP
	 * U-Boot hard-codes this as 24MHz. Try the first, and if that
	 * fails do the second.
	 */
	prate = clk_get_rate(inno->ref_clk);
	if (!prate)
		prate = 24000000;

	/*
	 * The PLL output frequency can be calculated using a simple formula:
	 * PLL_Output_Frequency = (FREF / PREDIV * FBDIV) / 2
	 * PLL_Output_Frequency: it is equal to DDR-Clock-Frequency * 2
	 */
	fref = prate / 2;
	if (rate > 1000000000UL)
		fout = 1000000000UL;
	else
		fout = rate;

	/* 5Mhz < Fref / prediv < 40MHz */
	min_prediv = DIV_ROUND_UP(fref, 40000000);
	max_prediv = fref / 5000000;

	for (_prediv = min_prediv; _prediv <= max_prediv; _prediv++) {
		u64 tmp;
		u32 delta;

		tmp = (u64)fout * _prediv;
		do_div(tmp, fref);
		_fbdiv = tmp;

		/*
		 * The possible settings of feedback divider are
		 * 12, 13, 14, 16, ~ 511
		 */
		if (_fbdiv == 15)
			continue;

		if (_fbdiv < 12 || _fbdiv > 511)
			continue;

		tmp = (u64)_fbdiv * fref;
		do_div(tmp, _prediv);

		delta = abs(fout - tmp);
		if (!delta) {
			best_prediv = _prediv;
			best_fbdiv = _fbdiv;
			best_freq = tmp;
			break;
		} else if (delta < min_delta) {
			best_prediv = _prediv;
			best_fbdiv = _fbdiv;
			best_freq = tmp;
			min_delta = delta;
		}
	}

	if (best_freq) {
		inno->pll.prediv = best_prediv;
		inno->pll.fbdiv = best_fbdiv;
		inno->pll.rate = best_freq;
	}

	return best_freq;
}

static void inno_dsidphy_mipi_mode_enable(struct inno_dsidphy *inno)
{
	struct phy_configure_opts_mipi_dphy *cfg = &inno->dphy_cfg;
	const struct inno_mipi_dphy_timing *timings;
	u32 t_txbyteclkhs, t_txclkesc;
	u32 txbyteclkhs, txclkesc, esc_clk_div;
	u32 hs_exit, clk_post, clk_pre, wakeup, lpx, ta_go, ta_sure, ta_wait;
	u32 hs_prepare, hs_trail, hs_zero, clk_lane_hs_zero, data_lane_hs_zero;
	unsigned int i;

	timings = inno->pdata->inno_mipi_dphy_timing_table;

	inno_dsidphy_pll_calc_rate(inno, cfg->hs_clk_rate);

	/* Select MIPI mode */
	phy_update_bits(inno, REGISTER_PART_LVDS, 0x03,
			MODE_ENABLE_MASK, MIPI_MODE_ENABLE);
	/* Configure PLL */
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x03,
			REG_PREDIV_MASK, REG_PREDIV(inno->pll.prediv));
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x03,
			REG_FBDIV_HI_MASK, REG_FBDIV_HI(inno->pll.fbdiv));
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x04,
			REG_FBDIV_LO_MASK, REG_FBDIV_LO(inno->pll.fbdiv));
	if (inno->pdata->max_rate == MAX_2_5GHZ) {
		phy_update_bits(inno, REGISTER_PART_ANALOG, 0x08,
				PLL_POST_DIV_ENABLE_MASK, PLL_POST_DIV_ENABLE);
		phy_update_bits(inno, REGISTER_PART_ANALOG, 0x0b,
				CLOCK_LANE_VOD_RANGE_SET_MASK,
				CLOCK_LANE_VOD_RANGE_SET(VOD_MAX_RANGE));
	}
	/* Enable PLL and LDO */
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x01,
			REG_LDOPD_MASK | REG_PLLPD_MASK,
			REG_LDOPD_POWER_ON | REG_PLLPD_POWER_ON);
	/* Reset analog */
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x01,
			REG_SYNCRST_MASK, REG_SYNCRST_RESET);
	udelay(1);
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x01,
			REG_SYNCRST_MASK, REG_SYNCRST_NORMAL);
	/* Reset digital */
	phy_update_bits(inno, REGISTER_PART_DIGITAL, 0x00,
			REG_DIG_RSTN_MASK, REG_DIG_RSTN_RESET);
	udelay(1);
	phy_update_bits(inno, REGISTER_PART_DIGITAL, 0x00,
			REG_DIG_RSTN_MASK, REG_DIG_RSTN_NORMAL);

	txbyteclkhs = inno->pll.rate / 8;
	t_txbyteclkhs = div_u64(PSEC_PER_SEC, txbyteclkhs);

	esc_clk_div = DIV_ROUND_UP(txbyteclkhs, 20000000);
	txclkesc = txbyteclkhs / esc_clk_div;
	t_txclkesc = div_u64(PSEC_PER_SEC, txclkesc);

	/*
	 * The value of counter for HS Ths-exit
	 * Ths-exit = Tpin_txbyteclkhs * value
	 */
	hs_exit = DIV_ROUND_UP(cfg->hs_exit, t_txbyteclkhs);
	/*
	 * The value of counter for HS Tclk-post
	 * Tclk-post = Tpin_txbyteclkhs * value
	 */
	clk_post = DIV_ROUND_UP(cfg->clk_post, t_txbyteclkhs);
	/*
	 * The value of counter for HS Tclk-pre
	 * Tclk-pre = Tpin_txbyteclkhs * value
	 */
	clk_pre = DIV_ROUND_UP(cfg->clk_pre, BITS_PER_BYTE);

	/*
	 * The value of counter for HS Tta-go
	 * Tta-go for turnaround
	 * Tta-go = Ttxclkesc * value
	 */
	ta_go = DIV_ROUND_UP(cfg->ta_go, t_txclkesc);
	/*
	 * The value of counter for HS Tta-sure
	 * Tta-sure for turnaround
	 * Tta-sure = Ttxclkesc * value
	 */
	ta_sure = DIV_ROUND_UP(cfg->ta_sure, t_txclkesc);
	/*
	 * The value of counter for HS Tta-wait
	 * Tta-wait for turnaround
	 * Tta-wait = Ttxclkesc * value
	 */
	ta_wait = DIV_ROUND_UP(cfg->ta_get, t_txclkesc);

	for (i = 0; i < inno->pdata->num_timings; i++)
		if (inno->pll.rate <= timings[i].rate)
			break;

	if (i == inno->pdata->num_timings)
		--i;

	/*
	 * The value of counter for HS Tlpx Time
	 * Tlpx = Tpin_txbyteclkhs * (2 + value)
	 */
	if (inno->pdata->max_rate == MAX_1GHZ) {
		lpx = DIV_ROUND_UP(cfg->lpx, t_txbyteclkhs);
		if (lpx >= 2)
			lpx -= 2;
	} else {
		lpx = timings[i].lpx;
	}

	hs_prepare = timings[i].hs_prepare;
	hs_trail = timings[i].hs_trail;
	clk_lane_hs_zero = timings[i].clk_lane_hs_zero;
	data_lane_hs_zero = timings[i].data_lane_hs_zero;
	wakeup = 0x3ff;

	for (i = REGISTER_PART_CLOCK_LANE; i <= REGISTER_PART_DATA3_LANE; i++) {
		if (i == REGISTER_PART_CLOCK_LANE)
			hs_zero = clk_lane_hs_zero;
		else
			hs_zero = data_lane_hs_zero;

		phy_update_bits(inno, i, 0x05, T_LPX_CNT_MASK,
				T_LPX_CNT(lpx));
		phy_update_bits(inno, i, 0x06, T_HS_PREPARE_CNT_MASK,
				T_HS_PREPARE_CNT(hs_prepare));
		if (inno->pdata->max_rate == MAX_2_5GHZ)
			phy_update_bits(inno, i, 0x06, T_HS_ZERO_CNT_HI_MASK,
					T_HS_ZERO_CNT_HI(hs_zero >> 6));
		phy_update_bits(inno, i, 0x07, T_HS_ZERO_CNT_LO_MASK,
				T_HS_ZERO_CNT_LO(hs_zero));
		phy_update_bits(inno, i, 0x08, T_HS_TRAIL_CNT_MASK,
				T_HS_TRAIL_CNT(hs_trail));
		if (inno->pdata->max_rate == MAX_2_5GHZ)
			phy_update_bits(inno, i, 0x11, T_HS_EXIT_CNT_HI_MASK,
					T_HS_EXIT_CNT_HI(hs_exit >> 5));
		phy_update_bits(inno, i, 0x09, T_HS_EXIT_CNT_LO_MASK,
				T_HS_EXIT_CNT_LO(hs_exit));
		if (inno->pdata->max_rate == MAX_2_5GHZ)
			phy_update_bits(inno, i, 0x10, T_CLK_POST_CNT_HI_MASK,
					T_CLK_POST_CNT_HI(clk_post >> 4));
		phy_update_bits(inno, i, 0x0a, T_CLK_POST_CNT_LO_MASK,
				T_CLK_POST_CNT_LO(clk_post));
		phy_update_bits(inno, i, 0x0e, T_CLK_PRE_CNT_MASK,
				T_CLK_PRE_CNT(clk_pre));
		phy_update_bits(inno, i, 0x0c, T_WAKEUP_CNT_HI_MASK,
				T_WAKEUP_CNT_HI(wakeup >> 8));
		phy_update_bits(inno, i, 0x0d, T_WAKEUP_CNT_LO_MASK,
				T_WAKEUP_CNT_LO(wakeup));
		phy_update_bits(inno, i, 0x10, T_TA_GO_CNT_MASK,
				T_TA_GO_CNT(ta_go));
		phy_update_bits(inno, i, 0x11, T_TA_SURE_CNT_MASK,
				T_TA_SURE_CNT(ta_sure));
		phy_update_bits(inno, i, 0x12, T_TA_WAIT_CNT_MASK,
				T_TA_WAIT_CNT(ta_wait));
	}

	/* Enable all lanes on analog part */
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x00,
			LANE_EN_MASK, LANE_EN_CK | LANE_EN_3 | LANE_EN_2 |
			LANE_EN_1 | LANE_EN_0);
}

static int inno_dsidphy_power_on(struct phy *phy)
{
	struct inno_dsidphy *inno = dev_get_priv(phy->dev);

	clk_prepare_enable(inno->pclk_phy);
	clk_prepare_enable(inno->ref_clk);

	/* Bandgap power on */
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x00,
			BANDGAP_POWER_MASK, BANDGAP_POWER_ON);
	/* Enable power work */
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x00,
			POWER_WORK_MASK, POWER_WORK_ENABLE);

	inno_dsidphy_mipi_mode_enable(inno);

	return 0;
}

static int inno_dsidphy_power_off(struct phy *phy)
{
	struct inno_dsidphy *inno = dev_get_priv(phy->dev);

	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x00, LANE_EN_MASK, 0);
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x01,
			REG_LDOPD_MASK | REG_PLLPD_MASK,
			REG_LDOPD_POWER_DOWN | REG_PLLPD_POWER_DOWN);
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x00,
			POWER_WORK_MASK, POWER_WORK_DISABLE);
	phy_update_bits(inno, REGISTER_PART_ANALOG, 0x00,
			BANDGAP_POWER_MASK, BANDGAP_POWER_DOWN);

	phy_update_bits(inno, REGISTER_PART_LVDS, 0x0b, LVDS_LANE_EN_MASK, 0);
	phy_update_bits(inno, REGISTER_PART_LVDS, 0x01,
			LVDS_DIGITAL_INTERNAL_ENABLE_MASK,
			LVDS_DIGITAL_INTERNAL_DISABLE);
	phy_update_bits(inno, REGISTER_PART_LVDS, 0x0b,
			LVDS_PLL_POWER_MASK | LVDS_BANDGAP_POWER_MASK,
			LVDS_PLL_POWER_OFF | LVDS_BANDGAP_POWER_DOWN);

	clk_disable_unprepare(inno->ref_clk);
	clk_disable_unprepare(inno->pclk_phy);

	return 0;
}

static int inno_dsidphy_configure(struct phy *phy, void *params)
{
	struct inno_dsidphy *inno = dev_get_priv(phy->dev);
	struct phy_configure_opts_mipi_dphy *config = params;
	int ret;

	ret = phy_mipi_dphy_config_validate(config);
	if (ret)
		return ret;

	memcpy(&inno->dphy_cfg, config, sizeof(inno->dphy_cfg));

	return 0;
}

static const struct phy_ops inno_dsidphy_ops = {
	.configure = inno_dsidphy_configure,
	.power_on = inno_dsidphy_power_on,
	.power_off = inno_dsidphy_power_off,
};

static const struct inno_video_phy_plat_data max_1ghz_video_phy_plat_data = {
	.inno_mipi_dphy_timing_table = inno_mipi_dphy_timing_table_max_1ghz,
	.num_timings = ARRAY_SIZE(inno_mipi_dphy_timing_table_max_1ghz),
	.max_rate = MAX_1GHZ,
};

static const struct inno_video_phy_plat_data max_2_5ghz_video_phy_plat_data = {
	.inno_mipi_dphy_timing_table = inno_mipi_dphy_timing_table_max_2_5ghz,
	.num_timings = ARRAY_SIZE(inno_mipi_dphy_timing_table_max_2_5ghz),
	.max_rate = MAX_2_5GHZ,
};

static int inno_dsidphy_probe(struct udevice *dev)
{
	struct inno_dsidphy *inno = dev_get_priv(dev);
	int ret;

	inno->dev = dev;
	inno->pdata = (const struct inno_video_phy_plat_data *)dev_get_driver_data(dev);

	inno->phy_base = dev_read_addr_ptr(dev);
	if (IS_ERR(inno->phy_base))
		return PTR_ERR(inno->phy_base);

	inno->ref_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(inno->ref_clk)) {
		ret = PTR_ERR(inno->ref_clk);
		dev_err(dev, "failed to get ref clock: %d\n", ret);
		return ret;
	}

	inno->pclk_phy = devm_clk_get(dev, "pclk");
	if (IS_ERR(inno->pclk_phy)) {
		ret = PTR_ERR(inno->pclk_phy);
		dev_err(dev, "failed to get phy pclk: %d\n", ret);
		return ret;
	}

	inno->rst = devm_reset_control_get(dev, "apb");
	if (IS_ERR(inno->rst)) {
		ret = PTR_ERR(inno->rst);
		dev_err(dev, "failed to get system reset control: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id inno_dsidphy_of_match[] = {
	{
		.compatible = "rockchip,px30-dsi-dphy",
		.data = (long)&max_1ghz_video_phy_plat_data,
	}, {
		.compatible = "rockchip,rk3128-dsi-dphy",
		.data = (long)&max_1ghz_video_phy_plat_data,
	}, {
		.compatible = "rockchip,rk3368-dsi-dphy",
		.data = (long)&max_1ghz_video_phy_plat_data,
	}, {
		.compatible = "rockchip,rk3568-dsi-dphy",
		.data = (long)&max_2_5ghz_video_phy_plat_data,
	},
	{}
};

U_BOOT_DRIVER(rockchip_inno_dsidphy) = {
	.name = "rockchip-inno-dsidphy",
	.id = UCLASS_PHY,
	.of_match = inno_dsidphy_of_match,
	.probe = inno_dsidphy_probe,
	.ops = &inno_dsidphy_ops,
	.priv_auto = sizeof(struct inno_dsidphy),
};
