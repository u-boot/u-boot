/*
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>

/* exynos4: return pll clock frequency */
static unsigned long exynos4_get_pll_clk(int pllreg)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();
	unsigned long r, m, p, s, k = 0, mask, fout;
	unsigned int freq;

	switch (pllreg) {
	case APLL:
		r = readl(&clk->apll_con0);
		break;
	case MPLL:
		r = readl(&clk->mpll_con0);
		break;
	case EPLL:
		r = readl(&clk->epll_con0);
		k = readl(&clk->epll_con1);
		break;
	case VPLL:
		r = readl(&clk->vpll_con0);
		k = readl(&clk->vpll_con1);
		break;
	default:
		printf("Unsupported PLL (%d)\n", pllreg);
		return 0;
	}

	/*
	 * APLL_CON: MIDV [25:16]
	 * MPLL_CON: MIDV [25:16]
	 * EPLL_CON: MIDV [24:16]
	 * VPLL_CON: MIDV [24:16]
	 */
	if (pllreg == APLL || pllreg == MPLL)
		mask = 0x3ff;
	else
		mask = 0x1ff;

	m = (r >> 16) & mask;

	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	freq = CONFIG_SYS_CLK_FREQ;

	if (pllreg == EPLL) {
		k = k & 0xffff;
		/* FOUT = (MDIV + K / 65536) * FIN / (PDIV * 2^SDIV) */
		fout = (m + k / 65536) * (freq / (p * (1 << s)));
	} else if (pllreg == VPLL) {
		k = k & 0xfff;
		/* FOUT = (MDIV + K / 1024) * FIN / (PDIV * 2^SDIV) */
		fout = (m + k / 1024) * (freq / (p * (1 << s)));
	} else {
		if (s < 1)
			s = 1;
		/* FOUT = MDIV * FIN / (PDIV * 2^(SDIV - 1)) */
		fout = m * (freq / (p * (1 << (s - 1))));
	}

	return fout;
}

/* exynos5: return pll clock frequency */
static unsigned long exynos5_get_pll_clk(int pllreg)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long r, m, p, s, k = 0, mask, fout;
	unsigned int freq, pll_div2_sel, fout_sel;

	switch (pllreg) {
	case APLL:
		r = readl(&clk->apll_con0);
		break;
	case MPLL:
		r = readl(&clk->mpll_con0);
		break;
	case EPLL:
		r = readl(&clk->epll_con0);
		k = readl(&clk->epll_con1);
		break;
	case VPLL:
		r = readl(&clk->vpll_con0);
		k = readl(&clk->vpll_con1);
		break;
	case BPLL:
		r = readl(&clk->bpll_con0);
		break;
	default:
		printf("Unsupported PLL (%d)\n", pllreg);
		return 0;
	}

	/*
	 * APLL_CON: MIDV [25:16]
	 * MPLL_CON: MIDV [25:16]
	 * EPLL_CON: MIDV [24:16]
	 * VPLL_CON: MIDV [24:16]
	 * BPLL_CON: MIDV [25:16]
	 */
	if (pllreg == APLL || pllreg == MPLL || pllreg == BPLL)
		mask = 0x3ff;
	else
		mask = 0x1ff;

	m = (r >> 16) & mask;

	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	freq = CONFIG_SYS_CLK_FREQ;

	if (pllreg == EPLL) {
		k = k & 0xffff;
		/* FOUT = (MDIV + K / 65536) * FIN / (PDIV * 2^SDIV) */
		fout = (m + k / 65536) * (freq / (p * (1 << s)));
	} else if (pllreg == VPLL) {
		k = k & 0xfff;
		/* FOUT = (MDIV + K / 1024) * FIN / (PDIV * 2^SDIV) */
		fout = (m + k / 1024) * (freq / (p * (1 << s)));
	} else {
		if (s < 1)
			s = 1;
		/* FOUT = MDIV * FIN / (PDIV * 2^(SDIV - 1)) */
		fout = m * (freq / (p * (1 << (s - 1))));
	}

	/* According to the user manual, in EVT1 MPLL and BPLL always gives
	 * 1.6GHz clock, so divide by 2 to get 800MHz MPLL clock.*/
	if (pllreg == MPLL || pllreg == BPLL) {
		pll_div2_sel = readl(&clk->pll_div2_sel);

		switch (pllreg) {
		case MPLL:
			fout_sel = (pll_div2_sel >> MPLL_FOUT_SEL_SHIFT)
					& MPLL_FOUT_SEL_MASK;
			break;
		case BPLL:
			fout_sel = (pll_div2_sel >> BPLL_FOUT_SEL_SHIFT)
					& BPLL_FOUT_SEL_MASK;
			break;
		default:
			fout_sel = -1;
			break;
		}

		if (fout_sel == 0)
			fout /= 2;
	}

	return fout;
}

/* exynos4: return ARM clock frequency */
static unsigned long exynos4_get_arm_clk(void)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();
	unsigned long div;
	unsigned long armclk;
	unsigned int core_ratio;
	unsigned int core2_ratio;

	div = readl(&clk->div_cpu0);

	/* CORE_RATIO: [2:0], CORE2_RATIO: [30:28] */
	core_ratio = (div >> 0) & 0x7;
	core2_ratio = (div >> 28) & 0x7;

	armclk = get_pll_clk(APLL) / (core_ratio + 1);
	armclk /= (core2_ratio + 1);

	return armclk;
}

/* exynos5: return ARM clock frequency */
static unsigned long exynos5_get_arm_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long div;
	unsigned long armclk;
	unsigned int arm_ratio;
	unsigned int arm2_ratio;

	div = readl(&clk->div_cpu0);

	/* ARM_RATIO: [2:0], ARM2_RATIO: [30:28] */
	arm_ratio = (div >> 0) & 0x7;
	arm2_ratio = (div >> 28) & 0x7;

	armclk = get_pll_clk(APLL) / (arm_ratio + 1);
	armclk /= (arm2_ratio + 1);

	return armclk;
}

/* exynos4: return pwm clock frequency */
static unsigned long exynos4_get_pwm_clk(void)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();
	unsigned long pclk, sclk;
	unsigned int sel;
	unsigned int ratio;

	if (s5p_get_cpu_rev() == 0) {
		/*
		 * CLK_SRC_PERIL0
		 * PWM_SEL [27:24]
		 */
		sel = readl(&clk->src_peril0);
		sel = (sel >> 24) & 0xf;

		if (sel == 0x6)
			sclk = get_pll_clk(MPLL);
		else if (sel == 0x7)
			sclk = get_pll_clk(EPLL);
		else if (sel == 0x8)
			sclk = get_pll_clk(VPLL);
		else
			return 0;

		/*
		 * CLK_DIV_PERIL3
		 * PWM_RATIO [3:0]
		 */
		ratio = readl(&clk->div_peril3);
		ratio = ratio & 0xf;
	} else if (s5p_get_cpu_rev() == 1) {
		sclk = get_pll_clk(MPLL);
		ratio = 8;
	} else
		return 0;

	pclk = sclk / (ratio + 1);

	return pclk;
}

/* exynos5: return pwm clock frequency */
static unsigned long exynos5_get_pwm_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long pclk, sclk;
	unsigned int ratio;

	/*
	 * CLK_DIV_PERIC3
	 * PWM_RATIO [3:0]
	 */
	ratio = readl(&clk->div_peric3);
	ratio = ratio & 0xf;
	sclk = get_pll_clk(MPLL);

	pclk = sclk / (ratio + 1);

	return pclk;
}

/* exynos4: return uart clock frequency */
static unsigned long exynos4_get_uart_clk(int dev_index)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();
	unsigned long uclk, sclk;
	unsigned int sel;
	unsigned int ratio;

	/*
	 * CLK_SRC_PERIL0
	 * UART0_SEL [3:0]
	 * UART1_SEL [7:4]
	 * UART2_SEL [8:11]
	 * UART3_SEL [12:15]
	 * UART4_SEL [16:19]
	 * UART5_SEL [23:20]
	 */
	sel = readl(&clk->src_peril0);
	sel = (sel >> (dev_index << 2)) & 0xf;

	if (sel == 0x6)
		sclk = get_pll_clk(MPLL);
	else if (sel == 0x7)
		sclk = get_pll_clk(EPLL);
	else if (sel == 0x8)
		sclk = get_pll_clk(VPLL);
	else
		return 0;

	/*
	 * CLK_DIV_PERIL0
	 * UART0_RATIO [3:0]
	 * UART1_RATIO [7:4]
	 * UART2_RATIO [8:11]
	 * UART3_RATIO [12:15]
	 * UART4_RATIO [16:19]
	 * UART5_RATIO [23:20]
	 */
	ratio = readl(&clk->div_peril0);
	ratio = (ratio >> (dev_index << 2)) & 0xf;

	uclk = sclk / (ratio + 1);

	return uclk;
}

/* exynos5: return uart clock frequency */
static unsigned long exynos5_get_uart_clk(int dev_index)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long uclk, sclk;
	unsigned int sel;
	unsigned int ratio;

	/*
	 * CLK_SRC_PERIC0
	 * UART0_SEL [3:0]
	 * UART1_SEL [7:4]
	 * UART2_SEL [8:11]
	 * UART3_SEL [12:15]
	 * UART4_SEL [16:19]
	 * UART5_SEL [23:20]
	 */
	sel = readl(&clk->src_peric0);
	sel = (sel >> (dev_index << 2)) & 0xf;

	if (sel == 0x6)
		sclk = get_pll_clk(MPLL);
	else if (sel == 0x7)
		sclk = get_pll_clk(EPLL);
	else if (sel == 0x8)
		sclk = get_pll_clk(VPLL);
	else
		return 0;

	/*
	 * CLK_DIV_PERIC0
	 * UART0_RATIO [3:0]
	 * UART1_RATIO [7:4]
	 * UART2_RATIO [8:11]
	 * UART3_RATIO [12:15]
	 * UART4_RATIO [16:19]
	 * UART5_RATIO [23:20]
	 */
	ratio = readl(&clk->div_peric0);
	ratio = (ratio >> (dev_index << 2)) & 0xf;

	uclk = sclk / (ratio + 1);

	return uclk;
}

/* exynos4: set the mmc clock */
static void exynos4_set_mmc_clk(int dev_index, unsigned int div)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();
	unsigned int addr;
	unsigned int val;

	/*
	 * CLK_DIV_FSYS1
	 * MMC0_PRE_RATIO [15:8], MMC1_PRE_RATIO [31:24]
	 * CLK_DIV_FSYS2
	 * MMC2_PRE_RATIO [15:8], MMC3_PRE_RATIO [31:24]
	 */
	if (dev_index < 2) {
		addr = (unsigned int)&clk->div_fsys1;
	} else {
		addr = (unsigned int)&clk->div_fsys2;
		dev_index -= 2;
	}

	val = readl(addr);
	val &= ~(0xff << ((dev_index << 4) + 8));
	val |= (div & 0xff) << ((dev_index << 4) + 8);
	writel(val, addr);
}

/* exynos5: set the mmc clock */
static void exynos5_set_mmc_clk(int dev_index, unsigned int div)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned int addr;
	unsigned int val;

	/*
	 * CLK_DIV_FSYS1
	 * MMC0_PRE_RATIO [15:8], MMC1_PRE_RATIO [31:24]
	 * CLK_DIV_FSYS2
	 * MMC2_PRE_RATIO [15:8], MMC3_PRE_RATIO [31:24]
	 */
	if (dev_index < 2) {
		addr = (unsigned int)&clk->div_fsys1;
	} else {
		addr = (unsigned int)&clk->div_fsys2;
		dev_index -= 2;
	}

	val = readl(addr);
	val &= ~(0xff << ((dev_index << 4) + 8));
	val |= (div & 0xff) << ((dev_index << 4) + 8);
	writel(val, addr);
}

/* get_lcd_clk: return lcd clock frequency */
static unsigned long exynos4_get_lcd_clk(void)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();
	unsigned long pclk, sclk;
	unsigned int sel;
	unsigned int ratio;

	/*
	 * CLK_SRC_LCD0
	 * FIMD0_SEL [3:0]
	 */
	sel = readl(&clk->src_lcd0);
	sel = sel & 0xf;

	/*
	 * 0x6: SCLK_MPLL
	 * 0x7: SCLK_EPLL
	 * 0x8: SCLK_VPLL
	 */
	if (sel == 0x6)
		sclk = get_pll_clk(MPLL);
	else if (sel == 0x7)
		sclk = get_pll_clk(EPLL);
	else if (sel == 0x8)
		sclk = get_pll_clk(VPLL);
	else
		return 0;

	/*
	 * CLK_DIV_LCD0
	 * FIMD0_RATIO [3:0]
	 */
	ratio = readl(&clk->div_lcd0);
	ratio = ratio & 0xf;

	pclk = sclk / (ratio + 1);

	return pclk;
}

/* get_lcd_clk: return lcd clock frequency */
static unsigned long exynos5_get_lcd_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long pclk, sclk;
	unsigned int sel;
	unsigned int ratio;

	/*
	 * CLK_SRC_LCD0
	 * FIMD0_SEL [3:0]
	 */
	sel = readl(&clk->src_disp1_0);
	sel = sel & 0xf;

	/*
	 * 0x6: SCLK_MPLL
	 * 0x7: SCLK_EPLL
	 * 0x8: SCLK_VPLL
	 */
	if (sel == 0x6)
		sclk = get_pll_clk(MPLL);
	else if (sel == 0x7)
		sclk = get_pll_clk(EPLL);
	else if (sel == 0x8)
		sclk = get_pll_clk(VPLL);
	else
		return 0;

	/*
	 * CLK_DIV_LCD0
	 * FIMD0_RATIO [3:0]
	 */
	ratio = readl(&clk->div_disp1_0);
	ratio = ratio & 0xf;

	pclk = sclk / (ratio + 1);

	return pclk;
}

void exynos4_set_lcd_clk(void)
{
	struct exynos4_clock *clk =
	    (struct exynos4_clock *)samsung_get_base_clock();
	unsigned int cfg = 0;

	/*
	 * CLK_GATE_BLOCK
	 * CLK_CAM	[0]
	 * CLK_TV	[1]
	 * CLK_MFC	[2]
	 * CLK_G3D	[3]
	 * CLK_LCD0	[4]
	 * CLK_LCD1	[5]
	 * CLK_GPS	[7]
	 */
	cfg = readl(&clk->gate_block);
	cfg |= 1 << 4;
	writel(cfg, &clk->gate_block);

	/*
	 * CLK_SRC_LCD0
	 * FIMD0_SEL		[3:0]
	 * MDNIE0_SEL		[7:4]
	 * MDNIE_PWM0_SEL	[8:11]
	 * MIPI0_SEL		[12:15]
	 * set lcd0 src clock 0x6: SCLK_MPLL
	 */
	cfg = readl(&clk->src_lcd0);
	cfg &= ~(0xf);
	cfg |= 0x6;
	writel(cfg, &clk->src_lcd0);

	/*
	 * CLK_GATE_IP_LCD0
	 * CLK_FIMD0		[0]
	 * CLK_MIE0		[1]
	 * CLK_MDNIE0		[2]
	 * CLK_DSIM0		[3]
	 * CLK_SMMUFIMD0	[4]
	 * CLK_PPMULCD0		[5]
	 * Gating all clocks for FIMD0
	 */
	cfg = readl(&clk->gate_ip_lcd0);
	cfg |= 1 << 0;
	writel(cfg, &clk->gate_ip_lcd0);

	/*
	 * CLK_DIV_LCD0
	 * FIMD0_RATIO		[3:0]
	 * MDNIE0_RATIO		[7:4]
	 * MDNIE_PWM0_RATIO	[11:8]
	 * MDNIE_PWM_PRE_RATIO	[15:12]
	 * MIPI0_RATIO		[19:16]
	 * MIPI0_PRE_RATIO	[23:20]
	 * set fimd ratio
	 */
	cfg &= ~(0xf);
	cfg |= 0x1;
	writel(cfg, &clk->div_lcd0);
}

void exynos5_set_lcd_clk(void)
{
	struct exynos5_clock *clk =
	    (struct exynos5_clock *)samsung_get_base_clock();
	unsigned int cfg = 0;

	/*
	 * CLK_GATE_BLOCK
	 * CLK_CAM	[0]
	 * CLK_TV	[1]
	 * CLK_MFC	[2]
	 * CLK_G3D	[3]
	 * CLK_LCD0	[4]
	 * CLK_LCD1	[5]
	 * CLK_GPS	[7]
	 */
	cfg = readl(&clk->gate_block);
	cfg |= 1 << 4;
	writel(cfg, &clk->gate_block);

	/*
	 * CLK_SRC_LCD0
	 * FIMD0_SEL		[3:0]
	 * MDNIE0_SEL		[7:4]
	 * MDNIE_PWM0_SEL	[8:11]
	 * MIPI0_SEL		[12:15]
	 * set lcd0 src clock 0x6: SCLK_MPLL
	 */
	cfg = readl(&clk->src_disp1_0);
	cfg &= ~(0xf);
	cfg |= 0x8;
	writel(cfg, &clk->src_disp1_0);

	/*
	 * CLK_GATE_IP_LCD0
	 * CLK_FIMD0		[0]
	 * CLK_MIE0		[1]
	 * CLK_MDNIE0		[2]
	 * CLK_DSIM0		[3]
	 * CLK_SMMUFIMD0	[4]
	 * CLK_PPMULCD0		[5]
	 * Gating all clocks for FIMD0
	 */
	cfg = readl(&clk->gate_ip_disp1);
	cfg |= 1 << 0;
	writel(cfg, &clk->gate_ip_disp1);

	/*
	 * CLK_DIV_LCD0
	 * FIMD0_RATIO		[3:0]
	 * MDNIE0_RATIO		[7:4]
	 * MDNIE_PWM0_RATIO	[11:8]
	 * MDNIE_PWM_PRE_RATIO	[15:12]
	 * MIPI0_RATIO		[19:16]
	 * MIPI0_PRE_RATIO	[23:20]
	 * set fimd ratio
	 */
	cfg &= ~(0xf);
	cfg |= 0x0;
	writel(cfg, &clk->div_disp1_0);
}

void exynos4_set_mipi_clk(void)
{
	struct exynos4_clock *clk =
	    (struct exynos4_clock *)samsung_get_base_clock();
	unsigned int cfg = 0;

	/*
	 * CLK_SRC_LCD0
	 * FIMD0_SEL		[3:0]
	 * MDNIE0_SEL		[7:4]
	 * MDNIE_PWM0_SEL	[8:11]
	 * MIPI0_SEL		[12:15]
	 * set mipi0 src clock 0x6: SCLK_MPLL
	 */
	cfg = readl(&clk->src_lcd0);
	cfg &= ~(0xf << 12);
	cfg |= (0x6 << 12);
	writel(cfg, &clk->src_lcd0);

	/*
	 * CLK_SRC_MASK_LCD0
	 * FIMD0_MASK		[0]
	 * MDNIE0_MASK		[4]
	 * MDNIE_PWM0_MASK	[8]
	 * MIPI0_MASK		[12]
	 * set src mask mipi0 0x1: Unmask
	 */
	cfg = readl(&clk->src_mask_lcd0);
	cfg |= (0x1 << 12);
	writel(cfg, &clk->src_mask_lcd0);

	/*
	 * CLK_GATE_IP_LCD0
	 * CLK_FIMD0		[0]
	 * CLK_MIE0		[1]
	 * CLK_MDNIE0		[2]
	 * CLK_DSIM0		[3]
	 * CLK_SMMUFIMD0	[4]
	 * CLK_PPMULCD0		[5]
	 * Gating all clocks for MIPI0
	 */
	cfg = readl(&clk->gate_ip_lcd0);
	cfg |= 1 << 3;
	writel(cfg, &clk->gate_ip_lcd0);

	/*
	 * CLK_DIV_LCD0
	 * FIMD0_RATIO		[3:0]
	 * MDNIE0_RATIO		[7:4]
	 * MDNIE_PWM0_RATIO	[11:8]
	 * MDNIE_PWM_PRE_RATIO	[15:12]
	 * MIPI0_RATIO		[19:16]
	 * MIPI0_PRE_RATIO	[23:20]
	 * set mipi ratio
	 */
	cfg &= ~(0xf << 16);
	cfg |= (0x1 << 16);
	writel(cfg, &clk->div_lcd0);
}

/*
 * I2C
 *
 * exynos5: obtaining the I2C clock
 */
static unsigned long exynos5_get_i2c_clk(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();
	unsigned long aclk_66, aclk_66_pre, sclk;
	unsigned int ratio;

	sclk = get_pll_clk(MPLL);

	ratio = (readl(&clk->div_top1)) >> 24;
	ratio &= 0x7;
	aclk_66_pre = sclk / (ratio + 1);
	ratio = readl(&clk->div_top0);
	ratio &= 0x7;
	aclk_66 = aclk_66_pre / (ratio + 1);
	return aclk_66;
}

unsigned long get_pll_clk(int pllreg)
{
	if (cpu_is_exynos5())
		return exynos5_get_pll_clk(pllreg);
	else
		return exynos4_get_pll_clk(pllreg);
}

unsigned long get_arm_clk(void)
{
	if (cpu_is_exynos5())
		return exynos5_get_arm_clk();
	else
		return exynos4_get_arm_clk();
}

unsigned long get_i2c_clk(void)
{
	if (cpu_is_exynos5()) {
		return exynos5_get_i2c_clk();
	} else {
		debug("I2C clock is not set for this CPU\n");
		return 0;
	}
}

unsigned long get_pwm_clk(void)
{
	if (cpu_is_exynos5())
		return exynos5_get_pwm_clk();
	else
		return exynos4_get_pwm_clk();
}

unsigned long get_uart_clk(int dev_index)
{
	if (cpu_is_exynos5())
		return exynos5_get_uart_clk(dev_index);
	else
		return exynos4_get_uart_clk(dev_index);
}

void set_mmc_clk(int dev_index, unsigned int div)
{
	if (cpu_is_exynos5())
		exynos5_set_mmc_clk(dev_index, div);
	else
		exynos4_set_mmc_clk(dev_index, div);
}

unsigned long get_lcd_clk(void)
{
	if (cpu_is_exynos4())
		return exynos4_get_lcd_clk();
	else
		return exynos5_get_lcd_clk();
}

void set_lcd_clk(void)
{
	if (cpu_is_exynos4())
		exynos4_set_lcd_clk();
	else
		exynos5_set_lcd_clk();
}

void set_mipi_clk(void)
{
	if (cpu_is_exynos4())
		exynos4_set_mipi_clk();
}
