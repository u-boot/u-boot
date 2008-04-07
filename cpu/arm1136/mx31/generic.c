/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
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
#include <asm/arch/mx31-regs.h>

static u32 mx31_decode_pll(u32 reg, u32 infreq)
{
	u32 mfi = (reg >> 10) & 0xf;
	u32 mfn = reg & 0x3f;
	u32 mfd = (reg >> 16) & 0x3f;
	u32 pd =  (reg >> 26) & 0xf;

	mfi = mfi <= 5 ? 5 : mfi;
	mfd += 1;
	pd += 1;

	return ((2 * (infreq >> 10) * (mfi * mfd + mfn)) /
		(mfd * pd)) << 10;
}

u32 mx31_get_mpl_dpdgck_clk(void)
{
	u32 infreq;

	if ((__REG(CCM_CCMR) & CCMR_PRCS_MASK) == CCMR_FPM)
		infreq = CONFIG_MX31_CLK32 * 1024;
	else
		infreq = CONFIG_MX31_HCLK_FREQ;

	return mx31_decode_pll(__REG(CCM_MPCTL), infreq); }

u32 mx31_get_mcu_main_clk(void)
{
	/* For now we assume mpl_dpdgck_clk == mcu_main_clk
	 * which should be correct for most boards
	 */
	return mx31_get_mpl_dpdgck_clk();
}

u32 mx31_get_ipg_clk(void)
{
	u32 freq = mx31_get_mcu_main_clk();
	u32 pdr0 = __REG(CCM_PDR0);

	freq /= ((pdr0 >> 3) & 0x7) + 1;
	freq /= ((pdr0 >> 6) & 0x3) + 1;

	return freq;
}

void mx31_dump_clocks(void)
{
	u32 cpufreq = mx31_get_mcu_main_clk();
	printf("mx31 cpu clock: %dMHz\n", cpufreq / 1000000);
	printf("ipg clock     : %dHz\n", mx31_get_ipg_clk());
}

void mx31_gpio_mux(unsigned long mode)
{
	unsigned long reg, shift, tmp;

	reg = IOMUXC_BASE + (mode & 0xfc);
	shift = (~mode & 0x3) * 8;

	tmp = __REG(reg);
	tmp &= ~(0xff << shift);
	tmp |= ((mode >> 8) & 0xff) << shift;
	__REG(reg) = tmp;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	printf("CPU:   Freescale i.MX31 at %d MHz\n",
		mx31_get_mcu_main_clk() / 1000000);
	return 0;
}
#endif
