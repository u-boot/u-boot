/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
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
#include <watchdog.h>

#ifdef	CONFIG_M5272
#include <asm/m5272.h>
#include <asm/immap_5272.h>
#endif

#ifdef	CONFIG_M5282
#include <asm/m5282.h>
#include <asm/immap_5282.h>
#endif

#ifdef	CONFIG_M5272
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (void)
{
	/* if we come from RAM we assume the CPU is
	 * already initialized.
	 */
#ifndef CONFIG_MONITOR_IS_IN_RAM
	volatile immap_t *regp = (immap_t *)CFG_MBAR;

	volatile unsigned char	*mbar;
	mbar = (volatile unsigned char *) CFG_MBAR;

	regp->sysctrl_reg.sc_scr = CFG_SCR;
	regp->sysctrl_reg.sc_spr = CFG_SPR;

	/* Setup Ports:	*/
	regp->gpio_reg.gpio_pacnt = CFG_PACNT;
	regp->gpio_reg.gpio_paddr = CFG_PADDR;
	regp->gpio_reg.gpio_padat = CFG_PADAT;
	regp->gpio_reg.gpio_pbcnt = CFG_PBCNT;
	regp->gpio_reg.gpio_pbddr = CFG_PBDDR;
	regp->gpio_reg.gpio_pbdat = CFG_PBDAT;
	regp->gpio_reg.gpio_pdcnt = CFG_PDCNT;

	/* Memory Controller: */
	regp->csctrl_reg.cs_br0 = CFG_BR0_PRELIM;
	regp->csctrl_reg.cs_or0 = CFG_OR0_PRELIM;

#if (defined(CFG_OR1_PRELIM) && defined(CFG_BR1_PRELIM))
	regp->csctrl_reg.cs_br1 = CFG_BR1_PRELIM;
	regp->csctrl_reg.cs_or1 = CFG_OR1_PRELIM;
#endif

#if defined(CFG_OR2_PRELIM) && defined(CFG_BR2_PRELIM)
	regp->csctrl_reg.cs_br2 = CFG_BR2_PRELIM;
	regp->csctrl_reg.cs_or2 = CFG_OR2_PRELIM;
#endif

#if defined(CFG_OR3_PRELIM) && defined(CFG_BR3_PRELIM)
	regp->csctrl_reg.cs_br3 = CFG_BR3_PRELIM;
	regp->csctrl_reg.cs_or3 = CFG_OR3_PRELIM;
#endif

#if defined(CFG_OR4_PRELIM) && defined(CFG_BR4_PRELIM)
	regp->csctrl_reg.cs_br4 = CFG_BR4_PRELIM;
	regp->csctrl_reg.cs_or4 = CFG_OR4_PRELIM;
#endif

#if defined(CFG_OR5_PRELIM) && defined(CFG_BR5_PRELIM)
	regp->csctrl_reg.cs_br5 = CFG_BR5_PRELIM;
	regp->csctrl_reg.cs_or5 = CFG_OR5_PRELIM;
#endif

#if defined(CFG_OR6_PRELIM) && defined(CFG_BR6_PRELIM)
	regp->csctrl_reg.cs_br6 = CFG_BR6_PRELIM;
	regp->csctrl_reg.cs_or6 = CFG_OR6_PRELIM;
#endif

#if defined(CFG_OR7_PRELIM) && defined(CFG_BR7_PRELIM)
	regp->csctrl_reg.cs_br7 = CFG_BR7_PRELIM;
	regp->csctrl_reg.cs_or7 = CFG_OR7_PRELIM;
#endif

#endif /* #ifndef CONFIG_MONITOR_IS_IN_RAM */

    /* enable instruction cache now */
    icache_enable();

}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r  (void)
{
	return (0);
}
#endif /* #ifdef CONFIG_M5272 */


#ifdef	CONFIG_M5282
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (void)
{

}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r  (void)
{
	return (0);
}
#endif
