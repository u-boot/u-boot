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

#ifdef	CONFIG_M5249
#include <asm/m5249.h>
#endif

#if defined(CONFIG_M5272)
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
#endif /* #if defined(CONFIG_M5272) */


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

#if defined(CONFIG_M5249)
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (void)
{
#ifndef CFG_PLL_BYPASS
	/*
	 *  Setup the PLL to run at the specified speed
	 *
	 */
	volatile unsigned long cpll = mbar2_readLong(MCFSIM_PLLCR);
	unsigned long pllcr;
#ifdef CFG_FAST_CLK
  	pllcr = 0x925a3100;                       /* ~140MHz clock (PLL bypass = 0) */
#else
	pllcr = 0x135a4140;                       /* ~72MHz clock (PLL bypass = 0) */
#endif
	cpll = cpll & 0xfffffffe; 		  /* Set PLL bypass mode = 0 (PSTCLK = crystal) */
	mbar2_writeLong(MCFSIM_PLLCR, cpll); 	  /* Set the PLL to bypass mode (PSTCLK = crystal) */
	mbar2_writeLong(MCFSIM_PLLCR, pllcr);  	  /* set the clock speed */
	pllcr ^= 0x00000001; 		      	  /* Set pll bypass to 1 */
	mbar2_writeLong(MCFSIM_PLLCR, pllcr);  	  /* Start locking (pll bypass = 1) */
	udelay(0x20);                             /* Wait for a lock ... */
#endif /* #ifndef CFG_PLL_BYPASS */

	/*
	 *  NOTE: by setting the GPIO_FUNCTION registers, we ensure that the UART pins
	 *        (UART0: gpio 30,27, UART1: gpio 31, 28) will be used as UART pins
	 *        which is their primary function.
	 *        ~Jeremy
	 */
	mbar2_writeLong(MCFSIM_GPIO_FUNC, CFG_GPIO_FUNC);
	mbar2_writeLong(MCFSIM_GPIO1_FUNC, CFG_GPIO1_FUNC);
	mbar2_writeLong(MCFSIM_GPIO_EN, CFG_GPIO_EN);
	mbar2_writeLong(MCFSIM_GPIO1_EN, CFG_GPIO1_EN);
	mbar2_writeLong(MCFSIM_GPIO_OUT, CFG_GPIO_OUT);
	mbar2_writeLong(MCFSIM_GPIO1_OUT, CFG_GPIO1_OUT);

	/*
	 *  dBug Compliance:
	 *    You can verify these values by using dBug's 'ird'
	 *    (Internal Register Display) command
	 *    ~Jeremy
	 *
 	 */
	mbar_writeByte(MCFSIM_MPARK, 0x30);    /* 5249 Internal Core takes priority over DMA */
	mbar_writeByte(MCFSIM_SYPCR, 0x00);
	mbar_writeByte(MCFSIM_SWIVR, 0x0f);
	mbar_writeByte(MCFSIM_SWSR, 0x00);
	mbar_writeLong(MCFSIM_IMR, 0xfffffbff);
	mbar_writeByte(MCFSIM_SWDICR, 0x00);
	mbar_writeByte(MCFSIM_TIMER1ICR, 0x00);
	mbar_writeByte(MCFSIM_TIMER2ICR, 0x88);
	mbar_writeByte(MCFSIM_I2CICR, 0x00);
	mbar_writeByte(MCFSIM_UART1ICR, 0x00);
	mbar_writeByte(MCFSIM_UART2ICR, 0x00);
	mbar_writeByte(MCFSIM_ICR6, 0x00);
	mbar_writeByte(MCFSIM_ICR7, 0x00);
	mbar_writeByte(MCFSIM_ICR8, 0x00);
	mbar_writeByte(MCFSIM_ICR9, 0x00);
	mbar_writeByte(MCFSIM_QSPIICR, 0x00);

	mbar2_writeLong(MCFSIM_GPIO_INT_EN, 0x00000080);
	mbar2_writeByte(MCFSIM_INTBASE, 0x40);  /* Base interrupts at 64 */
	mbar2_writeByte(MCFSIM_SPURVEC, 0x00);
	mbar2_writeLong(MCFSIM_IDECONFIG1, 0x00000020);  /* Enable a 1 cycle pre-drive cycle on CS1 */

	/* Setup interrupt priorities for gpio7 */
	/* mbar2_writeLong(MCFSIM_INTLEV5, 0x70000000); */

	/* IDE Config registers */
	mbar2_writeLong(MCFSIM_IDECONFIG1, 0x00000020);
	mbar2_writeLong(MCFSIM_IDECONFIG2, 0x00000000);

	/*
	 *  Setup chip selects...
	 */

	mbar_writeShort(MCFSIM_CSAR1, CFG_CSAR1);
	mbar_writeShort(MCFSIM_CSCR1, CFG_CSCR1);
	mbar_writeLong(MCFSIM_CSMR1, CFG_CSMR1);

	mbar_writeShort(MCFSIM_CSAR0, CFG_CSAR0);
	mbar_writeShort(MCFSIM_CSCR0, CFG_CSCR0);
	mbar_writeLong(MCFSIM_CSMR0, CFG_CSMR0);

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
#endif /* #if defined(CONFIG_M5249) */
