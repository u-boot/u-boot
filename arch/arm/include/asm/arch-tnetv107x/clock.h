/*
 * TNETV107X: Clock APIs
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#define PSC_MDCTL_NEXT_SWRSTDISABLE	0x0
#define PSC_MDCTL_NEXT_SYNCRST		0x1
#define PSC_MDCTL_NEXT_DISABLE		0x2
#define PSC_MDCTL_NEXT_ENABLE		0x3

#define CONFIG_SYS_INT_OSC_FREQ		24000000

#ifndef __ASSEMBLY__

/* PLL identifiers */
enum pll_type_e {
	SYS_PLL,
	TDM_PLL,
	ETH_PLL
};

/* PLL configuration data */
struct pll_init_data {
	int pll;
	int internal_osc;
	unsigned long pll_freq;
	unsigned long div_freq[10];
};

void init_plls(int num_pll, struct pll_init_data *config);
int  lpsc_status(unsigned int mod);
void lpsc_control(int mod, unsigned long state, int lrstz);
unsigned long clk_get_rate(unsigned int clk);
unsigned long clk_round_rate(unsigned int clk, unsigned long hz);
int clk_set_rate(unsigned int clk, unsigned long hz);

static inline void clk_enable(unsigned int mod)
{
	lpsc_control(mod, PSC_MDCTL_NEXT_ENABLE, -1);
}

static inline void clk_disable(unsigned int mod)
{
	lpsc_control(mod, PSC_MDCTL_NEXT_DISABLE, -1);
}

#endif

#endif
