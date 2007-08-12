/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
 * (C) Copyright 2007 DENX Software Engineering
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

/*
 * CPU specific code for the MPC512x family.
 *
 * Derived from the MPC83xx code.
 */

#include <common.h>
#include <command.h>
#include <mpc512x.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr ();
	u32 spridr = immr->sysconf.spridr;
	char buf[32];

	puts("CPU:  ");

	switch (spridr & 0xffff0000) {
	case SPR_5121E:
		puts ("MPC5121e ");
		break;
	default:
		printf ("Unknown part ID %08x ", spridr & 0xffff0000);
	}
	printf ("rev. %d.%d, Core ", SVR_MJREV (spridr), SVR_MNREV (spridr));

	switch (pvr & 0xffff0000) {
	case PVR_E300C4:
		puts ("e300c4 ");
		break;
	default:
		puts ("unknown ");
	}
	printf ("at %s MHz, CSB at %3d MHz\n", strmhz(buf, clock),
		gd->csb_clk / 1000000);
	return 0;
}


int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~( MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Enable Reset Control Reg - "RSTE" is the magic word that let us go
	 */
	immap->reset.rpr = 0x52535445;

	/* Verify Reset Control Reg is enabled */
	while (!((immap->reset.rcer) & RCER_CRE))
		;

	printf ("Resetting the board.\n");
	udelay(200);

	/* Perform reset */
	immap->reset.rcr = RCR_SWHR;

	/* Unreached... */
	return 1;
}


/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */
unsigned long get_tbclk (void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return tbclk;
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts ();

	/* Reset watchdog */
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	immr->wdt.swsrr = 0x556c;
	immr->wdt.swsrr = 0xaa39;

	if (re_enable)
		enable_interrupts ();
}
#endif
