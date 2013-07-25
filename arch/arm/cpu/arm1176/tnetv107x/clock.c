/*
 * TNETV107X: Clock management APIs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm-generic/errno.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/clock.h>

#define CLOCK_BASE		TNETV107X_CLOCK_CONTROL_BASE
#define PSC_BASE		TNETV107X_PSC_BASE

#define BIT(x)			(1 << (x))

#define MAX_PREDIV		64
#define MAX_POSTDIV		8
#define MAX_MULT		512
#define MAX_DIV			(MAX_PREDIV * MAX_POSTDIV)

/* LPSC registers */
#define PSC_PTCMD		0x120
#define PSC_PTSTAT		0x128
#define PSC_MDSTAT(n)		(0x800 + (n) * 4)
#define PSC_MDCTL(n)		(0xA00 + (n) * 4)

#define PSC_MDCTL_LRSTZ		BIT(8)

#define psc_reg_read(reg)	__raw_readl((u32 *)(PSC_BASE + (reg)))
#define psc_reg_write(reg, val)	__raw_writel(val, (u32 *)(PSC_BASE + (reg)))

/* SSPLL registers */
struct sspll_regs {
	u32	modes;
	u32	postdiv;
	u32	prediv;
	u32	mult_factor;
	u32	divider_range;
	u32	bw_divider;
	u32	spr_amount;
	u32	spr_rate_div;
	u32	diag;
};

/* SSPLL base addresses */
static struct sspll_regs *sspll_regs[] = {
	(struct sspll_regs *)(CLOCK_BASE + 0x040),
	(struct sspll_regs *)(CLOCK_BASE + 0x080),
	(struct sspll_regs *)(CLOCK_BASE + 0x0c0),
};

#define sspll_reg(pll, reg)		(&(sspll_regs[pll]->reg))
#define sspll_reg_read(pll, reg)	__raw_readl(sspll_reg(pll, reg))
#define sspll_reg_write(pll, reg, val)	__raw_writel(val, sspll_reg(pll, reg))


/* PLL Control Registers */
struct pllctl_regs {
	u32	ctl;		/* 00 */
	u32	ocsel;		/* 04 */
	u32	secctl;		/* 08 */
	u32	__pad0;
	u32	mult;		/* 10 */
	u32	prediv;		/* 14 */
	u32	div1;		/* 18 */
	u32	div2;		/* 1c */
	u32	div3;		/* 20 */
	u32	oscdiv1;	/* 24 */
	u32	postdiv;	/* 28 */
	u32	bpdiv;		/* 2c */
	u32	wakeup;		/* 30 */
	u32	__pad1;
	u32	cmd;		/* 38 */
	u32	stat;		/* 3c */
	u32	alnctl;		/* 40 */
	u32	dchange;	/* 44 */
	u32	cken;		/* 48 */
	u32	ckstat;		/* 4c */
	u32	systat;		/* 50 */
	u32	ckctl;		/* 54 */
	u32	__pad2[2];
	u32	div4;		/* 60 */
	u32	div5;		/* 64 */
	u32	div6;		/* 68 */
	u32	div7;		/* 6c */
	u32	div8;		/* 70 */
};

struct lpsc_map {
	int	pll, div;
};

static struct pllctl_regs *pllctl_regs[] = {
	(struct pllctl_regs *)(CLOCK_BASE + 0x700),
	(struct pllctl_regs *)(CLOCK_BASE + 0x300),
	(struct pllctl_regs *)(CLOCK_BASE + 0x500),
};

#define pllctl_reg(pll, reg)		(&(pllctl_regs[pll]->reg))
#define pllctl_reg_read(pll, reg)	__raw_readl(pllctl_reg(pll, reg))
#define pllctl_reg_write(pll, reg, val)	__raw_writel(val, pllctl_reg(pll, reg))

#define pllctl_reg_rmw(pll, reg, mask, val)			\
	pllctl_reg_write(pll, reg,				\
		(pllctl_reg_read(pll, reg) & ~(mask)) | val)

#define pllctl_reg_setbits(pll, reg, mask)			\
	pllctl_reg_rmw(pll, reg, 0, mask)

#define pllctl_reg_clrbits(pll, reg, mask)			\
	pllctl_reg_rmw(pll, reg, mask, 0)

/* PLLCTL Bits */
#define PLLCTL_CLKMODE		BIT(8)
#define PLLCTL_PLLSELB		BIT(7)
#define PLLCTL_PLLENSRC		BIT(5)
#define PLLCTL_PLLDIS		BIT(4)
#define PLLCTL_PLLRST		BIT(3)
#define PLLCTL_PLLPWRDN		BIT(1)
#define PLLCTL_PLLEN		BIT(0)

#define PLLDIV_ENABLE		BIT(15)

static int pll_div_offset[] = {
#define div_offset(reg)	offsetof(struct pllctl_regs, reg)
	div_offset(div1), div_offset(div2), div_offset(div3),
	div_offset(div4), div_offset(div5), div_offset(div6),
	div_offset(div7), div_offset(div8),
};

static unsigned long pll_bypass_mask[] = { 1, 4, 2 };
static unsigned long pll_div_mask[] = { 0x01ff, 0x00ff, 0x00ff };

/* Mappings from PLL+DIV to subsystem clocks */
#define sys_arm1176_clk		{SYS_PLL, 0}
#define sys_dsp_clk		{SYS_PLL, 1}
#define sys_ddr_clk		{SYS_PLL, 2}
#define sys_full_clk		{SYS_PLL, 3}
#define sys_lcd_clk		{SYS_PLL, 4}
#define sys_vlynq_ref_clk	{SYS_PLL, 5}
#define sys_tsc_clk		{SYS_PLL, 6}
#define sys_half_clk		{SYS_PLL, 7}

#define eth_clk_5		{ETH_PLL, 0}
#define eth_clk_50		{ETH_PLL, 1}
#define eth_clk_125		{ETH_PLL, 2}
#define eth_clk_250		{ETH_PLL, 3}
#define eth_clk_25		{ETH_PLL, 4}

#define tdm_clk			{TDM_PLL, 0}
#define tdm_extra_clk		{TDM_PLL, 1}
#define tdm1_clk		{TDM_PLL, 2}

static const struct lpsc_map lpsc_clk_map[] = {
	[TNETV107X_LPSC_ARM]			= sys_arm1176_clk,
	[TNETV107X_LPSC_GEM]			= sys_dsp_clk,
	[TNETV107X_LPSC_DDR2_PHY]		= sys_ddr_clk,
	[TNETV107X_LPSC_TPCC]			= sys_full_clk,
	[TNETV107X_LPSC_TPTC0]			= sys_full_clk,
	[TNETV107X_LPSC_TPTC1]			= sys_full_clk,
	[TNETV107X_LPSC_RAM]			= sys_full_clk,
	[TNETV107X_LPSC_MBX_LITE]		= sys_arm1176_clk,
	[TNETV107X_LPSC_LCD]			= sys_lcd_clk,
	[TNETV107X_LPSC_ETHSS]			= eth_clk_125,
	[TNETV107X_LPSC_AEMIF]			= sys_full_clk,
	[TNETV107X_LPSC_CHIP_CFG]		= sys_half_clk,
	[TNETV107X_LPSC_TSC]			= sys_tsc_clk,
	[TNETV107X_LPSC_ROM]			= sys_half_clk,
	[TNETV107X_LPSC_UART2]			= sys_half_clk,
	[TNETV107X_LPSC_PKTSEC]			= sys_half_clk,
	[TNETV107X_LPSC_SECCTL]			= sys_half_clk,
	[TNETV107X_LPSC_KEYMGR]			= sys_half_clk,
	[TNETV107X_LPSC_KEYPAD]			= sys_half_clk,
	[TNETV107X_LPSC_GPIO]			= sys_half_clk,
	[TNETV107X_LPSC_MDIO]			= sys_half_clk,
	[TNETV107X_LPSC_SDIO0]			= sys_half_clk,
	[TNETV107X_LPSC_UART0]			= sys_half_clk,
	[TNETV107X_LPSC_UART1]			= sys_half_clk,
	[TNETV107X_LPSC_TIMER0]			= sys_half_clk,
	[TNETV107X_LPSC_TIMER1]			= sys_half_clk,
	[TNETV107X_LPSC_WDT_ARM]		= sys_half_clk,
	[TNETV107X_LPSC_WDT_DSP]		= sys_half_clk,
	[TNETV107X_LPSC_SSP]			= sys_half_clk,
	[TNETV107X_LPSC_TDM0]			= tdm_clk,
	[TNETV107X_LPSC_VLYNQ]			= sys_vlynq_ref_clk,
	[TNETV107X_LPSC_MCDMA]			= sys_half_clk,
	[TNETV107X_LPSC_USB0]			= sys_half_clk,
	[TNETV107X_LPSC_TDM1]			= tdm1_clk,
	[TNETV107X_LPSC_DEBUGSS]		= sys_half_clk,
	[TNETV107X_LPSC_ETHSS_RGMII]		= eth_clk_250,
	[TNETV107X_LPSC_SYSTEM]			= sys_half_clk,
	[TNETV107X_LPSC_IMCOP]			= sys_dsp_clk,
	[TNETV107X_LPSC_SPARE]			= sys_half_clk,
	[TNETV107X_LPSC_SDIO1]			= sys_half_clk,
	[TNETV107X_LPSC_USB1]			= sys_half_clk,
	[TNETV107X_LPSC_USBSS]			= sys_half_clk,
	[TNETV107X_LPSC_DDR2_EMIF1_VRST]	= sys_ddr_clk,
	[TNETV107X_LPSC_DDR2_EMIF2_VCTL_RST]	= sys_ddr_clk,
};

static const unsigned long pll_ext_freq[] = {
	[SYS_PLL] = CONFIG_PLL_SYS_EXT_FREQ,
	[ETH_PLL] = CONFIG_PLL_ETH_EXT_FREQ,
	[TDM_PLL] = CONFIG_PLL_TDM_EXT_FREQ,
};

static unsigned long pll_freq_get(int pll)
{
	unsigned long mult = 1, prediv = 1, postdiv = 1;
	unsigned long ref = CONFIG_SYS_INT_OSC_FREQ;
	unsigned long ret;
	u32 bypass;

	bypass = __raw_readl((u32 *)(CLOCK_BASE));
	if (!(bypass & pll_bypass_mask[pll])) {
		mult	= sspll_reg_read(pll, mult_factor);
		prediv	= sspll_reg_read(pll, prediv) + 1;
		postdiv	= sspll_reg_read(pll, postdiv) + 1;
	}

	if (pllctl_reg_read(pll, ctl) & PLLCTL_CLKMODE)
		ref = pll_ext_freq[pll];

	if (!(pllctl_reg_read(pll, ctl) & PLLCTL_PLLEN))
		return ref;

	ret = (unsigned long)(ref + ((unsigned long long)ref * mult) / 256);
	ret /= (prediv * postdiv);

	return ret;
}

static unsigned long __pll_div_freq_get(int pll, unsigned int fpll,
					int div)
{
	int divider = 1;
	unsigned long divreg;

	divreg = __raw_readl((void *)pllctl_regs[pll] + pll_div_offset[div]);

	if (divreg & PLLDIV_ENABLE)
		divider = (divreg & pll_div_mask[pll]) + 1;

	return fpll / divider;
}

static unsigned long pll_div_freq_get(int pll, int div)
{
	unsigned int fpll = pll_freq_get(pll);

	return __pll_div_freq_get(pll, fpll, div);
}

static void __pll_div_freq_set(int pll, unsigned int fpll, int div,
			       unsigned long hz)
{
	int divider = (fpll / hz - 1);

	divider &= pll_div_mask[pll];
	divider |= PLLDIV_ENABLE;

	__raw_writel(divider, (void *)pllctl_regs[pll] + pll_div_offset[div]);
	pllctl_reg_setbits(pll, alnctl, (1 << div));
	pllctl_reg_setbits(pll, dchange, (1 << div));
}

static unsigned long pll_div_freq_set(int pll, int div, unsigned long hz)
{
	unsigned int fpll = pll_freq_get(pll);

	__pll_div_freq_set(pll, fpll, div, hz);

	pllctl_reg_write(pll, cmd, 1);

	/* Wait until new divider takes effect */
	while (pllctl_reg_read(pll, stat) & 0x01);

	return __pll_div_freq_get(pll, fpll, div);
}

unsigned long clk_get_rate(unsigned int clk)
{
	return pll_div_freq_get(lpsc_clk_map[clk].pll, lpsc_clk_map[clk].div);
}

unsigned long clk_round_rate(unsigned int clk, unsigned long hz)
{
	unsigned long fpll, divider, pll;

	pll = lpsc_clk_map[clk].pll;
	fpll = pll_freq_get(pll);
	divider = (fpll / hz - 1);
	divider &= pll_div_mask[pll];

	return fpll / (divider + 1);
}

int clk_set_rate(unsigned int clk, unsigned long _hz)
{
	unsigned long hz;

	hz = clk_round_rate(clk, _hz);
	if (hz != _hz)
		return -EINVAL;	/* Cannot set to target freq */

	pll_div_freq_set(lpsc_clk_map[clk].pll, lpsc_clk_map[clk].div, hz);
	return 0;
}

void lpsc_control(int mod, unsigned long state, int lrstz)
{
	u32 mdctl;

	mdctl = psc_reg_read(PSC_MDCTL(mod));
	mdctl &= ~0x1f;
	mdctl |= state;

	if (lrstz == 0)
		mdctl &= ~PSC_MDCTL_LRSTZ;
	else if (lrstz == 1)
		mdctl |= PSC_MDCTL_LRSTZ;

	psc_reg_write(PSC_MDCTL(mod), mdctl);

	psc_reg_write(PSC_PTCMD, 1);

	/* wait for power domain transition to end */
	while (psc_reg_read(PSC_PTSTAT) & 1);

	/* Wait for module state change */
	while ((psc_reg_read(PSC_MDSTAT(mod)) & 0x1f) != state);
}

int lpsc_status(unsigned int id)
{
	return psc_reg_read(PSC_MDSTAT(id)) & 0x1f;
}

static void init_pll(const struct pll_init_data *data)
{
	unsigned long fpll;
	unsigned long best_pre = 0, best_post = 0, best_mult = 0;
	unsigned long div, prediv, postdiv, mult;
	unsigned long delta, actual;
	long best_delta = -1;
	int i;
	u32 tmp;

	if (data->pll == SYS_PLL)
		return; /* cannot reconfigure system pll on the fly */

	tmp = pllctl_reg_read(data->pll, ctl);
	if (data->internal_osc) {
		tmp &= ~PLLCTL_CLKMODE;
		fpll = CONFIG_SYS_INT_OSC_FREQ;
	} else {
		tmp |= PLLCTL_CLKMODE;
		fpll = pll_ext_freq[data->pll];
	}
	pllctl_reg_write(data->pll, ctl, tmp);

	mult = data->pll_freq / fpll;
	for (mult = MAX(mult, 1); mult <= MAX_MULT; mult++) {
		div = (fpll * mult) / data->pll_freq;
		if (div < 1 || div > MAX_DIV)
			continue;

		for (postdiv = 1; postdiv <= min(div, MAX_POSTDIV); postdiv++) {
			prediv = div / postdiv;
			if (prediv < 1 || prediv > MAX_PREDIV)
				continue;

			actual = (fpll / prediv) * (mult / postdiv);
			delta = (actual - data->pll_freq);
			if (delta < 0)
				delta = -delta;
			if ((delta < best_delta) || (best_delta == -1)) {
				best_delta = delta;
				best_mult = mult;
				best_pre = prediv;
				best_post = postdiv;
				if (delta == 0)
					goto done;
			}
		}
	}
done:

	if (best_delta == -1) {
		printf("pll cannot derive %lu from %lu\n",
				data->pll_freq, fpll);
		return;
	}

	fpll = fpll * best_mult;
	fpll /= best_pre * best_post;

	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLENSRC);
	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLEN);

	pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLRST);

	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLPWRDN);
	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLDIS);

	sspll_reg_write(data->pll, mult_factor,	(best_mult - 1) << 8);
	sspll_reg_write(data->pll, prediv,	best_pre - 1);
	sspll_reg_write(data->pll, postdiv,	best_post - 1);

	for (i = 0; i < 10; i++)
		if (data->div_freq[i])
			__pll_div_freq_set(data->pll, fpll, i,
					   data->div_freq[i]);

	pllctl_reg_write(data->pll, cmd, 1);

	/* Wait until pll "go" operation completes */
	while (pllctl_reg_read(data->pll, stat) & 0x01);

	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLRST);
	pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLEN);
}

void init_plls(int num_pll, struct pll_init_data *config)
{
	int i;

	for (i = 0; i < num_pll; i++)
		init_pll(&config[i]);
}
