/*
 * (C) Copyright 2000
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
#include <405gp_enet.h>
#include <asm/processor.h>
#include <ppc4xx.h>


#define mtebc(reg, data)  mtdcr(ebccfga,reg);mtdcr(ebccfgd,data)


/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers
 */
void
cpu_init_f (void)
{
	/*
	 * External Bus Controller (EBC) Setup
	 */
#if (defined(CFG_EBC_PB0AP) && defined(CFG_EBC_PB0CR))
	/*
	 * Move the next instructions into icache, since these modify the flash
	 * we are running from!
	 */
	asm volatile("	bl	0f"		::: "lr");
	asm volatile("0:	mflr	3"		::: "r3");
	asm volatile("	addi 	4, 0, 14"	::: "r4");
	asm volatile("	mtctr	4"		::: "ctr");
	asm volatile("1:	icbt	0, 3");
	asm volatile("	addi	3, 3, 32"	::: "r3");
	asm volatile("	bdnz	1b"		::: "ctr", "cr0");
	asm volatile("	addis	3, 0, 0x0"	::: "r3");
	asm volatile("	ori	3, 3, 0xA000"	::: "r3");
	asm volatile("	mtctr	3"		::: "ctr");
	asm volatile("2:	bdnz	2b"		::: "ctr", "cr0");

	mtebc(pb0ap, CFG_EBC_PB0AP);
	mtebc(pb0cr, CFG_EBC_PB0CR);
#endif

#if (defined(CFG_EBC_PB1AP) && defined(CFG_EBC_PB1CR))
	mtebc(pb1ap, CFG_EBC_PB1AP);
	mtebc(pb1cr, CFG_EBC_PB1CR);
#endif

#if (defined(CFG_EBC_PB2AP) && defined(CFG_EBC_PB2CR))
	mtebc(pb2ap, CFG_EBC_PB2AP);
	mtebc(pb2cr, CFG_EBC_PB2CR);
#endif

#if (defined(CFG_EBC_PB3AP) && defined(CFG_EBC_PB3CR))
	mtebc(pb3ap, CFG_EBC_PB3AP);
	mtebc(pb3cr, CFG_EBC_PB3CR);
#endif

#if (defined(CFG_EBC_PB4AP) && defined(CFG_EBC_PB4CR))
	mtebc(pb4ap, CFG_EBC_PB4AP);
	mtebc(pb4cr, CFG_EBC_PB4CR);
#endif

#if (defined(CFG_EBC_PB5AP) && defined(CFG_EBC_PB5CR))
	mtebc(pb5ap, CFG_EBC_PB5AP);
	mtebc(pb5cr, CFG_EBC_PB5CR);
#endif

#if (defined(CFG_EBC_PB6AP) && defined(CFG_EBC_PB6CR))
	mtebc(pb6ap, CFG_EBC_PB6AP);
	mtebc(pb6cr, CFG_EBC_PB6CR);
#endif

#if (defined(CFG_EBC_PB7AP) && defined(CFG_EBC_PB7CR))
	mtebc(pb7ap, CFG_EBC_PB7AP);
	mtebc(pb7cr, CFG_EBC_PB7CR);
#endif

#if defined(CONFIG_WATCHDOG)
	unsigned long val;

	val = mfspr(tcr);
	val |= 0xf0000000;      /* generate system reset after 2.684 seconds */
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
#ifdef CONFIG_405GP
	DECLARE_GLOBAL_DATA_PTR;

	bd_t *bd = gd->bd;
	unsigned long reg;

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
#endif  /* CONFIG_405GP */
	return (0);
}
