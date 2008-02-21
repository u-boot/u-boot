/*
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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
 * cpu_init.c - low level cpu init
 */

#include <common.h>
#include <mpc86xx.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map
 * initialize a bunch of registers
 */

void cpu_init_f(void)
{
	volatile immap_t    *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_lbc_t *memctl = &immap->im_lbc;

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

#ifdef CONFIG_FSL_LAW
	init_laws();
#endif

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CFG_OR0_REMAP)
	memctl->or0 = CFG_OR0_REMAP;
#endif
#if defined(CFG_OR1_REMAP)
	memctl->or1 = CFG_OR1_REMAP;
#endif

	/* now restrict to preliminary range */
#if defined(CFG_BR0_PRELIM) && defined(CFG_OR0_PRELIM)
	memctl->br0 = CFG_BR0_PRELIM;
	memctl->or0 = CFG_OR0_PRELIM;
#endif

#if defined(CFG_BR1_PRELIM) && defined(CFG_OR1_PRELIM)
	memctl->or1 = CFG_OR1_PRELIM;
	memctl->br1 = CFG_BR1_PRELIM;
#endif

#if defined(CFG_BR2_PRELIM) && defined(CFG_OR2_PRELIM)
	memctl->or2 = CFG_OR2_PRELIM;
	memctl->br2 = CFG_BR2_PRELIM;
#endif

#if defined(CFG_BR3_PRELIM) && defined(CFG_OR3_PRELIM)
	memctl->or3 = CFG_OR3_PRELIM;
	memctl->br3 = CFG_BR3_PRELIM;
#endif

#if defined(CFG_BR4_PRELIM) && defined(CFG_OR4_PRELIM)
	memctl->or4 = CFG_OR4_PRELIM;
	memctl->br4 = CFG_BR4_PRELIM;
#endif

#if defined(CFG_BR5_PRELIM) && defined(CFG_OR5_PRELIM)
	memctl->or5 = CFG_OR5_PRELIM;
	memctl->br5 = CFG_BR5_PRELIM;
#endif

#if defined(CFG_BR6_PRELIM) && defined(CFG_OR6_PRELIM)
	memctl->or6 = CFG_OR6_PRELIM;
	memctl->br6 = CFG_BR6_PRELIM;
#endif

#if defined(CFG_BR7_PRELIM) && defined(CFG_OR7_PRELIM)
	memctl->or7 = CFG_OR7_PRELIM;
	memctl->br7 = CFG_BR7_PRELIM;
#endif

	/* enable the timebase bit in HID0 */
	set_hid0(get_hid0() | 0x4000000);

	/* enable EMCP, SYNCBE | ABE bits in HID1 */
	set_hid1(get_hid1() | 0x80000C00);
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#ifdef CONFIG_FSL_LAW
	disable_law(0);
#endif
	return 0;
}
