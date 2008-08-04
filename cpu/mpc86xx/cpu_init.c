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

#include <config.h>
#include <common.h>
#include <mpc86xx.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>

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
	return 0;
}

/* Set up BAT registers */
void setup_bats(void)
{
	write_bat(DBAT0, CFG_DBAT0U, CFG_DBAT0L);
	write_bat(IBAT0, CFG_IBAT0U, CFG_IBAT0L);
	write_bat(DBAT1, CFG_DBAT1U, CFG_DBAT1L);
	write_bat(IBAT1, CFG_IBAT1U, CFG_IBAT1L);
	write_bat(DBAT2, CFG_DBAT2U, CFG_DBAT2L);
	write_bat(IBAT2, CFG_IBAT2U, CFG_IBAT2L);
	write_bat(DBAT3, CFG_DBAT3U, CFG_DBAT3L);
	write_bat(IBAT3, CFG_IBAT3U, CFG_IBAT3L);
	write_bat(DBAT4, CFG_DBAT4U, CFG_DBAT4L);
	write_bat(IBAT4, CFG_IBAT4U, CFG_IBAT4L);
	write_bat(DBAT5, CFG_DBAT5U, CFG_DBAT5L);
	write_bat(IBAT5, CFG_IBAT5U, CFG_IBAT5L);
	write_bat(DBAT6, CFG_DBAT6U, CFG_DBAT6L);
	write_bat(IBAT6, CFG_IBAT6U, CFG_IBAT6L);
	write_bat(DBAT7, CFG_DBAT7U, CFG_DBAT7L);
	write_bat(IBAT7, CFG_IBAT7U, CFG_IBAT7L);

	return;
}
