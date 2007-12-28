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

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers
 */
void
cpu_init_f (void)
{
#if defined(CONFIG_WATCHDOG)
	unsigned long val;
#endif

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

#if defined (CONFIG_405EP)
	/*
	 * Set EMAC noise filter bits
	 */
	mtdcr(cpc0_epctl, CPC0_EPRCSR_E0NFE | CPC0_EPRCSR_E1NFE);
#endif /* CONFIG_405EP */
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
