// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip Innosilicon HDMI PHY
 *
 * Copyright (c) 2023 Edgeble AI Technologies Pvt. Ltd.
 * Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <div64.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define UPDATE(x, h, l)		(((x) << (l)) & GENMASK((h), (l)))

/* REG: 0x01 */
#define RK3328_BYPASS_RXSENSE_EN			BIT(2)
#define RK3328_BYPASS_POWERON_EN			BIT(1)
#define RK3328_BYPASS_PLLPD_EN				BIT(0)
/* REG: 0x02 */
#define RK3328_INT_POL_HIGH				BIT(7)
#define RK3328_BYPASS_PDATA_EN				BIT(4)
#define RK3328_PDATA_EN					BIT(0)
/* REG:0x05 */
#define RK3328_INT_TMDS_CLK(x)				UPDATE(x, 7, 4)
#define RK3328_INT_TMDS_D2(x)				UPDATE(x, 3, 0)
/* REG:0x07 */
#define RK3328_INT_TMDS_D1(x)				UPDATE(x, 7, 4)
#define RK3328_INT_TMDS_D0(x)				UPDATE(x, 3, 0)
/* for all RK3328_INT_TMDS_*, ESD_DET as defined in 0xc8-0xcb */
#define RK3328_INT_AGND_LOW_PULSE_LOCKED		BIT(3)
#define RK3328_INT_RXSENSE_LOW_PULSE_LOCKED		BIT(2)
#define RK3328_INT_VSS_AGND_ESD_DET			BIT(1)
#define RK3328_INT_AGND_VSS_ESD_DET			BIT(0)
/* REG: 0xa0 */
#define RK3328_PCLK_VCO_DIV_5_MASK			BIT(1)
#define RK3328_PCLK_VCO_DIV_5(x)			UPDATE(x, 1, 1)
#define RK3328_PRE_PLL_POWER_DOWN			BIT(0)
/* REG: 0xa1 */
#define RK3328_PRE_PLL_PRE_DIV_MASK			GENMASK(5, 0)
#define RK3328_PRE_PLL_PRE_DIV(x)			UPDATE(x, 5, 0)
/* REG: 0xa2 */
/* unset means center spread */
#define RK3328_SPREAD_SPECTRUM_MOD_DOWN			BIT(7)
#define RK3328_SPREAD_SPECTRUM_MOD_DISABLE		BIT(6)
#define RK3328_PRE_PLL_FRAC_DIV_DISABLE			UPDATE(3, 5, 4)
#define RK3328_PRE_PLL_FB_DIV_11_8_MASK			GENMASK(3, 0)
#define RK3328_PRE_PLL_FB_DIV_11_8(x)			UPDATE((x) >> 8, 3, 0)
/* REG: 0xa3 */
#define RK3328_PRE_PLL_FB_DIV_7_0(x)			UPDATE(x, 7, 0)
/* REG: 0xa4*/
#define RK3328_PRE_PLL_TMDSCLK_DIV_C_MASK		GENMASK(1, 0)
#define RK3328_PRE_PLL_TMDSCLK_DIV_C(x)			UPDATE(x, 1, 0)
#define RK3328_PRE_PLL_TMDSCLK_DIV_B_MASK		GENMASK(3, 2)
#define RK3328_PRE_PLL_TMDSCLK_DIV_B(x)			UPDATE(x, 3, 2)
#define RK3328_PRE_PLL_TMDSCLK_DIV_A_MASK		GENMASK(5, 4)
#define RK3328_PRE_PLL_TMDSCLK_DIV_A(x)			UPDATE(x, 5, 4)
/* REG: 0xa5 */
#define RK3328_PRE_PLL_PCLK_DIV_B_SHIFT			5
#define RK3328_PRE_PLL_PCLK_DIV_B_MASK			GENMASK(6, 5)
#define RK3328_PRE_PLL_PCLK_DIV_B(x)			UPDATE(x, 6, 5)
#define RK3328_PRE_PLL_PCLK_DIV_A_MASK			GENMASK(4, 0)
#define RK3328_PRE_PLL_PCLK_DIV_A(x)			UPDATE(x, 4, 0)
/* REG: 0xa6 */
#define RK3328_PRE_PLL_PCLK_DIV_C_SHIFT			5
#define RK3328_PRE_PLL_PCLK_DIV_C_MASK			GENMASK(6, 5)
#define RK3328_PRE_PLL_PCLK_DIV_C(x)			UPDATE(x, 6, 5)
#define RK3328_PRE_PLL_PCLK_DIV_D_MASK			GENMASK(4, 0)
#define RK3328_PRE_PLL_PCLK_DIV_D(x)			UPDATE(x, 4, 0)
/* REG: 0xa9 */
#define RK3328_PRE_PLL_LOCK_STATUS			BIT(0)
/* REG: 0xaa */
#define RK3328_POST_PLL_POST_DIV_ENABLE			GENMASK(3, 2)
#define RK3328_POST_PLL_REFCLK_SEL_TMDS			BIT(1)
#define RK3328_POST_PLL_POWER_DOWN			BIT(0)
/* REG:0xab */
#define RK3328_POST_PLL_FB_DIV_8(x)			UPDATE((x) >> 8, 7, 7)
#define RK3328_POST_PLL_PRE_DIV(x)			UPDATE(x, 4, 0)
/* REG: 0xac */
#define RK3328_POST_PLL_FB_DIV_7_0(x)			UPDATE(x, 7, 0)
/* REG: 0xad */
#define RK3328_POST_PLL_POST_DIV_MASK			GENMASK(1, 0)
#define RK3328_POST_PLL_POST_DIV_2			0x0
#define RK3328_POST_PLL_POST_DIV_4			0x1
#define RK3328_POST_PLL_POST_DIV_8			0x3
/* REG: 0xaf */
#define RK3328_POST_PLL_LOCK_STATUS			BIT(0)
/* REG: 0xb0 */
#define RK3328_BANDGAP_ENABLE				BIT(2)
/* REG: 0xb2 */
#define RK3328_TMDS_CLK_DRIVER_EN			BIT(3)
#define RK3328_TMDS_D2_DRIVER_EN			BIT(2)
#define RK3328_TMDS_D1_DRIVER_EN			BIT(1)
#define RK3328_TMDS_D0_DRIVER_EN			BIT(0)
#define RK3328_TMDS_DRIVER_ENABLE		(RK3328_TMDS_CLK_DRIVER_EN | \
						RK3328_TMDS_D2_DRIVER_EN | \
						RK3328_TMDS_D1_DRIVER_EN | \
						RK3328_TMDS_D0_DRIVER_EN)
/* REG:0xc5 */
#define RK3328_BYPASS_TERM_RESISTOR_CALIB		BIT(7)
#define RK3328_TERM_RESISTOR_CALIB_SPEED_14_8(x)	UPDATE((x) >> 8, 6, 0)
/* REG:0xc6 */
#define RK3328_TERM_RESISTOR_CALIB_SPEED_7_0(x)		UPDATE(x, 7, 0)
/* REG:0xc7 */
#define RK3328_TERM_RESISTOR_50				UPDATE(0, 2, 1)
#define RK3328_TERM_RESISTOR_62_5			UPDATE(1, 2, 1)
#define RK3328_TERM_RESISTOR_75				UPDATE(2, 2, 1)
#define RK3328_TERM_RESISTOR_100			UPDATE(3, 2, 1)
/* REG 0xc8 - 0xcb */
#define RK3328_ESD_DETECT_MASK				GENMASK(7, 6)
#define RK3328_ESD_DETECT_340MV				(0x0 << 6)
#define RK3328_ESD_DETECT_280MV				(0x1 << 6)
#define RK3328_ESD_DETECT_260MV				(0x2 << 6)
#define RK3328_ESD_DETECT_240MV				(0x3 << 6)
/* resistors can be used in parallel */
#define RK3328_TMDS_TERM_RESIST_MASK			GENMASK(5, 0)
#define RK3328_TMDS_TERM_RESIST_75			BIT(5)
#define RK3328_TMDS_TERM_RESIST_150			BIT(4)
#define RK3328_TMDS_TERM_RESIST_300			BIT(3)
#define RK3328_TMDS_TERM_RESIST_600			BIT(2)
#define RK3328_TMDS_TERM_RESIST_1000			BIT(1)
#define RK3328_TMDS_TERM_RESIST_2000			BIT(0)
/* REG: 0xd1 */
#define RK3328_PRE_PLL_FRAC_DIV_23_16(x)		UPDATE((x) >> 16, 7, 0)
/* REG: 0xd2 */
#define RK3328_PRE_PLL_FRAC_DIV_15_8(x)			UPDATE((x) >> 8, 7, 0)
/* REG: 0xd3 */
#define RK3328_PRE_PLL_FRAC_DIV_7_0(x)			UPDATE(x, 7, 0)

struct phy_config {
	unsigned long	tmdsclock;
	u8		regs[14];
};

struct pre_pll_config {
	unsigned long pixclock;
	unsigned long tmdsclock;
	u8 prediv;
	u16 fbdiv;
	u8 tmds_div_a;
	u8 tmds_div_b;
	u8 tmds_div_c;
	u8 pclk_div_a;
	u8 pclk_div_b;
	u8 pclk_div_c;
	u8 pclk_div_d;
	u8 vco_div_5_en;
	u32 fracdiv;
};

struct post_pll_config {
	unsigned long tmdsclock;
	u8 prediv;
	u16 fbdiv;
	u8 postdiv;
	u8 version;
};

struct inno_hdmi_phy_plat_ops {
	void (*init)(struct phy *phy);
	int (*power_on)(struct phy *phy, const struct post_pll_config *cfg,
			const struct phy_config *phy_cfg);
	void (*power_off)(struct phy *phy);
	void (*clk_enable)(struct phy *phy);
	void (*clk_disable)(struct phy *phy);
	unsigned long (*clk_recalc_rate)(struct phy *phy,
					 unsigned long parent_rate);
	long (*clk_round_rate)(struct phy *phy, unsigned long rate);
	int (*clk_set_rate)(struct phy *phy, unsigned long rate,
			    unsigned long parent_rate);
};

enum inno_hdmi_phy_type {
	INNO_HDMI_PHY_RK3328,
};

struct inno_hdmi_phy_data {
	enum inno_hdmi_phy_type	phy_type;
	const struct inno_hdmi_phy_plat_ops *plat_ops;
	const struct phy_config	*phy_cfg_table;
};

struct inno_hdmi_phy {
	struct udevice *dev;
	ofnode node;
	void *regs;

	struct clk refoclk;
	struct clk sysclk;
	unsigned long tmdsclock;
	unsigned long pixclock;
	u32 bus_width;
	struct phy_config *phy_cfg;
	const struct inno_hdmi_phy_data *data;
};

static const struct pre_pll_config pre_pll_cfg_table[] = {
	{ 25175000,  25175000,  3,  125, 3, 1, 1,  1, 3, 3,  4, 0, 0xe00000},
	{ 25175000,  31468750,  1,   41, 0, 3, 3,  1, 3, 3,  4, 0, 0xf5554f},
	{ 27000000,  27000000,  1,   36, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 27000000,  33750000,  1,   45, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 31500000,  31500000,  1,   42, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 31500000,  39375000,  1,  105, 1, 3, 3, 10, 0, 3,  4, 0,      0x0},
	{ 33750000,  33750000,  1,   45, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 33750000,  42187500,  1,  169, 2, 3, 3, 15, 0, 3,  4, 0,      0x0},
	{ 35500000,  35500000,  1,   71, 2, 2, 2,  6, 0, 3,  4, 0,      0x0},
	{ 35500000,  44375000,  1,   74, 3, 1, 1, 25, 0, 1,  1, 0,      0x0},
	{ 36000000,  36000000,  1,   36, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{ 36000000,  45000000,  1,   45, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{ 40000000,  40000000,  1,   40, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{ 40000000,  50000000,  1,   50, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{ 49500000,  49500000,  1,   66, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 49500000,  61875000,  1,  165, 1, 3, 3, 10, 0, 3,  4, 0,      0x0},
	{ 50000000,  50000000,  1,   50, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{ 50000000,  62500000,  1,  125, 2, 2, 2, 15, 0, 2,  2, 0,      0x0},
	{ 54000000,  54000000,  1,   36, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{ 54000000,  67500000,  1,   45, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{ 56250000,  56250000,  1,   75, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 56250000,  70312500,  1,  117, 3, 1, 1, 25, 0, 1,  1, 0,      0x0},
	{ 59341000,  59341000,  1,  118, 2, 2, 2,  6, 0, 3,  4, 0, 0xae978d},
	{ 59341000,  74176250,  2,  148, 2, 1, 1, 15, 0, 1,  1, 0, 0x5a3d70},
	{ 59400000,  59400000,  1,   99, 3, 1, 1,  1, 3, 3,  4, 0,      0x0},
	{ 59400000,  74250000,  1,   99, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 65000000,  65000000,  1,   65, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{ 65000000,  81250000,  3,  325, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 68250000,  68250000,  1,   91, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 68250000,  85312500,  1,  142, 3, 1, 1, 25, 0, 1,  1, 0,      0x0},
	{ 71000000,  71000000,  1,   71, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{ 71000000,  88750000,  3,  355, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 72000000,  72000000,  1,   36, 2, 0, 0,  1, 1, 2,  2, 0,      0x0},
	{ 72000000,  90000000,  1,   60, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{ 73250000,  73250000,  3,  293, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 73250000,  91562500,  1,   61, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{ 74176000,  74176000,  1,   37, 2, 0, 0,  1, 1, 2,  2, 0, 0x16872b},
	{ 74176000,  92720000,  2,  185, 2, 1, 1, 15, 0, 1,  1, 0, 0x70a3d7},
	{ 74250000,  74250000,  1,   99, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 74250000,  92812500,  4,  495, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 75000000,  75000000,  1,   50, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{ 75000000,  93750000,  1,  125, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 78750000,  78750000,  1,  105, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 78750000,  98437500,  1,  164, 3, 1, 1, 25, 0, 1,  1, 0,      0x0},
	{ 79500000,  79500000,  1,   53, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{ 79500000,  99375000,  1,  199, 2, 2, 2, 15, 0, 2,  2, 0,      0x0},
	{ 83500000,  83500000,  2,  167, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{ 83500000, 104375000,  1,  104, 2, 1, 1, 15, 0, 1,  1, 0, 0x600000},
	{ 85500000,  85500000,  1,   57, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{ 85500000, 106875000,  1,  178, 3, 1, 1, 25, 0, 1,  1, 0,      0x0},
	{ 85750000,  85750000,  3,  343, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 85750000, 107187500,  1,  143, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{ 88750000,  88750000,  3,  355, 0, 3, 3,  1, 2, 3,  4, 0,      0x0},
	{ 88750000, 110937500,  1,  110, 2, 1, 1, 15, 0, 1,  1, 0, 0xf00000},
	{ 94500000,  94500000,  1,   63, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{ 94500000, 118125000,  1,  197, 3, 1, 1, 25, 0, 1,  1, 0,      0x0},
	{101000000, 101000000,  1,  101, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{101000000, 126250000,  1,   42, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{102250000, 102250000,  4,  409, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{102250000, 127812500,  1,  128, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{106500000, 106500000,  1,   71, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{106500000, 133125000,  1,  133, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{108000000, 108000000,  1,   36, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{108000000, 135000000,  1,   45, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{115500000, 115500000,  1,   77, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{115500000, 144375000,  1,   48, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{117500000, 117500000,  2,  235, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{117500000, 146875000,  1,   49, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{119000000, 119000000,  1,  119, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{119000000, 148750000,  3,  148, 0, 1, 1,  1, 3, 1,  1, 0, 0xc00000},
	{121750000, 121750000,  4,  487, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{121750000, 152187500,  1,  203, 0, 3, 3,  1, 3, 3,  4, 0,      0x0},
	{122500000, 122500000,  2,  245, 2, 1, 1,  1, 1, 3,  4, 0,      0x0},
	{122500000, 153125000,  1,   51, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{135000000, 135000000,  1,   45, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{135000000, 168750000,  1,  169, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{136750000, 136750000,  1,   68, 2, 0, 0,  1, 1, 2,  2, 0, 0x600000},
	{136750000, 170937500,  1,  113, 0, 2, 2,  1, 3, 2,  2, 0, 0xf5554f},
	{140250000, 140250000,  2,  187, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{140250000, 175312500,  1,  117, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{146250000, 146250000,  2,  195, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{146250000, 182812500,  1,   61, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{148250000, 148250000,  3,  222, 2, 0, 0,  1, 1, 2,  2, 0, 0x600000},
	{148250000, 185312500,  1,  123, 0, 2, 2,  1, 3, 2,  2, 0, 0x8aaab0},
	{148352000, 148352000,  2,  148, 2, 0, 0,  1, 1, 2,  2, 0, 0x5a1cac},
	{148352000, 185440000,  3,  185, 0, 1, 1,  1, 3, 1,  1, 0, 0x70a3d7},
	{148500000, 148500000,  1,   99, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{148500000, 185625000,  4,  495, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{154000000, 154000000,  1,   77, 2, 0, 0,  1, 1, 2,  2, 0,      0x0},
	{154000000, 192500000,  1,   64, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{156000000, 156000000,  1,   52, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{156000000, 195000000,  1,   65, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{156750000, 156750000,  2,  209, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{156750000, 195937500,  1,  196, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{157000000, 157000000,  2,  157, 2, 0, 0,  1, 1, 2,  2, 0,      0x0},
	{157000000, 196250000,  1,  131, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{157500000, 157500000,  1,  105, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{157500000, 196875000,  1,  197, 2, 1, 1, 15, 0, 1,  1, 0,      0x0},
	{162000000, 162000000,  1,   54, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{162000000, 202500000,  2,  135, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{175500000, 175500000,  1,  117, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{175500000, 219375000,  1,   73, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{179500000, 179500000,  3,  359, 0, 2, 2,  1, 0, 3,  4, 0,      0x0},
	{179500000, 224375000,  1,   75, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{182750000, 182750000,  1,   91, 2, 0, 0,  1, 1, 2,  2, 0, 0x600000},
	{182750000, 228437500,  1,  152, 0, 2, 2,  1, 3, 2,  2, 0, 0x4aaab0},
	{182750000, 228437500,  1,  152, 0, 2, 2,  1, 3, 2,  2, 0, 0x4aaab0},
	{187000000, 187000000,  2,  187, 2, 0, 0,  1, 1, 2,  2, 0,      0x0},
	{187000000, 233750000,  1,   39, 0, 0, 0,  1, 3, 0,  0, 1,      0x0},
	{187250000, 187250000,  3,  280, 2, 0, 0,  1, 1, 2,  2, 0, 0xe00000},
	{187250000, 234062500,  1,  156, 0, 2, 2,  1, 3, 2,  2, 0,  0xaaab0},
	{189000000, 189000000,  1,   63, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{189000000, 236250000,  1,   79, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{193250000, 193250000,  3,  289, 2, 0, 0,  1, 1, 2,  2, 0, 0xe00000},
	{193250000, 241562500,  1,  161, 0, 2, 2,  1, 3, 2,  2, 0,  0xaaab0},
	{202500000, 202500000,  2,  135, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{202500000, 253125000,  1,  169, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{204750000, 204750000,  4,  273, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{204750000, 255937500,  1,  171, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{208000000, 208000000,  1,  104, 2, 0, 0,  1, 1, 2,  2, 0,      0x0},
	{208000000, 260000000,  1,  173, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{214750000, 214750000,  1,  107, 2, 0, 0,  1, 1, 2,  2, 0, 0x600000},
	{214750000, 268437500,  1,  178, 0, 2, 2,  1, 3, 2,  2, 0, 0xf5554f},
	{218250000, 218250000,  4,  291, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{218250000, 272812500,  1,   91, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{229500000, 229500000,  2,  153, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{229500000, 286875000,  1,  191, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{234000000, 234000000,  1,   39, 0, 0, 0,  1, 0, 1,  1, 0,      0x0},
	{234000000, 292500000,  1,  195, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{241500000, 241500000,  2,  161, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{241500000, 301875000,  1,  201, 0, 2, 2,  1, 3, 2,  2, 0,      0x0},
	{245250000, 245250000,  4,  327, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{245250000, 306562500,  1,   51, 0, 0, 0,  1, 3, 0,  0, 1,      0x0},
	{245500000, 245500000,  4,  491, 2, 0, 0,  1, 1, 2,  2, 0,      0x0},
	{245500000, 306875000,  1,   51, 0, 0, 0,  1, 3, 0,  0, 1,      0x0},
	{261000000, 261000000,  1,   87, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{261000000, 326250000,  1,  109, 0, 1, 1,  1, 3, 1,  1, 0,      0x0},
	{268250000, 268250000,  9,  402, 0, 0, 0,  1, 0, 1,  1, 0, 0x600000},
	{268250000, 335312500,  1,  111, 0, 1, 1,  1, 3, 1,  1, 0, 0xc5554f},
	{268500000, 268500000,  2,  179, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{268500000, 335625000,  1,   56, 0, 0, 0,  1, 3, 0,  0, 1,      0x0},
	{281250000, 281250000,  4,  375, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{281250000, 351562500,  1,  117, 0, 3, 1,  1, 3, 1,  1, 0,      0x0},
	{288000000, 288000000,  1,   48, 0, 0, 0,  1, 0, 1,  1, 0,      0x0},
	{288000000, 360000000,  1,   60, 0, 2, 0,  1, 3, 0,  0, 1,      0x0},
	{296703000, 296703000,  1,   49, 0, 0, 0,  1, 0, 1,  1, 0, 0x7353f7},
	{296703000, 370878750,  1,  123, 0, 3, 1,  1, 3, 1,  1, 0, 0xa051eb},
	{297000000, 297000000,  1,   99, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{297000000, 371250000,  4,  495, 0, 3, 1,  1, 3, 1,  1, 0,      0x0},
	{312250000, 312250000,  9,  468, 0, 0, 0,  1, 0, 1,  1, 0, 0x600000},
	{312250000, 390312500,  1,  130, 0, 3, 1,  1, 3, 1,  1, 0, 0x1aaab0},
	{317000000, 317000000,  3,  317, 0, 1, 1,  1, 0, 2,  2, 0,      0x0},
	{317000000, 396250000,  1,   66, 0, 2, 0,  1, 3, 0,  0, 1,      0x0},
	{319750000, 319750000,  3,  159, 0, 0, 0,  1, 0, 1,  1, 0, 0xe00000},
	{319750000, 399687500,  3,  199, 0, 2, 0,  1, 3, 0,  0, 1, 0xd80000},
	{333250000, 333250000,  9,  499, 0, 0, 0,  1, 0, 1,  1, 0, 0xe00000},
	{333250000, 416562500,  1,  138, 0, 3, 1,  1, 3, 1,  1, 0, 0xdaaab0},
	{348500000, 348500000,  9,  522, 0, 2, 0,  1, 0, 1,  1, 0, 0xc00000},
	{348500000, 435625000,  1,  145, 0, 3, 1,  1, 3, 1,  1, 0, 0x35554f},
	{356500000, 356500000,  9,  534, 0, 2, 0,  1, 0, 1,  1, 0, 0xc00000},
	{356500000, 445625000,  1,  148, 0, 3, 1,  1, 3, 1,  1, 0, 0x8aaab0},
	{380500000, 380500000,  9,  570, 0, 2, 0,  1, 0, 1,  1, 0, 0xc00000},
	{380500000, 475625000,  1,  158, 0, 3, 1,  1, 3, 1,  1, 0, 0x8aaab0},
	{443250000, 443250000,  1,   73, 0, 2, 0,  1, 0, 1,  1, 0, 0xe00000},
	{443250000, 554062500,  1,   92, 0, 2, 0,  1, 3, 0,  0, 1, 0x580000},
	{505250000, 505250000,  9,  757, 0, 2, 0,  1, 0, 1,  1, 0, 0xe00000},
	{552750000, 552750000,  3,  276, 0, 2, 0,  1, 0, 1,  1, 0, 0x600000},
	{593407000, 296703500,  3,  296, 0, 1, 1,  1, 0, 1,  1, 0, 0xb41893},
	{593407000, 370879375,  4,  494, 0, 3, 1,  1, 3, 0,  0, 1, 0x817e4a},
	{593407000, 593407000,  3,  296, 0, 2, 0,  1, 0, 1,  1, 0, 0xb41893},
	{594000000, 297000000,  1,   99, 0, 1, 1,  1, 0, 1,  1, 0,      0x0},
	{594000000, 371250000,  4,  495, 0, 3, 1,  1, 3, 0,  0, 1,      0x0},
	{594000000, 594000000,  1,   99, 0, 2, 0,  1, 0, 1,  1, 0,      0x0},
	{ /* sentinel */ }
};

static const struct post_pll_config post_pll_cfg_table[] = {
	{33750000,  1, 40, 8, 1},
	{33750000,  1, 80, 8, 2},
	{74250000,  1, 40, 8, 1},
	{74250000, 18, 80, 8, 2},
	{148500000, 2, 40, 4, 3},
	{297000000, 4, 40, 2, 3},
	{594000000, 8, 40, 1, 3},
	{ /* sentinel */ }
};

/* phy tuning values for an undocumented set of registers */
static const struct phy_config rk3328_phy_cfg[] = {
	{	165000000, {
			0x07, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x08, 0x08, 0x08,
			0x00, 0xac, 0xcc, 0xcc, 0xcc,
		},
	}, {
		340000000, {
			0x0b, 0x0d, 0x0d, 0x0d, 0x07, 0x15, 0x08, 0x08, 0x08,
			0x3f, 0xac, 0xcc, 0xcd, 0xdd,
		},
	}, {
		594000000, {
			0x10, 0x1a, 0x1a, 0x1a, 0x07, 0x15, 0x08, 0x08, 0x08,
			0x00, 0xac, 0xcc, 0xcc, 0xcc,
		},
	}, { /* sentinel */ },
};

static inline void inno_write(struct inno_hdmi_phy *inno, u32 reg, u8 val)
{
	writel(val, inno->regs + (reg * 4));
}

static inline u8 inno_read(struct inno_hdmi_phy *inno, u32 reg)
{
	u32 val;

	val = readl(inno->regs + (reg * 4));

	return val;
}

static inline void inno_update_bits(struct inno_hdmi_phy *inno, u8 reg,
				    u8 mask, u8 val)
{
	u32 tmp, orig;

	orig = inno_read(inno, reg);
	tmp = orig & ~mask;
	tmp |= val & mask;
	inno_write(inno, reg, tmp);
}

#define inno_poll(inno, reg, val, cond, sleep_us, timeout_us) \
	readl_poll_sleep_timeout((inno)->regs + ((reg) * 4), val, cond, sleep_us, timeout_us)

static unsigned long inno_hdmi_phy_get_tmdsclk(struct inno_hdmi_phy *inno,
					       unsigned long rate)
{
	int bus_width = inno->bus_width;

	switch (bus_width) {
	case 4:
	case 5:
	case 6:
	case 10:
	case 12:
	case 16:
		return (u64)rate * bus_width / 8;
	default:
		return rate;
	}
}

static
unsigned long inno_hdmi_phy_rk3328_clk_recalc_rate(struct phy *phy,
						   unsigned long parent_rate)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);
	unsigned long frac;
	u8 nd, no_a, no_b, no_d;
	u64 vco;
	u16 nf;

	nd = inno_read(inno, 0xa1) & RK3328_PRE_PLL_PRE_DIV_MASK;
	nf = ((inno_read(inno, 0xa2) & RK3328_PRE_PLL_FB_DIV_11_8_MASK) << 8);
	nf |= inno_read(inno, 0xa3);
	vco = parent_rate * nf;

	if (!(inno_read(inno, 0xa2) & RK3328_PRE_PLL_FRAC_DIV_DISABLE)) {
		frac = inno_read(inno, 0xd3) |
		       (inno_read(inno, 0xd2) << 8) |
		       (inno_read(inno, 0xd1) << 16);
		vco += DIV_ROUND_CLOSEST(parent_rate * frac, (1 << 24));
	}

	if (inno_read(inno, 0xa0) & RK3328_PCLK_VCO_DIV_5_MASK) {
		do_div(vco, nd * 5);
	} else {
		no_a = inno_read(inno, 0xa5) & RK3328_PRE_PLL_PCLK_DIV_A_MASK;
		no_b = inno_read(inno, 0xa5) & RK3328_PRE_PLL_PCLK_DIV_B_MASK;
		no_b >>= RK3328_PRE_PLL_PCLK_DIV_B_SHIFT;
		no_b += 2;
		no_d = inno_read(inno, 0xa6) & RK3328_PRE_PLL_PCLK_DIV_D_MASK;

		do_div(vco, (nd * (no_a == 1 ? no_b : no_a) * no_d * 2));
	}

	inno->pixclock = DIV_ROUND_CLOSEST((unsigned long)vco, 1000) * 1000;

	dev_info(phy->dev, "rate %lu vco %llu\n", inno->pixclock, vco);

	return inno->pixclock;
}

static long inno_hdmi_phy_rk3328_clk_round_rate(struct phy *phy,
						unsigned long rate)
{
	const struct pre_pll_config *cfg = pre_pll_cfg_table;

	rate = (rate / 1000) * 1000;

	for (; cfg->pixclock != 0; cfg++)
		if (cfg->pixclock == rate)
			break;

	if (cfg->pixclock == 0)
		return -EINVAL;

	return cfg->pixclock;
}

static const
struct pre_pll_config *inno_hdmi_phy_get_pre_pll_cfg(struct inno_hdmi_phy *inno,
						     unsigned long rate)
{
	const struct pre_pll_config *cfg = pre_pll_cfg_table;
	unsigned long tmdsclock = inno_hdmi_phy_get_tmdsclk(inno, rate);

	for (; cfg->pixclock != 0; cfg++)
		if (cfg->pixclock == rate && cfg->tmdsclock == tmdsclock)
			break;

	if (cfg->pixclock == 0)
		return ERR_PTR(-EINVAL);

	return cfg;
}

static int
inno_hdmi_phy_rk3328_clk_set_rate(struct phy *phy,
				  unsigned long rate,
				  unsigned long parent_rate)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);
	unsigned long tmdsclock = inno_hdmi_phy_get_tmdsclk(inno, rate);
	const struct pre_pll_config *cfg;
	u32 val;
	int ret;

	dev_info(phy->dev, "rate %lu tmdsclk %lu\n", rate, tmdsclock);

	if (inno->pixclock == rate && inno->tmdsclock == tmdsclock)
		return 0;

	cfg = inno_hdmi_phy_get_pre_pll_cfg(inno, rate);
	if (IS_ERR(cfg))
		return PTR_ERR(cfg);

	inno_update_bits(inno, 0xa0, RK3328_PRE_PLL_POWER_DOWN,
			 RK3328_PRE_PLL_POWER_DOWN);

	/* Configure pre-pll */
	inno_update_bits(inno, 0xa0, RK3328_PCLK_VCO_DIV_5_MASK,
			 RK3328_PCLK_VCO_DIV_5(cfg->vco_div_5_en));
	inno_write(inno, 0xa1, RK3328_PRE_PLL_PRE_DIV(cfg->prediv));

	val = RK3328_SPREAD_SPECTRUM_MOD_DISABLE;
	if (!cfg->fracdiv)
		val |= RK3328_PRE_PLL_FRAC_DIV_DISABLE;
	inno_write(inno, 0xa2, RK3328_PRE_PLL_FB_DIV_11_8(cfg->fbdiv) | val);
	inno_write(inno, 0xa3, RK3328_PRE_PLL_FB_DIV_7_0(cfg->fbdiv));
	inno_write(inno, 0xa5, RK3328_PRE_PLL_PCLK_DIV_A(cfg->pclk_div_a) |
		   RK3328_PRE_PLL_PCLK_DIV_B(cfg->pclk_div_b));
	inno_write(inno, 0xa6, RK3328_PRE_PLL_PCLK_DIV_C(cfg->pclk_div_c) |
		   RK3328_PRE_PLL_PCLK_DIV_D(cfg->pclk_div_d));
	inno_write(inno, 0xa4, RK3328_PRE_PLL_TMDSCLK_DIV_C(cfg->tmds_div_c) |
		   RK3328_PRE_PLL_TMDSCLK_DIV_A(cfg->tmds_div_a) |
		   RK3328_PRE_PLL_TMDSCLK_DIV_B(cfg->tmds_div_b));
	inno_write(inno, 0xd3, RK3328_PRE_PLL_FRAC_DIV_7_0(cfg->fracdiv));
	inno_write(inno, 0xd2, RK3328_PRE_PLL_FRAC_DIV_15_8(cfg->fracdiv));
	inno_write(inno, 0xd1, RK3328_PRE_PLL_FRAC_DIV_23_16(cfg->fracdiv));

	inno_update_bits(inno, 0xa0, RK3328_PRE_PLL_POWER_DOWN, 0);

	/* Wait for Pre-PLL lock */
	ret = inno_poll(inno, 0xa9, val, val & RK3328_PRE_PLL_LOCK_STATUS,
			1000, 10000);
	if (ret) {
		dev_err(phy->dev, "Pre-PLL locking failed\n");
		return ret;
	}

	inno->pixclock = rate;
	inno->tmdsclock = tmdsclock;

	return 0;
}

static void inno_hdmi_phy_rk3328_clk_enable(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);

	inno_update_bits(inno, 0xa0, RK3328_PRE_PLL_POWER_DOWN, 0);
}

static void inno_hdmi_phy_rk3328_clk_disable(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);

	inno_update_bits(inno, 0xa0, RK3328_PRE_PLL_POWER_DOWN,
			 RK3328_PRE_PLL_POWER_DOWN);
}

static int
inno_hdmi_phy_rk3328_power_on(struct phy *phy,
			      const struct post_pll_config *cfg,
			      const struct phy_config *phy_cfg)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);
	int ret;
	u32 v;

	inno_update_bits(inno, 0x02, RK3328_PDATA_EN, 0);
	inno_update_bits(inno, 0xaa, RK3328_POST_PLL_POWER_DOWN,
			 RK3328_POST_PLL_POWER_DOWN);

	inno_write(inno, 0xac, RK3328_POST_PLL_FB_DIV_7_0(cfg->fbdiv));
	if (cfg->postdiv == 1) {
		inno_write(inno, 0xab, RK3328_POST_PLL_FB_DIV_8(cfg->fbdiv) |
			   RK3328_POST_PLL_PRE_DIV(cfg->prediv));
		inno_write(inno, 0xaa, RK3328_POST_PLL_REFCLK_SEL_TMDS |
			   RK3328_POST_PLL_POWER_DOWN);
	} else {
		v = (cfg->postdiv / 2) - 1;
		v &= RK3328_POST_PLL_POST_DIV_MASK;
		inno_write(inno, 0xad, v);
		inno_write(inno, 0xab, RK3328_POST_PLL_FB_DIV_8(cfg->fbdiv) |
			   RK3328_POST_PLL_PRE_DIV(cfg->prediv));
		inno_write(inno, 0xaa, RK3328_POST_PLL_POST_DIV_ENABLE |
			   RK3328_POST_PLL_REFCLK_SEL_TMDS |
			   RK3328_POST_PLL_POWER_DOWN);
	}

	for (v = 0; v < 14; v++)
		inno_write(inno, 0xb5 + v, phy_cfg->regs[v]);

	/* set ESD detection threshold for TMDS CLK, D2, D1 and D0 */
	for (v = 0; v < 4; v++)
		inno_update_bits(inno, 0xc8 + v, RK3328_ESD_DETECT_MASK,
				 RK3328_ESD_DETECT_340MV);

	if (phy_cfg->tmdsclock > 340000000) {
		/* Set termination resistor to 100ohm */
		v = clk_get_rate(&inno->sysclk) / 100000;
		inno_write(inno, 0xc5, RK3328_TERM_RESISTOR_CALIB_SPEED_14_8(v)
			   | RK3328_BYPASS_TERM_RESISTOR_CALIB);
		inno_write(inno, 0xc6, RK3328_TERM_RESISTOR_CALIB_SPEED_7_0(v));
		inno_write(inno, 0xc7, RK3328_TERM_RESISTOR_100);
		inno_update_bits(inno, 0xc5,
				 RK3328_BYPASS_TERM_RESISTOR_CALIB, 0);
	} else {
		inno_write(inno, 0xc5, RK3328_BYPASS_TERM_RESISTOR_CALIB);

		/* clk termination resistor is 50ohm (parallel resistors) */
		if (phy_cfg->tmdsclock > 165000000)
			inno_update_bits(inno, 0xc8,
					 RK3328_TMDS_TERM_RESIST_MASK,
					 RK3328_TMDS_TERM_RESIST_75 |
					 RK3328_TMDS_TERM_RESIST_150);

		/* data termination resistor for D2, D1 and D0 is 150ohm */
		for (v = 0; v < 3; v++)
			inno_update_bits(inno, 0xc9 + v,
					 RK3328_TMDS_TERM_RESIST_MASK,
					 RK3328_TMDS_TERM_RESIST_150);
	}

	inno_update_bits(inno, 0xaa, RK3328_POST_PLL_POWER_DOWN, 0);
	inno_update_bits(inno, 0xb0, RK3328_BANDGAP_ENABLE,
			 RK3328_BANDGAP_ENABLE);
	inno_update_bits(inno, 0xb2, RK3328_TMDS_DRIVER_ENABLE,
			 RK3328_TMDS_DRIVER_ENABLE);

	/* Wait for post PLL lock */
	ret = inno_poll(inno, 0xaf, v, v & RK3328_POST_PLL_LOCK_STATUS,
			1000, 10000);
	if (ret) {
		dev_err(phy->dev, "Post-PLL locking failed\n");
		return ret;
	}

	if (phy_cfg->tmdsclock > 340000000)
		mdelay(100);

	inno_update_bits(inno, 0x02, RK3328_PDATA_EN, RK3328_PDATA_EN);

	/* Enable PHY IRQ */
	inno_write(inno, 0x05, RK3328_INT_TMDS_CLK(RK3328_INT_VSS_AGND_ESD_DET)
		   | RK3328_INT_TMDS_D2(RK3328_INT_VSS_AGND_ESD_DET));
	inno_write(inno, 0x07, RK3328_INT_TMDS_D1(RK3328_INT_VSS_AGND_ESD_DET)
		   | RK3328_INT_TMDS_D0(RK3328_INT_VSS_AGND_ESD_DET));

	return 0;
}

static void inno_hdmi_phy_rk3328_power_off(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);

	inno_update_bits(inno, 0xb2, RK3328_TMDS_DRIVER_ENABLE, 0);
	inno_update_bits(inno, 0xb0, RK3328_BANDGAP_ENABLE, 0);
	inno_update_bits(inno, 0xaa, RK3328_POST_PLL_POWER_DOWN,
			 RK3328_POST_PLL_POWER_DOWN);

	/* Disable PHY IRQ */
	inno_write(inno, 0x05, 0);
	inno_write(inno, 0x07, 0);
}

static void inno_hdmi_phy_rk3328_init(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);
	const struct inno_hdmi_phy_plat_ops *plat_ops = inno->data->plat_ops;

	/*
	 * Use phy internal register control
	 * rxsense/poweron/pllpd/pdataen signal.
	 */
	inno_write(inno, 0x01, RK3328_BYPASS_RXSENSE_EN |
		   RK3328_BYPASS_POWERON_EN |
		   RK3328_BYPASS_PLLPD_EN);
	inno_write(inno, 0x02, RK3328_INT_POL_HIGH | RK3328_BYPASS_PDATA_EN |
		   RK3328_PDATA_EN);

	/* Disable phy irq */
	inno_write(inno, 0x05, 0);
	inno_write(inno, 0x07, 0);

	if (plat_ops->clk_recalc_rate)
		plat_ops->clk_recalc_rate(phy, clk_get_rate(&inno->refoclk));

	if (plat_ops->clk_round_rate)
		plat_ops->clk_round_rate(phy, inno->pixclock);
}

static const struct inno_hdmi_phy_plat_ops rk3328_hdmi_phy_plat_ops = {
	.init = inno_hdmi_phy_rk3328_init,
	.power_on = inno_hdmi_phy_rk3328_power_on,
	.power_off = inno_hdmi_phy_rk3328_power_off,
	.clk_enable = inno_hdmi_phy_rk3328_clk_enable,
	.clk_disable = inno_hdmi_phy_rk3328_clk_disable,
	.clk_recalc_rate = inno_hdmi_phy_rk3328_clk_recalc_rate,
	.clk_round_rate = inno_hdmi_phy_rk3328_clk_round_rate,
	.clk_set_rate = inno_hdmi_phy_rk3328_clk_set_rate,
};

static int inno_hdmi_phy_power_on(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);
	const struct post_pll_config *cfg = post_pll_cfg_table;
	const struct phy_config *phy_cfg = inno->data->phy_cfg_table;
	u32 tmdsclock = inno_hdmi_phy_get_tmdsclk(inno, inno->pixclock);
	const struct inno_hdmi_phy_plat_ops *plat_ops = inno->data->plat_ops;
	int ret;

	if (!tmdsclock) {
		dev_err(phy->dev, "TMDS clock is zero!\n");
		return -EINVAL;
	}

	if (!plat_ops->power_on)
		return -EINVAL;

	dev_info(phy->dev, "TMDS clock = %d\n", tmdsclock);

	for (; cfg->tmdsclock != ~0UL; cfg++)
		if (tmdsclock <= cfg->tmdsclock)
			break;

	for (; phy_cfg->tmdsclock != ~0UL; phy_cfg++)
		if (tmdsclock <= phy_cfg->tmdsclock)
			break;

	if (cfg->tmdsclock == 0 || phy_cfg->tmdsclock == 0)
		return -EINVAL;

	if (plat_ops->clk_set_rate) {
		ret = plat_ops->clk_set_rate(phy, inno->pixclock, 24000000);
		if (ret)
			return ret;
	}

	if (plat_ops->clk_enable)
		plat_ops->clk_enable(phy);

	if (plat_ops->power_on) {
		ret = plat_ops->power_on(phy, cfg, phy_cfg);
		if (ret) {
			if (plat_ops->clk_disable)
				plat_ops->clk_disable(phy);
			return ret;
		}
	}

	return 0;
}

static int inno_hdmi_phy_power_off(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);
	const struct inno_hdmi_phy_plat_ops *plat_ops = inno->data->plat_ops;

	if (!plat_ops->power_off)
		return -EINVAL;

	plat_ops->power_off(phy);

	if (plat_ops->clk_disable)
		plat_ops->clk_disable(phy);

	inno->tmdsclock = 0;

	return 0;
}

static int inno_hdmi_phy_init(struct phy *phy)
{
	struct inno_hdmi_phy *inno = dev_get_priv(phy->dev);

	if (inno->data->plat_ops->init)
		inno->data->plat_ops->init(phy);

	return 0;
}

static struct phy_ops inno_hdmi_phy_ops = {
	.init = inno_hdmi_phy_init,
	.power_on = inno_hdmi_phy_power_on,
	.power_off = inno_hdmi_phy_power_off,
};

static int inno_hdmi_phy_probe(struct udevice *dev)
{
	struct inno_hdmi_phy *inno = dev_get_priv(dev);
	int ret;

	inno->regs = dev_read_addr_ptr(dev);
	if (!inno->regs)
		return -ENOMEM;

	inno->data = (const struct inno_hdmi_phy_data *)dev_get_driver_data(dev);
	if (!inno->data)
		return -EINVAL;

	inno->bus_width = 8;

	ret = clk_get_by_name(dev, "refoclk", &inno->refoclk);
	if (ret) {
		dev_err(dev, "failed to get the refoclk (ret=%d)\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "sysclk", &inno->sysclk);
	if (ret) {
		dev_err(dev, "failed to get the sysclk (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

static const struct inno_hdmi_phy_data rk3328_inno_hdmi_phy_drv_data = {
	.phy_type = INNO_HDMI_PHY_RK3328,
	.plat_ops = &rk3328_hdmi_phy_plat_ops,
	.phy_cfg_table = rk3328_phy_cfg,
};

static const struct udevice_id inno_hdmi_phy_ids[] = {
	{
		.compatible = "rockchip,rk3328-hdmi-phy",
		.data = (ulong)&rk3328_inno_hdmi_phy_drv_data,
	},
	{ /* sentile */ }
};

U_BOOT_DRIVER(inno_hdmi_phy) = {
	.name = "inno_hdmi_phy",
	.id = UCLASS_PHY,
	.of_match = inno_hdmi_phy_ids,
	.ops = &inno_hdmi_phy_ops,
	.probe = inno_hdmi_phy_probe,
	.priv_auto = sizeof(struct inno_hdmi_phy),
};
