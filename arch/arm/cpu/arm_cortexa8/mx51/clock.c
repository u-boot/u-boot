/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
#include <asm/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>

enum pll_clocks {
	PLL1_CLOCK = 0,
	PLL2_CLOCK,
	PLL3_CLOCK,
	PLL_CLOCKS,
};

struct mxc_pll_reg *mxc_plls[PLL_CLOCKS] = {
	[PLL1_CLOCK] = (struct mxc_pll_reg *)PLL1_BASE_ADDR,
	[PLL2_CLOCK] = (struct mxc_pll_reg *)PLL2_BASE_ADDR,
	[PLL3_CLOCK] = (struct mxc_pll_reg *)PLL3_BASE_ADDR,
};

struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;

/*
 * Calculate the frequency of this pll.
 */
static u32 decode_pll(struct mxc_pll_reg *pll, u32 infreq)
{
	u32 mfi, mfn, mfd, pd;

	mfn = __raw_readl(&pll->mfn);
	mfd = __raw_readl(&pll->mfd) + 1;
	mfi = __raw_readl(&pll->op);
	pd = (mfi  & 0xF) + 1;
	mfi = (mfi >> 4) & 0xF;
	mfi = (mfi >= 5) ? mfi : 5;

	return ((4 * (infreq / 1000) * (mfi * mfd + mfn)) / (mfd * pd)) * 1000;
}

/*
 * Get mcu main rate
 */
u32 get_mcu_main_clk(void)
{
	u32 reg, freq;

	reg = (__raw_readl(&mxc_ccm->cacrr) & MXC_CCM_CACRR_ARM_PODF_MASK) >>
		MXC_CCM_CACRR_ARM_PODF_OFFSET;
	freq = decode_pll(mxc_plls[PLL1_CLOCK], CONFIG_MX51_HCLK_FREQ);
	return freq / (reg + 1);
}

/*
 * Get the rate of peripheral's root clock.
 */
static u32 get_periph_clk(void)
{
	u32 reg;

	reg = __raw_readl(&mxc_ccm->cbcdr);
	if (!(reg & MXC_CCM_CBCDR_PERIPH_CLK_SEL))
		return decode_pll(mxc_plls[PLL2_CLOCK], CONFIG_MX51_HCLK_FREQ);
	reg = __raw_readl(&mxc_ccm->cbcmr);
	switch ((reg & MXC_CCM_CBCMR_PERIPH_CLK_SEL_MASK) >>
		MXC_CCM_CBCMR_PERIPH_CLK_SEL_OFFSET) {
	case 0:
		return decode_pll(mxc_plls[PLL1_CLOCK], CONFIG_MX51_HCLK_FREQ);
	case 1:
		return decode_pll(mxc_plls[PLL3_CLOCK], CONFIG_MX51_HCLK_FREQ);
	default:
		return 0;
	}
	/* NOTREACHED */
}

/*
 * Get the rate of ipg clock.
 */
static u32 get_ipg_clk(void)
{
	u32 ahb_podf, ipg_podf;

	ahb_podf = __raw_readl(&mxc_ccm->cbcdr);
	ipg_podf = (ahb_podf & MXC_CCM_CBCDR_IPG_PODF_MASK) >>
			MXC_CCM_CBCDR_IPG_PODF_OFFSET;
	ahb_podf = (ahb_podf & MXC_CCM_CBCDR_AHB_PODF_MASK) >>
			MXC_CCM_CBCDR_AHB_PODF_OFFSET;
	return get_periph_clk() / ((ahb_podf + 1) * (ipg_podf + 1));
}

/*
 * Get the rate of ipg_per clock.
 */
static u32 get_ipg_per_clk(void)
{
	u32 pred1, pred2, podf;

	if (__raw_readl(&mxc_ccm->cbcmr) & MXC_CCM_CBCMR_PERCLK_IPG_CLK_SEL)
		return get_ipg_clk();
	/* Fixme: not handle what about lpm*/
	podf = __raw_readl(&mxc_ccm->cbcdr);
	pred1 = (podf & MXC_CCM_CBCDR_PERCLK_PRED1_MASK) >>
		MXC_CCM_CBCDR_PERCLK_PRED1_OFFSET;
	pred2 = (podf & MXC_CCM_CBCDR_PERCLK_PRED2_MASK) >>
		MXC_CCM_CBCDR_PERCLK_PRED2_OFFSET;
	podf = (podf & MXC_CCM_CBCDR_PERCLK_PODF_MASK) >>
		MXC_CCM_CBCDR_PERCLK_PODF_OFFSET;

	return get_periph_clk() / ((pred1 + 1) * (pred2 + 1) * (podf + 1));
}

/*
 * Get the rate of uart clk.
 */
static u32 get_uart_clk(void)
{
	unsigned int freq, reg, pred, podf;

	reg = __raw_readl(&mxc_ccm->cscmr1);
	switch ((reg & MXC_CCM_CSCMR1_UART_CLK_SEL_MASK) >>
		MXC_CCM_CSCMR1_UART_CLK_SEL_OFFSET) {
	case 0x0:
		freq = decode_pll(mxc_plls[PLL1_CLOCK],
				    CONFIG_MX51_HCLK_FREQ);
		break;
	case 0x1:
		freq = decode_pll(mxc_plls[PLL2_CLOCK],
				    CONFIG_MX51_HCLK_FREQ);
		break;
	case 0x2:
		freq = decode_pll(mxc_plls[PLL3_CLOCK],
				    CONFIG_MX51_HCLK_FREQ);
		break;
	default:
		return 66500000;
	}

	reg = __raw_readl(&mxc_ccm->cscdr1);

	pred = (reg & MXC_CCM_CSCDR1_UART_CLK_PRED_MASK) >>
		MXC_CCM_CSCDR1_UART_CLK_PRED_OFFSET;

	podf = (reg & MXC_CCM_CSCDR1_UART_CLK_PODF_MASK) >>
		MXC_CCM_CSCDR1_UART_CLK_PODF_OFFSET;
	freq /= (pred + 1) * (podf + 1);

	return freq;
}

/*
 * This function returns the low power audio clock.
 */
u32 get_lp_apm(void)
{
	u32 ret_val = 0;
	u32 ccsr = __raw_readl(&mxc_ccm->ccsr);

	if (((ccsr >> 9) & 1) == 0)
		ret_val = CONFIG_MX51_HCLK_FREQ;
	else
		ret_val = ((32768 * 1024));

	return ret_val;
}

/*
 * get cspi clock rate.
 */
u32 imx_get_cspiclk(void)
{
	u32 ret_val = 0, pdf, pre_pdf, clk_sel;
	u32 cscmr1 = __raw_readl(&mxc_ccm->cscmr1);
	u32 cscdr2 = __raw_readl(&mxc_ccm->cscdr2);

	pre_pdf = (cscdr2 & MXC_CCM_CSCDR2_CSPI_CLK_PRED_MASK) \
			>> MXC_CCM_CSCDR2_CSPI_CLK_PRED_OFFSET;
	pdf = (cscdr2 & MXC_CCM_CSCDR2_CSPI_CLK_PODF_MASK) \
			>> MXC_CCM_CSCDR2_CSPI_CLK_PODF_OFFSET;
	clk_sel = (cscmr1 & MXC_CCM_CSCMR1_CSPI_CLK_SEL_MASK) \
			>> MXC_CCM_CSCMR1_CSPI_CLK_SEL_OFFSET;

	switch (clk_sel) {
	case 0:
		ret_val = decode_pll(mxc_plls[PLL1_CLOCK],
					CONFIG_MX51_HCLK_FREQ) /
					((pre_pdf + 1) * (pdf + 1));
		break;
	case 1:
		ret_val = decode_pll(mxc_plls[PLL2_CLOCK],
					CONFIG_MX51_HCLK_FREQ) /
					((pre_pdf + 1) * (pdf + 1));
		break;
	case 2:
		ret_val = decode_pll(mxc_plls[PLL3_CLOCK],
					CONFIG_MX51_HCLK_FREQ) /
					((pre_pdf + 1) * (pdf + 1));
		break;
	default:
		ret_val = get_lp_apm() / ((pre_pdf + 1) * (pdf + 1));
		break;
	}

	return ret_val;
}

/*
 * The API of get mxc clockes.
 */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_AHB_CLK:
		break;
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_IPG_PERCLK:
		return get_ipg_per_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_CSPI_CLK:
		return imx_get_cspiclk();
	case MXC_FEC_CLK:
		return decode_pll(mxc_plls[PLL1_CLOCK],
				    CONFIG_MX51_HCLK_FREQ);
	default:
		break;
	}
	return -1;
}

u32 imx_get_uartclk(void)
{
	return get_uart_clk();
}


u32 imx_get_fecclk(void)
{
	return mxc_get_clock(MXC_IPG_CLK);
}

/*
 * Dump some core clockes.
 */
int do_mx51_showclocks(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 freq;

	freq = decode_pll(mxc_plls[PLL1_CLOCK], CONFIG_MX51_HCLK_FREQ);
	printf("mx51 pll1: %dMHz\n", freq / 1000000);
	freq = decode_pll(mxc_plls[PLL2_CLOCK], CONFIG_MX51_HCLK_FREQ);
	printf("mx51 pll2: %dMHz\n", freq / 1000000);
	freq = decode_pll(mxc_plls[PLL3_CLOCK], CONFIG_MX51_HCLK_FREQ);
	printf("mx51 pll3: %dMHz\n", freq / 1000000);
	printf("ipg clock     : %dHz\n", mxc_get_clock(MXC_IPG_CLK));
	printf("ipg per clock : %dHz\n", mxc_get_clock(MXC_IPG_PERCLK));

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	clockinfo,	CONFIG_SYS_MAXARGS,	1,	do_mx51_showclocks,
	"display mx51 clocks\n",
	""
);
