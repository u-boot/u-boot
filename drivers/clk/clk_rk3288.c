/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk3288_clk_plat {
	enum rk_clk_id clk_id;
};

struct rk3288_clk_priv {
	struct rk3288_grf *grf;
	struct rk3288_cru *cru;
	ulong rate;
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
	FREF_MIN_HZ	= 269 * 1000000,
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

	/* CLKSEL1: pd bus clk pll sel: codec or general */
	PD_BUS_SEL_PLL_MASK	= 15,
	PD_BUS_SEL_CPLL		= 0,
	PD_BUS_SEL_GPLL,

	/* pd bus pclk div: pclk = pd_bus_aclk /(div + 1) */
	PD_BUS_PCLK_DIV_SHIFT	= 12,
	PD_BUS_PCLK_DIV_MASK	= 7,

	/* pd bus hclk div: aclk_bus: hclk_bus = 1:1 or 2:1 or 4:1 */
	PD_BUS_HCLK_DIV_SHIFT	= 8,
	PD_BUS_HCLK_DIV_MASK	= 3,

	/* pd bus aclk div: pd_bus_aclk = pd_bus_src_clk /(div0 * div1) */
	PD_BUS_ACLK_DIV0_SHIFT	= 3,
	PD_BUS_ACLK_DIV0_MASK	= 0x1f,
	PD_BUS_ACLK_DIV1_SHIFT	= 0,
	PD_BUS_ACLK_DIV1_MASK	= 0x7,

	/*
	 * CLKSEL10
	 * peripheral bus pclk div:
	 * aclk_bus: pclk_bus = 1:1 or 2:1 or 4:1 or 8:1
	 */
	PERI_PCLK_DIV_SHIFT	= 12,
	PERI_PCLK_DIV_MASK	= 7,

	/* peripheral bus hclk div: aclk_bus: hclk_bus = 1:1 or 2:1 or 4:1 */
	PERI_HCLK_DIV_SHIFT	= 8,
	PERI_HCLK_DIV_MASK	= 3,

	/*
	 * peripheral bus aclk div:
	 *    aclk_periph = periph_clk_src / (peri_aclk_div_con + 1)
	 */
	PERI_ACLK_DIV_SHIFT	= 0,
	PERI_ACLK_DIV_MASK	= 0x1f,

	/* CLKSEL37 */
	DPLL_MODE_MASK		= 0x3,
	DPLL_MODE_SHIFT		= 4,
	DPLL_MODE_SLOW		= 0,
	DPLL_MODE_NORM,

	CPLL_MODE_MASK		= 3,
	CPLL_MODE_SHIFT		= 8,
	CPLL_MODE_SLOW		= 0,
	CPLL_MODE_NORM,

	GPLL_MODE_MASK		= 3,
	GPLL_MODE_SHIFT		= 12,
	GPLL_MODE_SLOW		= 0,
	GPLL_MODE_NORM,

	NPLL_MODE_MASK		= 3,
	NPLL_MODE_SHIFT		= 14,
	NPLL_MODE_SLOW		= 0,
	NPLL_MODE_NORM,

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

	debug("PLL at %p: nf=%d, nr=%d, no=%d, vco=%u Hz, output=%u Hz\n",
	      pll, div->nf, div->nr, div->no, vco_hz, output_hz);
	assert(vco_hz >= VCO_MIN_HZ && vco_hz <= VCO_MAX_HZ &&
	       output_hz >= OUTPUT_MIN_HZ && output_hz <= OUTPUT_MAX_HZ &&
	       (div->no == 1 || !(div->no % 2)));

	/* enter rest */
	rk_setreg(&pll->con3, 1 << PLL_RESET_SHIFT);

	rk_clrsetreg(&pll->con0,
		     CLKR_MASK << CLKR_SHIFT | PLL_OD_MASK,
		     ((div->nr - 1) << CLKR_SHIFT) | (div->no - 1));
	rk_clrsetreg(&pll->con1, CLKF_MASK, div->nf - 1);
	rk_clrsetreg(&pll->con2, PLL_BWADJ_MASK, (div->nf >> 1) - 1);

	udelay(10);

	/* return form rest */
	rk_clrreg(&pll->con3, 1 << PLL_RESET_SHIFT);

	return 0;
}

static inline unsigned int log2(unsigned int value)
{
	return fls(value) - 1;
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

	debug("%s: cru=%p, grf=%p, hz=%u\n", __func__, cru, grf, hz);
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
		debug("Unsupported SDRAM frequency, add to clock.c!");
		return -EINVAL;
	}

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK << DPLL_MODE_SHIFT,
		     DPLL_MODE_SLOW << DPLL_MODE_SHIFT);

	rkclk_set_pll(cru, CLK_DDR, &dpll_cfg[cfg]);

	/* wait for pll lock */
	while (!(readl(&grf->soc_status[1]) & SOCSTS_DPLL_LOCK))
		udelay(1);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK << DPLL_MODE_SHIFT,
		     DPLL_MODE_NORM << DPLL_MODE_SHIFT);

	return 0;
}

#ifdef CONFIG_SPL_BUILD
static void rkclk_init(struct rk3288_cru *cru, struct rk3288_grf *grf)
{
	u32 aclk_div;
	u32 hclk_div;
	u32 pclk_div;

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK << GPLL_MODE_SHIFT |
		     CPLL_MODE_MASK << CPLL_MODE_SHIFT,
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
		     PD_BUS_PCLK_DIV_MASK << PD_BUS_PCLK_DIV_SHIFT |
		     PD_BUS_HCLK_DIV_MASK << PD_BUS_HCLK_DIV_SHIFT |
		     PD_BUS_ACLK_DIV0_MASK << PD_BUS_ACLK_DIV0_SHIFT |
		     PD_BUS_ACLK_DIV1_MASK << PD_BUS_ACLK_DIV1_SHIFT,
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

	hclk_div = log2(PERI_ACLK_HZ / PERI_HCLK_HZ);
	assert((1 << hclk_div) * PERI_HCLK_HZ ==
		PERI_ACLK_HZ && (hclk_div < 0x4));

	pclk_div = log2(PERI_ACLK_HZ / PERI_PCLK_HZ);
	assert((1 << pclk_div) * PERI_PCLK_HZ ==
		PERI_ACLK_HZ && (pclk_div < 0x4));

	rk_clrsetreg(&cru->cru_clksel_con[10],
		     PERI_PCLK_DIV_MASK << PERI_PCLK_DIV_SHIFT |
		     PERI_HCLK_DIV_MASK << PERI_HCLK_DIV_SHIFT |
		     PERI_ACLK_DIV_MASK << PERI_ACLK_DIV_SHIFT,
		     pclk_div << PERI_PCLK_DIV_SHIFT |
		     hclk_div << PERI_HCLK_DIV_SHIFT |
		     aclk_div << PERI_ACLK_DIV_SHIFT);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK << GPLL_MODE_SHIFT |
		     CPLL_MODE_MASK << CPLL_MODE_SHIFT,
		     GPLL_MODE_NORM << GPLL_MODE_SHIFT |
		     GPLL_MODE_NORM << CPLL_MODE_SHIFT);
}
#endif

/* Get pll rate by id */
static uint32_t rkclk_pll_get_rate(struct rk3288_cru *cru,
				   enum rk_clk_id clk_id)
{
	uint32_t nr, no, nf;
	uint32_t con;
	int pll_id = rk_pll_id(clk_id);
	struct rk3288_pll *pll = &cru->pll[pll_id];
	static u8 clk_shift[CLK_COUNT] = {
		0xff, APLL_WORK_SHIFT, DPLL_WORK_SHIFT, CPLL_WORK_SHIFT,
		GPLL_WORK_SHIFT, NPLL_WORK_SHIFT
	};
	uint shift;

	con = readl(&cru->cru_mode_con);
	shift = clk_shift[clk_id];
	switch ((con >> shift) & APLL_WORK_MASK) {
	case APLL_WORK_SLOW:
		return OSC_HZ;
	case APLL_WORK_NORMAL:
		/* normal mode */
		con = readl(&pll->con0);
		no = ((con >> CLKOD_SHIFT) & CLKOD_MASK) + 1;
		nr = ((con >> CLKR_SHIFT) & CLKR_MASK) + 1;
		con = readl(&pll->con1);
		nf = ((con >> CLKF_SHIFT) & CLKF_MASK) + 1;

		return (24 * nf / (nr * no)) * 1000000;
	case APLL_WORK_DEEP:
	default:
		return 32768;
	}
}

static ulong rk3288_clk_get_rate(struct udevice *dev)
{
	struct rk3288_clk_plat *plat = dev_get_platdata(dev);
	struct rk3288_clk_priv *priv = dev_get_priv(dev);

	debug("%s\n", dev->name);
	return rkclk_pll_get_rate(priv->cru, plat->clk_id);
}

static ulong rk3288_clk_set_rate(struct udevice *dev, ulong rate)
{
	struct rk3288_clk_plat *plat = dev_get_platdata(dev);
	struct rk3288_clk_priv *priv = dev_get_priv(dev);

	debug("%s\n", dev->name);
	switch (plat->clk_id) {
	case CLK_DDR:
		rkclk_configure_ddr(priv->cru, priv->grf, rate);
		break;
	default:
		return -ENOENT;
	}

	return 0;
}

static ulong rockchip_mmc_get_clk(struct rk3288_cru *cru, uint clk_general_rate,
				  enum periph_id periph)
{
	uint src_rate;
	uint div, mux;
	u32 con;

	switch (periph) {
	case PERIPH_ID_EMMC:
		con = readl(&cru->cru_clksel_con[12]);
		mux = (con >> EMMC_PLL_SHIFT) & EMMC_PLL_MASK;
		div = (con >> EMMC_DIV_SHIFT) & EMMC_DIV_MASK;
		break;
	case PERIPH_ID_SDCARD:
		con = readl(&cru->cru_clksel_con[12]);
		mux = (con >> MMC0_PLL_SHIFT) & MMC0_PLL_MASK;
		div = (con >> MMC0_DIV_SHIFT) & MMC0_DIV_MASK;
		break;
	case PERIPH_ID_SDMMC2:
		con = readl(&cru->cru_clksel_con[12]);
		mux = (con >> SDIO0_PLL_SHIFT) & SDIO0_PLL_MASK;
		div = (con >> SDIO0_DIV_SHIFT) & SDIO0_DIV_MASK;
		break;
	default:
		return -EINVAL;
	}

	src_rate = mux == EMMC_PLL_SELECT_24MHZ ? OSC_HZ : clk_general_rate;
	return DIV_TO_RATE(src_rate, div);
}

static ulong rockchip_mmc_set_clk(struct rk3288_cru *cru, uint clk_general_rate,
				  enum periph_id periph, uint freq)
{
	int src_clk_div;
	int mux;

	debug("%s: clk_general_rate=%u\n", __func__, clk_general_rate);
	src_clk_div = RATE_TO_DIV(clk_general_rate, freq);

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
	case PERIPH_ID_EMMC:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     EMMC_PLL_MASK << EMMC_PLL_SHIFT |
			     EMMC_DIV_MASK << EMMC_DIV_SHIFT,
			     mux << EMMC_PLL_SHIFT |
			     (src_clk_div - 1) << EMMC_DIV_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		rk_clrsetreg(&cru->cru_clksel_con[11],
			     MMC0_PLL_MASK << MMC0_PLL_SHIFT |
			     MMC0_DIV_MASK << MMC0_DIV_SHIFT,
			     mux << MMC0_PLL_SHIFT |
			     (src_clk_div - 1) << MMC0_DIV_SHIFT);
		break;
	case PERIPH_ID_SDMMC2:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     SDIO0_PLL_MASK << SDIO0_PLL_SHIFT |
			     SDIO0_DIV_MASK << SDIO0_DIV_SHIFT,
			     mux << SDIO0_PLL_SHIFT |
			     (src_clk_div - 1) << SDIO0_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_mmc_get_clk(cru, clk_general_rate, periph);
}

static ulong rockchip_spi_get_clk(struct rk3288_cru *cru, uint clk_general_rate,
				  enum periph_id periph)
{
	uint div, mux;
	u32 con;

	switch (periph) {
	case PERIPH_ID_SPI0:
		con = readl(&cru->cru_clksel_con[25]);
		mux = (con >> SPI0_PLL_SHIFT) & SPI0_PLL_MASK;
		div = (con >> SPI0_DIV_SHIFT) & SPI0_DIV_MASK;
		break;
	case PERIPH_ID_SPI1:
		con = readl(&cru->cru_clksel_con[25]);
		mux = (con >> SPI1_PLL_SHIFT) & SPI1_PLL_MASK;
		div = (con >> SPI1_DIV_SHIFT) & SPI1_DIV_MASK;
		break;
	case PERIPH_ID_SPI2:
		con = readl(&cru->cru_clksel_con[39]);
		mux = (con >> SPI2_PLL_SHIFT) & SPI2_PLL_MASK;
		div = (con >> SPI2_DIV_SHIFT) & SPI2_DIV_MASK;
		break;
	default:
		return -EINVAL;
	}
	assert(mux == SPI0_PLL_SELECT_GENERAL);

	return DIV_TO_RATE(clk_general_rate, div);
}

static ulong rockchip_spi_set_clk(struct rk3288_cru *cru, uint clk_general_rate,
				  enum periph_id periph, uint freq)
{
	int src_clk_div;

	debug("%s: clk_general_rate=%u\n", __func__, clk_general_rate);
	src_clk_div = RATE_TO_DIV(clk_general_rate, freq);
	switch (periph) {
	case PERIPH_ID_SPI0:
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI0_PLL_MASK << SPI0_PLL_SHIFT |
			     SPI0_DIV_MASK << SPI0_DIV_SHIFT,
			     SPI0_PLL_SELECT_GENERAL << SPI0_PLL_SHIFT |
			     src_clk_div << SPI0_DIV_SHIFT);
		break;
	case PERIPH_ID_SPI1:
		rk_clrsetreg(&cru->cru_clksel_con[25],
			     SPI1_PLL_MASK << SPI1_PLL_SHIFT |
			     SPI1_DIV_MASK << SPI1_DIV_SHIFT,
			     SPI1_PLL_SELECT_GENERAL << SPI1_PLL_SHIFT |
			     src_clk_div << SPI1_DIV_SHIFT);
		break;
	case PERIPH_ID_SPI2:
		rk_clrsetreg(&cru->cru_clksel_con[39],
			     SPI2_PLL_MASK << SPI2_PLL_SHIFT |
			     SPI2_DIV_MASK << SPI2_DIV_SHIFT,
			     SPI2_PLL_SELECT_GENERAL << SPI2_PLL_SHIFT |
			     src_clk_div << SPI2_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_spi_get_clk(cru, clk_general_rate, periph);
}

ulong rk3288_set_periph_rate(struct udevice *dev, int periph, ulong rate)
{
	struct rk3288_clk_priv *priv = dev_get_priv(dev);
	ulong new_rate;

	switch (periph) {
	case PERIPH_ID_EMMC:
	case PERIPH_ID_SDCARD:
		new_rate = rockchip_mmc_set_clk(priv->cru, clk_get_rate(dev),
						periph, rate);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
		new_rate = rockchip_spi_set_clk(priv->cru, clk_get_rate(dev),
						periph, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static struct clk_ops rk3288_clk_ops = {
	.get_rate	= rk3288_clk_get_rate,
	.set_rate	= rk3288_clk_set_rate,
	.set_periph_rate = rk3288_set_periph_rate,
};

static int rk3288_clk_probe(struct udevice *dev)
{
	struct rk3288_clk_plat *plat = dev_get_platdata(dev);
	struct rk3288_clk_priv *priv = dev_get_priv(dev);

	if (plat->clk_id != CLK_OSC) {
		struct rk3288_clk_priv *parent_priv = dev_get_priv(dev->parent);

		priv->cru = parent_priv->cru;
		priv->grf = parent_priv->grf;
		return 0;
	}
	priv->cru = (struct rk3288_cru *)dev_get_addr(dev);
	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
#ifdef CONFIG_SPL_BUILD
	rkclk_init(priv->cru, priv->grf);
#endif

	return 0;
}

static const char *const clk_name[CLK_COUNT] = {
	"osc",
	"apll",
	"dpll",
	"cpll",
	"gpll",
	"mpll",
};

static int rk3288_clk_bind(struct udevice *dev)
{
	struct rk3288_clk_plat *plat = dev_get_platdata(dev);
	int pll, ret;

	/* We only need to set up the root clock */
	if (dev->of_offset == -1) {
		plat->clk_id = CLK_OSC;
		return 0;
	}

	/* Create devices for P main clocks */
	for (pll = 1; pll < CLK_COUNT; pll++) {
		struct udevice *child;
		struct rk3288_clk_plat *cplat;

		debug("%s %s\n", __func__, clk_name[pll]);
		ret = device_bind_driver(dev, "clk_rk3288", clk_name[pll],
					 &child);
		if (ret)
			return ret;
		cplat = dev_get_platdata(child);
		cplat->clk_id = pll;
	}

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "rk3288_reset", "reset", &dev);
	if (ret)
		debug("Warning: No RK3288 reset driver: ret=%d\n", ret);

	return 0;
}

static const struct udevice_id rk3288_clk_ids[] = {
	{ .compatible = "rockchip,rk3288-cru" },
	{ }
};

U_BOOT_DRIVER(clk_rk3288) = {
	.name		= "clk_rk3288",
	.id		= UCLASS_CLK,
	.of_match	= rk3288_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rk3288_clk_priv),
	.platdata_auto_alloc_size = sizeof(struct rk3288_clk_plat),
	.ops		= &rk3288_clk_ops,
	.bind		= rk3288_clk_bind,
	.probe		= rk3288_clk_probe,
};
