/*
 * (C) Copyright 2000-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <ppc4xx_enet.h>
#include <asm/processor.h>
#include <asm/gpio.h>
#include <ppc4xx.h>

#if defined(CONFIG_405GP)  || defined(CONFIG_405EP)
DECLARE_GLOBAL_DATA_PTR;
#endif

#ifdef CFG_INIT_DCACHE_CS
# if (CFG_INIT_DCACHE_CS == 0)
#  define PBxAP pb0ap
#  define PBxCR pb0cr
#  if (defined(CFG_EBC_PB0AP) && defined(CFG_EBC_PB0CR))
#   define PBxAP_VAL CFG_EBC_PB0AP
#   define PBxCR_VAL CFG_EBC_PB0CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 1)
#  define PBxAP pb1ap
#  define PBxCR pb1cr
#  if (defined(CFG_EBC_PB1AP) && defined(CFG_EBC_PB1CR))
#   define PBxAP_VAL CFG_EBC_PB1AP
#   define PBxCR_VAL CFG_EBC_PB1CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 2)
#  define PBxAP pb2ap
#  define PBxCR pb2cr
#  if (defined(CFG_EBC_PB2AP) && defined(CFG_EBC_PB2CR))
#   define PBxAP_VAL CFG_EBC_PB2AP
#   define PBxCR_VAL CFG_EBC_PB2CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 3)
#  define PBxAP pb3ap
#  define PBxCR pb3cr
#  if (defined(CFG_EBC_PB3AP) && defined(CFG_EBC_PB3CR))
#   define PBxAP_VAL CFG_EBC_PB3AP
#   define PBxCR_VAL CFG_EBC_PB3CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 4)
#  define PBxAP pb4ap
#  define PBxCR pb4cr
#  if (defined(CFG_EBC_PB4AP) && defined(CFG_EBC_PB4CR))
#   define PBxAP_VAL CFG_EBC_PB4AP
#   define PBxCR_VAL CFG_EBC_PB4CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 5)
#  define PBxAP pb5ap
#  define PBxCR pb5cr
#  if (defined(CFG_EBC_PB5AP) && defined(CFG_EBC_PB5CR))
#   define PBxAP_VAL CFG_EBC_PB5AP
#   define PBxCR_VAL CFG_EBC_PB5CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 6)
#  define PBxAP pb6ap
#  define PBxCR pb6cr
#  if (defined(CFG_EBC_PB6AP) && defined(CFG_EBC_PB6CR))
#   define PBxAP_VAL CFG_EBC_PB6AP
#   define PBxCR_VAL CFG_EBC_PB6CR
#  endif
# endif
# if (CFG_INIT_DCACHE_CS == 7)
#  define PBxAP pb7ap
#  define PBxCR pb7cr
#  if (defined(CFG_EBC_PB7AP) && defined(CFG_EBC_PB7CR))
#   define PBxAP_VAL CFG_EBC_PB7AP
#   define PBxCR_VAL CFG_EBC_PB7CR
#  endif
# endif
#endif /* CFG_INIT_DCACHE_CS */

#ifndef CFG_PLL_RECONFIG
#define CFG_PLL_RECONFIG	0
#endif

void reconfigure_pll(u32 new_cpu_freq)
{
#if defined(CONFIG_440EPX)
	int	reset_needed = 0;
	u32	reg, temp;
	u32	prbdv0, target_prbdv0,				/* CLK_PRIMBD */
		fwdva, target_fwdva, fwdvb, target_fwdvb,	/* CLK_PLLD */
		fbdv, target_fbdv, lfbdv, target_lfbdv,
		perdv0,	target_perdv0,				/* CLK_PERD */
		spcid0,	target_spcid0;				/* CLK_SPCID */

	/* Reconfigure clocks if necessary.
	 * See PPC440EPx User's Manual, sections 8.2 and 14 */
	if (new_cpu_freq == 667) {
		target_prbdv0 = 2;
		target_fwdva = 2;
		target_fwdvb = 4;
		target_fbdv = 20;
		target_lfbdv = 1;
		target_perdv0 = 4;
		target_spcid0 = 4;

		mfcpr(clk_primbd, reg);
		temp = (reg & PRBDV_MASK) >> 24;
		prbdv0 = temp ? temp : 8;
		if (prbdv0 != target_prbdv0) {
			reg &= ~PRBDV_MASK;
			reg |= ((target_prbdv0 == 8 ? 0 : target_prbdv0) << 24);
			mtcpr(clk_primbd, reg);
			reset_needed = 1;
		}

		mfcpr(clk_plld, reg);

		temp = (reg & PLLD_FWDVA_MASK) >> 16;
		fwdva = temp ? temp : 16;

		temp = (reg & PLLD_FWDVB_MASK) >> 8;
		fwdvb = temp ? temp : 8;

		temp = (reg & PLLD_FBDV_MASK) >> 24;
		fbdv = temp ? temp : 32;

		temp = (reg & PLLD_LFBDV_MASK);
		lfbdv = temp ? temp : 64;

		if (fwdva != target_fwdva || fbdv != target_fbdv || lfbdv != target_lfbdv) {
			reg &= ~(PLLD_FWDVA_MASK | PLLD_FWDVB_MASK |
				 PLLD_FBDV_MASK | PLLD_LFBDV_MASK);
			reg |= ((target_fwdva == 16 ? 0 : target_fwdva) << 16) |
				((target_fwdvb == 8 ? 0 : target_fwdvb) << 8) |
				((target_fbdv == 32 ? 0 : target_fbdv) << 24) |
				(target_lfbdv == 64 ? 0 : target_lfbdv);
			mtcpr(clk_plld, reg);
			reset_needed = 1;
		}

		mfcpr(clk_perd, reg);
		perdv0 = (reg & CPR0_PERD_PERDV0_MASK) >> 24;
		if (perdv0 != target_perdv0) {
			reg &= ~CPR0_PERD_PERDV0_MASK;
			reg |= (target_perdv0 << 24);
			mtcpr(clk_perd, reg);
			reset_needed = 1;
		}

		mfcpr(clk_spcid, reg);
		temp = (reg & CPR0_SPCID_SPCIDV0_MASK) >> 24;
		spcid0 = temp ? temp : 4;
		if (spcid0 != target_spcid0) {
			reg &= ~CPR0_SPCID_SPCIDV0_MASK;
			reg |= ((target_spcid0 == 4 ? 0 : target_spcid0) << 24);
			mtcpr(clk_spcid, reg);
			reset_needed = 1;
		}

		/* Set reload inhibit so configuration will persist across
		 * processor resets */
		mfcpr(clk_icfg, reg);
		reg &= ~CPR0_ICFG_RLI_MASK;
		reg |= 1 << 31;
		mtcpr(clk_icfg, reg);
	}

	/* Reset processor if configuration changed */
	if (reset_needed) {
		__asm__ __volatile__ ("sync; isync");
		mtspr(dbcr0, 0x20000000);
	}
#endif
}

/*
 * Breath some life into the CPU...
 *
 * Reconfigure PLL if necessary,
 * set up the memory map,
 * initialize a bunch of registers
 */
void
cpu_init_f (void)
{
#if defined(CONFIG_WATCHDOG)
	unsigned long val;
#endif
	reconfigure_pll(CFG_PLL_RECONFIG);

#if (defined(CONFIG_405EP) || defined (CONFIG_405EX)) && !defined(CFG_4xx_GPIO_TABLE)
	/*
	 * GPIO0 setup (select GPIO or alternate function)
	 */
#if defined(CFG_GPIO0_OR)
	out32(GPIO0_OR, CFG_GPIO0_OR);		/* set initial state of output pins	*/
#endif
#if defined(CFG_GPIO0_ODR)
	out32(GPIO0_ODR, CFG_GPIO0_ODR);	/* open-drain select			*/
#endif
	out32(GPIO0_OSRH, CFG_GPIO0_OSRH);	/* output select			*/
	out32(GPIO0_OSRL, CFG_GPIO0_OSRL);
	out32(GPIO0_ISR1H, CFG_GPIO0_ISR1H);	/* input select				*/
	out32(GPIO0_ISR1L, CFG_GPIO0_ISR1L);
	out32(GPIO0_TSRH, CFG_GPIO0_TSRH);	/* three-state select			*/
	out32(GPIO0_TSRL, CFG_GPIO0_TSRL);
#if defined(CFG_GPIO0_ISR2H)
	out32(GPIO0_ISR2H, CFG_GPIO0_ISR2H);
	out32(GPIO0_ISR2L, CFG_GPIO0_ISR2L);
#endif
#if defined (CFG_GPIO0_TCR)
	out32(GPIO0_TCR, CFG_GPIO0_TCR);	/* enable output driver for outputs	*/
#endif
#endif /* CONFIG_405EP ... && !CFG_4xx_GPIO_TABLE */

#if defined (CONFIG_405EP)
	/*
	 * Set EMAC noise filter bits
	 */
	mtdcr(cpc0_epctl, CPC0_EPRCSR_E0NFE | CPC0_EPRCSR_E1NFE);

	/*
	 * Enable the internal PCI arbiter
	 */
	mtdcr(cpc0_pci, mfdcr(cpc0_pci) | CPC0_PCI_HOST_CFG_EN | CPC0_PCI_ARBIT_EN);
#endif /* CONFIG_405EP */

#if defined(CFG_4xx_GPIO_TABLE)
	gpio_set_chip_configuration();
#endif /* CFG_4xx_GPIO_TABLE */

	/*
	 * External Bus Controller (EBC) Setup
	 */
#if (defined(CFG_EBC_PB0AP) && defined(CFG_EBC_PB0CR))
#if (defined(CONFIG_405GP) || defined(CONFIG_405CR) || \
     defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
     defined(CONFIG_405EX) || defined(CONFIG_405))
	/*
	 * Move the next instructions into icache, since these modify the flash
	 * we are running from!
	 */
	asm volatile("	bl	0f"		::: "lr");
	asm volatile("0:	mflr	3"		::: "r3");
	asm volatile("	addi	4, 0, 14"	::: "r4");
	asm volatile("	mtctr	4"		::: "ctr");
	asm volatile("1:	icbt	0, 3");
	asm volatile("	addi	3, 3, 32"	::: "r3");
	asm volatile("	bdnz	1b"		::: "ctr", "cr0");
	asm volatile("	addis	3, 0, 0x0"	::: "r3");
	asm volatile("	ori	3, 3, 0xA000"	::: "r3");
	asm volatile("	mtctr	3"		::: "ctr");
	asm volatile("2:	bdnz	2b"		::: "ctr", "cr0");
#endif

	mtebc(pb0ap, CFG_EBC_PB0AP);
	mtebc(pb0cr, CFG_EBC_PB0CR);
#endif

#if (defined(CFG_EBC_PB1AP) && defined(CFG_EBC_PB1CR) && !(CFG_INIT_DCACHE_CS == 1))
	mtebc(pb1ap, CFG_EBC_PB1AP);
	mtebc(pb1cr, CFG_EBC_PB1CR);
#endif

#if (defined(CFG_EBC_PB2AP) && defined(CFG_EBC_PB2CR) && !(CFG_INIT_DCACHE_CS == 2))
	mtebc(pb2ap, CFG_EBC_PB2AP);
	mtebc(pb2cr, CFG_EBC_PB2CR);
#endif

#if (defined(CFG_EBC_PB3AP) && defined(CFG_EBC_PB3CR) && !(CFG_INIT_DCACHE_CS == 3))
	mtebc(pb3ap, CFG_EBC_PB3AP);
	mtebc(pb3cr, CFG_EBC_PB3CR);
#endif

#if (defined(CFG_EBC_PB4AP) && defined(CFG_EBC_PB4CR) && !(CFG_INIT_DCACHE_CS == 4))
	mtebc(pb4ap, CFG_EBC_PB4AP);
	mtebc(pb4cr, CFG_EBC_PB4CR);
#endif

#if (defined(CFG_EBC_PB5AP) && defined(CFG_EBC_PB5CR) && !(CFG_INIT_DCACHE_CS == 5))
	mtebc(pb5ap, CFG_EBC_PB5AP);
	mtebc(pb5cr, CFG_EBC_PB5CR);
#endif

#if (defined(CFG_EBC_PB6AP) && defined(CFG_EBC_PB6CR) && !(CFG_INIT_DCACHE_CS == 6))
	mtebc(pb6ap, CFG_EBC_PB6AP);
	mtebc(pb6cr, CFG_EBC_PB6CR);
#endif

#if (defined(CFG_EBC_PB7AP) && defined(CFG_EBC_PB7CR) && !(CFG_INIT_DCACHE_CS == 7))
	mtebc(pb7ap, CFG_EBC_PB7AP);
	mtebc(pb7cr, CFG_EBC_PB7CR);
#endif

#if defined (CFG_EBC_CFG)
	mtebc(EBC0_CFG, CFG_EBC_CFG);
#endif

#if defined(CONFIG_WATCHDOG)
	val = mfspr(tcr);
#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
	val |= 0xb8000000;      /* generate system reset after 1.34 seconds */
#elif defined(CONFIG_440EPX)
	val |= 0xb0000000;      /* generate system reset after 1.34 seconds */
#else
	val |= 0xf0000000;      /* generate system reset after 2.684 seconds */
#endif
#if defined(CFG_4xx_RESET_TYPE)
	val &= ~0x30000000;			/* clear WRC bits */
	val |= CFG_4xx_RESET_TYPE << 28;	/* set board specific WRC type */
#endif
	mtspr(tcr, val);

	val = mfspr(tsr);
	val |= 0x80000000;      /* enable watchdog timer */
	mtspr(tsr, val);

	reset_4xx_watchdog();
#endif /* CONFIG_WATCHDOG */
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
#if defined(CONFIG_405GP)  || defined(CONFIG_405EP)
	bd_t *bd = gd->bd;
	unsigned long reg;
#if defined(CONFIG_405GP)
	uint pvr = get_pvr();
#endif

#ifdef CFG_INIT_DCACHE_CS
	/*
	 * Flush and invalidate dcache, then disable CS for temporary stack.
	 * Afterwards, this CS can be used for other purposes
	 */
	dcache_disable();   /* flush and invalidate dcache */
	mtebc(PBxAP, 0);
	mtebc(PBxCR, 0);    /* disable CS for temporary stack */

#if (defined(PBxAP_VAL) && defined(PBxCR_VAL))
	/*
	 * Write new value into CS register
	 */
	mtebc(PBxAP, PBxAP_VAL);
	mtebc(PBxCR, PBxCR_VAL);
#endif
#endif /* CFG_INIT_DCACHE_CS */

	/*
	 * Write Ethernetaddress into on-chip register
	 */
	reg = 0x00000000;
	reg |= bd->bi_enetaddr[0];           /* set high address */
	reg = reg << 8;
	reg |= bd->bi_enetaddr[1];
	out32 (EMAC_IAH, reg);

	reg = 0x00000000;
	reg |= bd->bi_enetaddr[2];           /* set low address  */
	reg = reg << 8;
	reg |= bd->bi_enetaddr[3];
	reg = reg << 8;
	reg |= bd->bi_enetaddr[4];
	reg = reg << 8;
	reg |= bd->bi_enetaddr[5];
	out32 (EMAC_IAL, reg);

#if defined(CONFIG_405GP)
	/*
	 * Set edge conditioning circuitry on PPC405GPr
	 * for compatibility to existing PPC405GP designs.
	 */
	if ((pvr & 0xfffffff0) == (PVR_405GPR_RB & 0xfffffff0)) {
		mtdcr(ecr, 0x60606000);
	}
#endif  /* defined(CONFIG_405GP) */
#endif  /* defined(CONFIG_405GP) || defined(CONFIG_405EP) */

	return (0);
}
