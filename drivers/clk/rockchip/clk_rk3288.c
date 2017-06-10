/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <mapmem.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/hardware.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <linux/log2.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk3288_clk_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3288_cru dtd;
#endif
};

struct pll_div {
	u32 nr;
	u32 nf;
	u32 no;
};

enum {
	VCO_MAX_HZ	= 2200U * 1000000,
	VCO_MIN_HZ	= 440 * 1000000,
	OUTPUT_MAX_HZ	= 2200U * 1000000,
	OUTPUT_MIN_HZ	= 27500000,
	FREF_MAX_HZ	= 2200U * 1000000,
	FREF_MIN_HZ	= 269 * 1000,
};

enum {
	/* PLL CON0 */
	PLL_OD_MASK		= 0x0f,

	/* PLL CON1 */
	PLL_NF_MASK		= 0x1fff,

	/* PLL CON2 */
	PLL_BWADJ_MASK		= 0x0fff,

	/* PLL CON3 */
	PLL_RESET_SHIFT		= 5,

	/* CLKSEL0 */
	CORE_SEL_PLL_SHIFT	= 15,
	CORE_SEL_PLL_MASK	= 1 << CORE_SEL_PLL_SHIFT,
	A17_DIV_SHIFT		= 8,
	A17_DIV_MASK		= 0x1f << A17_DIV_SHIFT,
	MP_DIV_SHIFT		= 4,
	MP_DIV_MASK		= 0xf << MP_DIV_SHIFT,
	M0_DIV_SHIFT		= 0,
	M0_DIV_MASK		= 0xf << M0_DIV_SHIFT,

	/* CLKSEL1: pd bus clk pll sel: codec or general */
	PD_BUS_SEL_PLL_MASK	= 15,
	PD_BUS_SEL_CPLL		= 0,
	PD_BUS_SEL_GPLL,

	/* pd bus pclk div: pclk = pd_bus_aclk /(div + 1) */
	PD_BUS_PCLK_DIV_SHIFT	= 12,
	PD_BUS_PCLK_DIV_MASK	= 7 << PD_BUS_PCLK_DIV_SHIFT,

	/* pd bus hclk div: aclk_bus: hclk_bus = 1:1 or 2:1 or 4:1 */
	PD_BUS_HCLK_DIV_SHIFT	= 8,
	PD_BUS_HCLK_DIV_MASK	= 3 << PD_BUS_HCLK_DIV_SHIFT,

	/* pd bus aclk div: pd_bus_aclk = pd_bus_src_clk /(div0 * div1) */
	PD_BUS_ACLK_DIV0_SHIFT	= 3,
	PD_BUS_ACLK_DIV0_MASK	= 0x1f << PD_BUS_ACLK_DIV0_SHIFT,
	PD_BUS_ACLK_DIV1_SHIFT	= 0,
	PD_BUS_ACLK_DIV1_MASK	= 0x7 << PD_BUS_ACLK_DIV1_SHIFT,

	/*
	 * CLKSEL10
	 * peripheral bus pclk div:
	 * aclk_bus: pclk_bus = 1:1 or 2:1 or 4:1 or 8:1
	 */
	PERI_SEL_PLL_SHIFT	 = 15,
	PERI_SEL_PLL_MASK	 = 1 << PERI_SEL_PLL_SHIFT,
	PERI_SEL_CPLL		= 0,
	PERI_SEL_GPLL,

	PERI_PCLK_DIV_SHIFT	= 12,
	PERI_PCLK_DIV_MASK	= 3 << PERI_PCLK_DIV_SHIFT,

	/* peripheral bus hclk div: aclk_bus: hclk_bus = 1:1 or 2:1 or 4:1 */
	PERI_HCLK_DIV_SHIFT	= 8,
	PERI_HCLK_DIV_MASK	= 3 << PERI_HCLK_DIV_SHIFT,

	/*
	 * peripheral bus aclk div:
	 *    aclk_periph = periph_clk_src / (peri_aclk_div_con + 1)
	 */
	PERI_ACLK_DIV_SHIFT	= 0,
	PERI_ACLK_DIV_MASK	= 0x1f << PERI_ACLK_DIV_SHIFT,

	SOCSTS_DPLL_LOCK	= 1 << 5,
	SOCSTS_APLL_LOCK	= 1 << 6,
	SOCSTS_CPLL_LOCK	= 1 << 7,
	SOCSTS_GPLL_LOCK	= 1 << 8,
	SOCSTS_NPLL_LOCK	= 1 << 9,
};

#define RATE_TO_DIV(input_rate, output_rate) \
	((input_rate) / (output_rate) - 1);

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _nr, _no) {\
	.nr = _nr, .nf = (u32)((u64)hz * _nr * _no / OSC_HZ), .no = _no};\
	_Static_assert(((u64)hz * _nr * _no / OSC_HZ) * OSC_HZ /\
		       (_nr * _no) == hz, #hz "Hz cannot be hit with PLL "\
		       "divisors on line " __stringify(__LINE__));

/* Keep divisors as low as possible to reduce jitter and power usage */
static const struct pll_div apll_init_cfg = PLL_DIVISORS(APLL_HZ, 1, 1);
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 2, 2);
static const struct pll_div cpll_init_cfg = PLL_DIVISORS(CPLL_HZ, 1, 2);

static int rkclk_set_pll(struct rk3288_cru *cru, enum rk_clk_id clk_id,
			 const struct pll_div *div)
{
	int pll_id = rk_pll_id(clk_id);
	struct rk3288_pll *pll = &cru->pll[pll_id];
	/* All PLLs have same VCO and output frequency range restrictions. */
	uint vco_hz = OSC_HZ / 1000 * div->nf / div->nr * 1000;
	uint output_hz = vco_hz / div->no;

	debug("PLL at %x: nf=%d, nr=%d, no=%d, vco=%u Hz, output=%u Hz\n",
	      (uint)pll, div->nf, div->nr, div->no, vco_hz, output_hz);
	assert(vco_hz >= VCO_MIN_HZ && vco_hz <= VCO_MAX_HZ &&
	       output_hz >= OUTPUT_MIN_HZ && output_hz <= OUTPUT_MAX_HZ &&
	       (div->no == 1 || !(div->no % 2)));

	/* enter reset */
	rk_setreg(&pll->con3, 1 << PLL_RESET_SHIFT);

	rk_clrsetreg(&pll->con0, CLKR_MASK | PLL_OD_MASK,
		     ((div->nr - 1) << CLKR_SHIFT) | (div->no - 1));
	rk_clrsetreg(&pll->con1, CLKF_MASK, div->nf - 1);
	rk_clrsetreg(&pll->con2, PLL_BWADJ_MASK, (div->nf >> 1) - 1);

	udelay(10);

	/* return from reset */
	rk_clrreg(&pll->con3, 1 << PLL_RESET_SHIFT);

	return 0;
}

static int rkclk_configure_ddr(struct rk3288_cru *cru, struct rk3288_grf *grf,
			       unsigned int hz)
{
	static const struct pll_div dpll_cfg[] = {
		{.nf = 25, .nr = 2, .no = 1},
		{.nf = 400, .nr = 9, .no = 2},
		{.nf = 500, .nr = 9, .no = 2},
		{.nf = 100, .nr = 3, .no = 1},
	};
	int cfg;

	switch (hz) {
	case 300000000:
		cfg = 0;
		break;
	case 533000000:	/* actually 533.3P MHz */
		cfg = 1;
		break;
	case 666000000:	/* actually 666.6P MHz */
		cfg = 2;
		break;
	case 800000000:
		cfg = 3;
		break;
	default:
		debug("Unsupported SDRAM frequency");
		return -EINVAL;
	}

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK,
		     DPLL_MODE_SLOW << DPLL_MODE_SHIFT);

	rkclk_set_pll(cru, CLK_DDR, &dpll_cfg[cfg]);

	/* wait for pll lock */
	while (!(readl(&grf->soc_status[1]) & SOCSTS_DPLL_LOCK))
		udelay(1);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK,
		     DPLL_MODE_NORMAL << DPLL_MODE_SHIFT);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
#define VCO_MAX_KHZ	2200000
#define VCO_MIN_KHZ	440000
#define FREF_MAX_KHZ	2200000
#define FREF_MIN_KHZ	269

static int pll_para_config(ulong freq_hz, struct pll_div *div, uint *ext_div)
{
	uint ref_khz = OSC_HZ / 1000, nr, nf = 0;
	uint fref_khz;
	uint diff_khz, best_diff_khz;
	const uint max_nr = 1 << 6, max_nf = 1 << 12, max_no = 1 << 4;
	uint vco_khz;
	uint no = 1;
	uint freq_khz = freq_hz / 1000;

	if (!freq_hz) {
		printf("%s: the frequency can not be 0 Hz\n", __func__);
		return -EINVAL;
	}

	no = DIV_ROUND_UP(VCO_MIN_KHZ, freq_khz);
	if (ext_div) {
		*ext_div = DIV_ROUND_UP(no, max_no);
		no = DIV_ROUND_UP(no, *ext_div);
	}

	/* only even divisors (and 1) are supported */
	if (no > 1)
		no = DIV_ROUND_UP(no, 2) * 2;

	vco_khz = freq_khz * no;
	if (ext_div)
		vco_khz *= *ext_div;

	if (vco_khz < VCO_MIN_KHZ || vco_khz > VCO_MAX_KHZ || no > max_no) {
		printf("%s: Cannot find out a supported VCO for Frequency (%luHz).\n",
		       __func__, freq_hz);
		return -1;
	}

	div->no = no;

	best_diff_khz = vco_khz;
	for (nr = 1; nr < max_nr && best_diff_khz; nr++) {
		fref_khz = ref_khz / nr;
		if (fref_khz < FREF_MIN_KHZ)
			break;
		if (fref_khz > FREF_MAX_KHZ)
			continue;

		nf = vco_khz / fref_khz;
		if (nf >= max_nf)
			continue;
		diff_khz = vco_khz - nf * fref_khz;
		if (nf + 1 < max_nf && diff_khz > fref_khz / 2) {
			nf++;
			diff_khz = fref_khz - diff_khz;
		}

		if (diff_khz >= best_diff_khz)
			continue;

		best_diff_khz = diff_khz;
		div->nr = nr;
		div->nf = nf;
	}

	if (best_diff_khz > 4 * 1000) {
		printf("%s: Failed to match output frequency %lu, difference is %u Hz, exceed 4MHZ\n",
		       __func__, freq_hz, best_diff_khz * 1000);
		return -EINVAL;
	}

	return 0;
}

static int rockchip_mac_set_clk(struct rk3288_cru *cru,
				  int periph, uint freq)
{
	/* Assuming mac_clk is fed by an external clock */
	rk_clrsetreg(&cru->cru_clksel_con[21],
		     RMII_EXTCLK_MASK,
		     RMII_EXTCLK_SELECT_EXT_CLK << RMII_EXTCLK_SHIFT);

	 return 0;
}

static int rockchip_vop_set_clk(struct rk3288_cru *cru, struct rk3288_grf *grf,
				int periph, unsigned int rate_hz)
{
	struct pll_div npll_config = {0};
	u32 lcdc_div;
	int ret;

	ret = pll_para_config(rate_hz, &npll_config, &lcdc_div);
	if (ret)
		return ret;

	rk_clrsetreg(&cru->cru_mode_con, NPLL_MODE_MASK,
		     NPLL_MODE_SLOW << NPLL_MODE_SHIFT);
	rkclk_set_pll(cru, CLK_NEW, &npll_config);

	/* waiting for pll lock */
	while (1) {
		if (readl(&grf->soc_status[1]) & SOCSTS_NPLL_LOCK)
			break;
		udelay(1);
	}

	rk_clrsetreg(&cru->cru_mode_con, NPLL_MODE_MASK,
		     NPLL_MODE_NORMAL << NPLL_MODE_SHIFT);

	/* vop dclk source clk: npll,dclk_div: 1 */
	switch (periph) {
	case DCLK_VOP0:
		rk_clrsetreg(&cru->cru_clksel_con[27], 0xff << 8 | 3 << 0,
			     (lcdc_div - 1) << 8 | 2 << 0);
		break;
	case DCLK_VOP1:
		rk_clrsetreg(&cru->cru_clksel_con[29], 0xff << 8 | 3 << 6,
			     (lcdc_div - 1) << 8 | 2 << 6);
		break;
	}

	return 0;
}
#endif /* CONFIG_SPL_BUILD */

static void rkclk_init(struct rk3288_cru *cru, struct rk3288_grf *grf)
{
	u32 aclk_div;
	u32 hclk_div;
	u32 pclk_div;

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK | CPLL_MODE_MASK,
		     GPLL_MODE_SLOW << GPLL_MODE_SHIFT |
		     CPLL_MODE_SLOW << CPLL_MODE_SHIFT);

	/* init pll */
	rkclk_set_pll(cru, CLK_GENERAL, &gpll_init_cfg);
	rkclk_set_pll(cru, CLK_CODEC, &cpll_init_cfg);

	/* waiting for pll lock */
	while ((readl(&grf->soc_status[1]) &
			(SOCSTS_CPLL_LOCK | SOCSTS_GPLL_LOCK)) !=
			(SOCSTS_CPLL_LOCK | SOCSTS_GPLL_LOCK))
		udelay(1);

	/*
	 * pd_bus clock pll source selection and
	 * set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = GPLL_HZ / PD_BUS_ACLK_HZ - 1;
	assert((aclk_div + 1) * PD_BUS_ACLK_HZ == GPLL_HZ && aclk_div < 0x1f);
	hclk_div = PD_BUS_ACLK_HZ / PD_BUS_HCLK_HZ - 1;
	assert((hclk_div + 1) * PD_BUS_HCLK_HZ ==
		PD_BUS_ACLK_HZ && (hclk_div < 0x4) && (hclk_div != 0x2));

	pclk_div = PD_BUS_ACLK_HZ / PD_BUS_PCLK_HZ - 1;
	assert((pclk_div + 1) * PD_BUS_PCLK_HZ ==
		PD_BUS_ACLK_HZ && pclk_div < 0x7);

	rk_clrsetreg(&cru->cru_clksel_con[1],
		     PD_BUS_PCLK_DIV_MASK | PD_BUS_HCLK_DIV_MASK |
		     PD_BUS_ACLK_DIV0_MASK | PD_BUS_ACLK_DIV1_MASK,
		     pclk_div << PD_BUS_PCLK_DIV_SHIFT |
		     hclk_div << PD_BUS_HCLK_DIV_SHIFT |
		     aclk_div << PD_BUS_ACLK_DIV0_SHIFT |
		     0 << 0);

	/*
	 * peri clock pll source selection and
	 * set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = GPLL_HZ / PERI_ACLK_HZ - 1;
	assert((aclk_div + 1) * PERI_ACLK_HZ == GPLL_HZ && aclk_div < 0x1f);

	hclk_div = ilog2(PERI_ACLK_HZ / PERI_HCLK_HZ);
	assert((1 << hclk_div) * PERI_HCLK_HZ ==
		PERI_ACLK_HZ && (hclk_div < 0x4));

	pclk_div = ilog2(PERI_ACLK_HZ / PERI_PCLK_HZ);
	assert((1 << pclk_div) * PERI_PCLK_HZ ==
		PERI_ACLK_HZ && (pclk_div < 0x4));

	rk_clrsetreg(&cru->cru_clksel_con[10],
		     PERI_PCLK_DIV_MASK | PERI_HCLK_DIV_MASK |
		     PERI_ACLK_DIV_MASK,
		     PERI_SEL_GPLL << PERI_SEL_PLL_SHIFT |
		     pclk_div << PERI_PCLK_DIV_SHIFT |
		     hclk_div << PERI_HCLK_DIV_SHIFT |
		     aclk_div << PERI_ACLK_DIV_SHIFT);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK | CPLL_MODE_MASK,
		     GPLL_MODE_NORMAL << GPLL_MODE_SHIFT |
		     CPLL_MODE_NORMAL << CPLL_MODE_SHIFT);
}

void rk3288_clk_configure_cpu(struct rk3288_cru *cru, struct rk3288_grf *grf)
{
	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con, APLL_MODE_MASK,
		     APLL_MODE_SLOW << APLL_MODE_SHIFT);

	rkclk_set_pll(cru, CLK_ARM, &apll_init_cfg);

	/* waiting for pll lock */
	while (!(readl(&grf->soc_status[1]) & SOCSTS_APLL_LOCK))
		udelay(1);

	/*
	 * core clock pll source selection and
	 * set up dependent divisors for MPAXI/M0AXI and ARM clocks.
	 * core clock select apll, apll clk = 1800MHz
	 * arm clk = 1800MHz, mpclk = 450MHz, m0clk = 900MHz
	 */
	rk_clrsetreg(&cru->cru_clksel_con[0],
		     CORE_SEL_PLL_MASK | A17_DIV_MASK | MP_DIV_MASK |
		     M0_DIV_MASK,
		     0 << A17_DIV_SHIFT |
		     3 << MP_DIV_SHIFT |
		     1 << M0_DIV_SHIFT);

	/*
	 * set up dependent divisors for L2RAM/ATCLK and PCLK clocks.
	 * l2ramclk = 900MHz, atclk = 450MHz, pclk_dbg = 450MHz
	 */
	rk_clrsetreg(&cru->cru_clksel_con[37],
		     CLK_L2RAM_DIV_MASK | ATCLK_CORE_DIV_CON_MASK |
		     PCLK_CORE_DBG_DIV_MASK,
		     1 << CLK_L2RAM_DIV_SHIFT |
		     3 << ATCLK_CORE_DIV_CON_SHIFT |
		     3 << PCLK_CORE_DBG_DIV_SHIFT);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con, APLL_MODE_MASK,
		     APLL_MODE_NORMAL << APLL_MODE_SHIFT);
}

/* Get pll rate by id */
static uint32_t rkclk_pll_get_rate(struct rk3288_cru *cru,
				   enum rk_clk_id clk_id)
{
	uint32_t nr, no, nf;
	uint32_t con;
	int pll_id = rk_pll_id(clk_id);
	struct rk3288_pll *pll = &cru->pll[pll_id];
	static u8 clk_shift[CLK_COUNT] = {
		0xff, APLL_MODE_SHIFT, DPLL_MODE_SHIFT, CPLL_MODE_SHIFT,
		GPLL_MODE_SHIFT, NPLL_MODE_SHIFT
	};
	uint shift;

	con = readl(&cru->cru_mode_con);
	shift = clk_shift[clk_id];
	switch ((con >> shift) & CRU_MODE_MASK) {
	case APLL_MODE_SLOW:
		return OSC_HZ;
	case APLL_MODE_NORMAL:
		/* normal mode */
		con = readl(&pll->con0);
		no = ((con & CLKOD_MASK) >> CLKOD_SHIFT) + 1;
		nr = ((con & CLKR_MASK) >> CLKR_SHIFT) + 1;
		con = readl(&pll->con1);
		nf = ((con & CLKF_MASK) >> CLKF_SHIFT) + 1;

		return (24 * nf / (nr * no)) * 1000000;
	case APLL_MODE_DEEP:
	default:
		return 32768;
	}
}

static ulong rockchip_mmc_get_clk(struct rk3288_cru *cru, uint gclk_rate,
				  int periph)
{
	uint src_rate;
	uint div, mux;
	u32 con;

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
		con = readl(&cru->cru_clksel_con[12]);
		mux = (con & EMMC_PLL_MASK) >> EMMC_PLL_SHIFT;
		div = (con & EMMC_DIV_MASK) >> EMMC_DIV_SHIFT;
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		con = readl(&cru->cru_clksel_con[11]);
		mux = (con & MMC0_PLL_MASK) >> MMC0_PLL_SHIFT;
		div = (con & MMC0_DIV_MASK) >> MMC0_DIV_SHIFT;
		break;
	case HCLK_SDIO0:
	case SCLK_SDIO0:
		con = readl(&cru->cru_clksel_con[12]);
		mux = (con & SDIO0_PLL_MASK) >> SDIO0_PLL_SHIFT;
		div = (con & SDIO0_DIV_MASK) >> SDIO0_DIV_SHIFT;
		break;
	default:
		return -EINVAL;
	}

	src_rate = mux == EMMC_PLL_SELECT_24MHZ ? OSC_HZ : gclk_rate;
	return DIV_TO_RATE(src_rate, div);
}

static ulong rockchip_mmc_set_clk(struct rk3288_cru *cru, uint gclk_rate,
				  int  periph, uint freq)
{
	int src_clk_div;
	int mux;

	debug("%s: gclk_rate=%u\n", __func__, gclk_rate);
	src_clk_div = RATE_TO_DIV(gclk_rate, freq);

	if (src_clk_div > 0x3f) {
		src_clk_div = RATE_TO_DIV(OSC_HZ, freq);
		mux = EMMC_PLL_SELECT_24MHZ;
		assert((int)EMMC_PLL_SELECT_24MHZ ==
		       (int)MMC0_PLL_SELECT_24MHZ);
	} else {
		mux = EMMC_PLL_SELECT_GENERAL;
		assert((int)EMMC_PLL_SELECT_GENERAL ==
		       (int)MMC0_PLL_SELECT_GENERAL);
	}
	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     EMMC_PLL_MASK | EMMC_DIV_MASK,
			     mux << EMMC_PLL_SHIFT |
			     (src_clk_div - 1) << EMMC_DIV_SHIFT);
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		rk_clrsetreg(&cru->cru_clksel_con[11],
			     MMC0_PLL_MASK | MMC0_DIV_MASK,
			     mux << MMC0_PLL_SHIFT |
			     (src_clk_div - 1) << MMC0_DIV_SHIFT);
		break;
	case HCLK_SDIO0:
	case SCLK_SDIO0:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     SDIO0_PLL_MASK | SDIO0_DIV_MASK,
			     mux << SDIO0_PLL_SHIFT |
			     (src_clk_div - 1) << SDIO0_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_mmc_get_clk(cru, gclk_rate, periph);
}

static ulong rockchip_spi_get_clk(struct rk3288_cru *cru, uint gclk_rate,
				  int periph)
{
	uint div, mux;
	u32 con;

	switch (periph) {
	case SCLK_SPI0:
		con = readl(&cru->cru_clksel_con[25]);
		mux = (con & SPI0_PLL_MASK) >> SPI0_PLL_SHIFT;
		div = (con & SPI0_DIV_MASK) >> SPI0_DIV_SHIFT;
		break;
	case SCLK_SPI1:
		con = readl(&cru->cru_clksel_con[25]);
		mux = (con & SPI1_PLL_MASK) >> SPI1_PLL_SHIFT;
		div = (con & SPI1_DIV_MASK) >> SPI1_DIV_SHIFT;
		break;
	case SCLK_SPI2:
		con = readl(&cru->cru_clksel_con[39]);
		mux = (con & SPI2_PLL_MASK) >> SPI2_PLL_SHIFT;
		div = (con & SPI2_DIV_MASK) >> SPI2_DIV_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	assert(mux == SPI0_PLL_SELECT_GENERAL);

	return DIV_TO_RATE(gclk_rate, div);
}

static ulong rockchip_spi_set_clk(struct rk3288_cru *cru, uint gclk_rate,
				  int periph, uint freq)
{
	int src_clk_div;

	debug("%s: clk_general_rate=%u\n", __func__, gclk_rate);
	src_clk_div = RATE_TO_DIV(gclk_rate, freq);
	switch (periph) {
	case SCLK_SPI0:
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI0_PLL_MASK | SPI0_DIV_MASK,
			     SPI0_PLL_SELECT_GENERAL << SPI0_PLL_SHIFT |
			     src_clk_div << SPI0_DIV_SHIFT);
		break;
	case SCLK_SPI1:
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI1_PLL_MASK | SPI1_DIV_MASK,
			     SPI1_PLL_SELECT_GENERAL << SPI1_PLL_SHIFT |
			     src_clk_div << SPI1_DIV_SHIFT);
		break;
	case SCLK_SPI2:
		rk_clrsetreg(&cru->cru_clksel_con[39],
			     SPI2_PLL_MASK | SPI2_DIV_MASK,
			     SPI2_PLL_SELECT_GENERAL << SPI2_PLL_SHIFT |
			     src_clk_div << SPI2_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_spi_get_clk(cru, gclk_rate, periph);
}

static ulong rk3288_clk_get_rate(struct clk *clk)
{
	struct rk3288_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate, gclk_rate;

	gclk_rate = rkclk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case 0 ... 63:
		new_rate = rkclk_pll_get_rate(priv->cru, clk->id);
		break;
	case HCLK_EMMC:
	case HCLK_SDMMC:
	case HCLK_SDIO0:
	case SCLK_EMMC:
	case SCLK_SDMMC:
	case SCLK_SDIO0:
		new_rate = rockchip_mmc_get_clk(priv->cru, gclk_rate, clk->id);
		break;
	case SCLK_SPI0:
	case SCLK_SPI1:
	case SCLK_SPI2:
		new_rate = rockchip_spi_get_clk(priv->cru, gclk_rate, clk->id);
		break;
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_I2C4:
	case PCLK_I2C5:
		return gclk_rate;
	case PCLK_PWM:
		return PD_BUS_PCLK_HZ;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static ulong rk3288_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3288_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3288_cru *cru = priv->cru;
	ulong new_rate, gclk_rate;

	gclk_rate = rkclk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case PLL_APLL:
		/* We only support a fixed rate here */
		if (rate != 1800000000)
			return -EINVAL;
		rk3288_clk_configure_cpu(priv->cru, priv->grf);
		new_rate = rate;
		break;
	case CLK_DDR:
		new_rate = rkclk_configure_ddr(priv->cru, priv->grf, rate);
		break;
	case HCLK_EMMC:
	case HCLK_SDMMC:
	case HCLK_SDIO0:
	case SCLK_EMMC:
	case SCLK_SDMMC:
	case SCLK_SDIO0:
		new_rate = rockchip_mmc_set_clk(cru, gclk_rate, clk->id, rate);
		break;
	case SCLK_SPI0:
	case SCLK_SPI1:
	case SCLK_SPI2:
		new_rate = rockchip_spi_set_clk(cru, gclk_rate, clk->id, rate);
		break;
#ifndef CONFIG_SPL_BUILD
	case SCLK_MAC:
		new_rate = rockchip_mac_set_clk(priv->cru, clk->id, rate);
		break;
	case DCLK_VOP0:
	case DCLK_VOP1:
		new_rate = rockchip_vop_set_clk(cru, priv->grf, clk->id, rate);
		break;
	case SCLK_EDP_24M:
		/* clk_edp_24M source: 24M */
		rk_setreg(&cru->cru_clksel_con[28], 1 << 15);

		/* rst edp */
		rk_setreg(&cru->cru_clksel_con[6], 1 << 15);
		udelay(1);
		rk_clrreg(&cru->cru_clksel_con[6], 1 << 15);
		new_rate = rate;
		break;
	case ACLK_VOP0:
	case ACLK_VOP1: {
		u32 div;

		/* vop aclk source clk: cpll */
		div = CPLL_HZ / rate;
		assert((div - 1 < 64) && (div * rate == CPLL_HZ));

		switch (clk->id) {
		case ACLK_VOP0:
			rk_clrsetreg(&cru->cru_clksel_con[31],
				     3 << 6 | 0x1f << 0,
				     0 << 6 | (div - 1) << 0);
			break;
		case ACLK_VOP1:
			rk_clrsetreg(&cru->cru_clksel_con[31],
				     3 << 14 | 0x1f << 8,
				     0 << 14 | (div - 1) << 8);
			break;
		}
		new_rate = rate;
		break;
	}
	case PCLK_HDMI_CTRL:
		/* enable pclk hdmi ctrl */
		rk_clrreg(&cru->cru_clkgate_con[16], 1 << 9);

		/* software reset hdmi */
		rk_setreg(&cru->cru_clkgate_con[7], 1 << 9);
		udelay(1);
		rk_clrreg(&cru->cru_clkgate_con[7], 1 << 9);
		new_rate = rate;
		break;
#endif
	default:
		return -ENOENT;
	}

	return new_rate;
}

static struct clk_ops rk3288_clk_ops = {
	.get_rate	= rk3288_clk_get_rate,
	.set_rate	= rk3288_clk_set_rate,
};

static int rk3288_clk_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3288_clk_priv *priv = dev_get_priv(dev);

	priv->cru = (struct rk3288_cru *)devfdt_get_addr(dev);
#endif

	return 0;
}

static int rk3288_clk_probe(struct udevice *dev)
{
	struct rk3288_clk_priv *priv = dev_get_priv(dev);
	bool init_clocks = false;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->grf))
		return PTR_ERR(priv->grf);
#ifdef CONFIG_SPL_BUILD
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3288_clk_plat *plat = dev_get_platdata(dev);

	priv->cru = map_sysmem(plat->dtd.reg[0], plat->dtd.reg[1]);
#endif
	init_clocks = true;
#endif
	if (!(gd->flags & GD_FLG_RELOC)) {
		u32 reg;

		/*
		 * Init clocks in U-Boot proper if the NPLL is runnning. This
		 * indicates that a previous boot loader set up the clocks, so
		 * we need to redo it. U-Boot's SPL does not set this clock.
		 */
		reg = readl(&priv->cru->cru_mode_con);
		if (((reg & NPLL_MODE_MASK) >> NPLL_MODE_SHIFT) ==
				NPLL_MODE_NORMAL)
			init_clocks = true;
	}

	if (init_clocks)
		rkclk_init(priv->cru, priv->grf);

	return 0;
}

static int rk3288_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "rk3288_sysreset", "reset", &dev);
	if (ret)
		debug("Warning: No RK3288 reset driver: ret=%d\n", ret);

	return 0;
}

static const struct udevice_id rk3288_clk_ids[] = {
	{ .compatible = "rockchip,rk3288-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3288_cru) = {
	.name		= "rockchip_rk3288_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3288_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rk3288_clk_priv),
	.platdata_auto_alloc_size = sizeof(struct rk3288_clk_plat),
	.ops		= &rk3288_clk_ops,
	.bind		= rk3288_clk_bind,
	.ofdata_to_platdata	= rk3288_clk_ofdata_to_platdata,
	.probe		= rk3288_clk_probe,
};
