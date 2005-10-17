/*
 * Copyright 2004 Freescale Semiconductor, Inc.
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
 *
 * Change log:
 *
 * 20050101: Eran Liberty (liberty@freescale.com)
 *	     Initial file creating (porting from 85XX & 8260)
 */

/*
 * CPU specific code for the MPC83xx family.
 *
 * Derived from the MPC8260 and MPC85xx.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc83xx.h>
#include <asm/processor.h>


int checkcpu(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr();
	char buf[32];

	if ((pvr & 0xFFFF0000) != PVR_83xx) {
		puts("Not MPC83xx Family!!!\n");
		return -1;
	}

	puts("CPU:   MPC83xx, ");
	switch(pvr) {
	case PVR_8349_REV10:
		break;
	case PVR_8349_REV11:
		break;
	default:
		puts("Rev: Unknown\n");
		return -1;	/* Not sure what this is */
	}
	printf("Rev: %d.%d at %s MHz\n", (pvr & 0xf0) >> 4,
		(pvr & 0x0f), strmhz(buf, clock));

	return 0;
}


void upmconfig (uint upm, uint *table, uint size)
{
	hang();		/* FIXME: upconfig() needed? */
}


int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
#ifndef MPC83xx_RESET
	ulong addr;
#endif

	volatile immap_t *immap = (immap_t *) CFG_IMMRBAR;

#ifdef MPC83xx_RESET
	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~( MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/* enable Reset Control Reg */
	immap->reset.rpr = 0x52535445;

	/* confirm Reset Control Reg is enabled */
	while(!((immap->reset.rcer) & RCER_CRE));

	printf("Resetting the board.");
	printf("\n");

	udelay(200);

	/* perform reset, only one bit */
	immap->reset.rcr = RCR_SWHR;

#else	/* ! MPC83xx_RESET */

	immap->reset.rmr = RMR_CSRE;    /* Checkstop Reset enable */

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
	addr = CFG_RESET_ADDRESS;

	printf("resetting the board.");
	printf("\n");
	((void (*)(void)) addr) ();
#endif	/* MPC83xx_RESET */

	return 1;
}


/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */

unsigned long get_tbclk(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return tbclk;
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	hang();		/* FIXME: implement watchdog_reset()? */
}
#endif /* CONFIG_WATCHDOG */
