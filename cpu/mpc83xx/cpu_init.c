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
 *           Initial file creating (porting from 85XX & 8260)
 */

#include <common.h>
#include <mpc83xx.h>
#include <ioports.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * im)
{
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/* RSR - Reset Status Register - clear all status (4.6.1.3) */
	gd->reset_status = im->reset.rsr;
	im->reset.rsr = ~(RSR_RES);

	/*
	 * RMR - Reset Mode Register
	 * contains checkstop reset enable (4.6.1.4)
	 */
	im->reset.rmr = (RMR_CSRE & (1<<RMR_CSRE_SHIFT));

	/* LCRR - Clock Ratio Register (10.3.1.16) */
	im->lbus.lcrr = CFG_LCRR;

	/* Enable Time Base & Decrimenter ( so we will have udelay() )*/
	im->sysconf.spcr |= SPCR_TBEN;

	/* System General Purpose Register */
#ifdef CFG_SICRH
	im->sysconf.sicrh = CFG_SICRH;
#endif
#ifdef CFG_SICRL
	im->sysconf.sicrl = CFG_SICRL;
#endif

	/*
	 * Memory Controller:
	 */

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CFG_BR0_PRELIM)  \
	&& defined(CFG_OR0_PRELIM) \
	&& defined(CFG_LBLAWBAR0_PRELIM) \
	&& defined(CFG_LBLAWAR0_PRELIM)
	im->lbus.bank[0].br = CFG_BR0_PRELIM;
	im->lbus.bank[0].or = CFG_OR0_PRELIM;
	im->sysconf.lblaw[0].bar = CFG_LBLAWBAR0_PRELIM;
	im->sysconf.lblaw[0].ar = CFG_LBLAWAR0_PRELIM;
#else
#error 	CFG_BR0_PRELIM, CFG_OR0_PRELIM, CFG_LBLAWBAR0_PRELIM & CFG_LBLAWAR0_PRELIM must be defined
#endif

#if defined(CFG_BR1_PRELIM) && defined(CFG_OR1_PRELIM)
	im->lbus.bank[1].br = CFG_BR1_PRELIM;
	im->lbus.bank[1].or = CFG_OR1_PRELIM;
#endif
#if defined(CFG_LBLAWBAR1_PRELIM) && defined(CFG_LBLAWAR1_PRELIM)
	im->sysconf.lblaw[1].bar = CFG_LBLAWBAR1_PRELIM;
	im->sysconf.lblaw[1].ar = CFG_LBLAWAR1_PRELIM;
#endif
#if defined(CFG_BR2_PRELIM) && defined(CFG_OR2_PRELIM)
	im->lbus.bank[2].br = CFG_BR2_PRELIM;
	im->lbus.bank[2].or = CFG_OR2_PRELIM;
#endif
#if defined(CFG_LBLAWBAR2_PRELIM) && defined(CFG_LBLAWAR2_PRELIM)
	im->sysconf.lblaw[2].bar = CFG_LBLAWBAR2_PRELIM;
	im->sysconf.lblaw[2].ar = CFG_LBLAWAR2_PRELIM;
#endif
#if defined(CFG_BR3_PRELIM) && defined(CFG_OR3_PRELIM)
	im->lbus.bank[3].br = CFG_BR3_PRELIM;
	im->lbus.bank[3].or = CFG_OR3_PRELIM;
#endif
#if defined(CFG_LBLAWBAR3_PRELIM) && defined(CFG_LBLAWAR3_PRELIM)
	im->sysconf.lblaw[3].bar = CFG_LBLAWBAR3_PRELIM;
	im->sysconf.lblaw[3].ar = CFG_LBLAWAR3_PRELIM;
#endif
#if defined(CFG_BR4_PRELIM) && defined(CFG_OR4_PRELIM)
	im->lbus.bank[4].br = CFG_BR4_PRELIM;
	im->lbus.bank[4].or = CFG_OR4_PRELIM;
#endif
#if defined(CFG_LBLAWBAR4_PRELIM) && defined(CFG_LBLAWAR4_PRELIM)
	im->sysconf.lblaw[4].bar = CFG_LBLAWBAR4_PRELIM;
	im->sysconf.lblaw[4].ar = CFG_LBLAWAR4_PRELIM;
#endif
#if defined(CFG_BR5_PRELIM) && defined(CFG_OR5_PRELIM)
	im->lbus.bank[5].br = CFG_BR5_PRELIM;
	im->lbus.bank[5].or = CFG_OR5_PRELIM;
#endif
#if defined(CFG_LBLAWBAR5_PRELIM) && defined(CFG_LBLAWAR5_PRELIM)
	im->sysconf.lblaw[5].bar = CFG_LBLAWBAR5_PRELIM;
	im->sysconf.lblaw[5].ar = CFG_LBLAWAR5_PRELIM;
#endif
#if defined(CFG_BR6_PRELIM) && defined(CFG_OR6_PRELIM)
	im->lbus.bank[6].br = CFG_BR6_PRELIM;
	im->lbus.bank[6].or = CFG_OR6_PRELIM;
#endif
#if defined(CFG_LBLAWBAR6_PRELIM) && defined(CFG_LBLAWAR6_PRELIM)
	im->sysconf.lblaw[6].bar = CFG_LBLAWBAR6_PRELIM;
	im->sysconf.lblaw[6].ar = CFG_LBLAWAR6_PRELIM;
#endif
#if defined(CFG_BR7_PRELIM) && defined(CFG_OR7_PRELIM)
	im->lbus.bank[7].br = CFG_BR7_PRELIM;
	im->lbus.bank[7].or = CFG_OR7_PRELIM;
#endif
#if defined(CFG_LBLAWBAR7_PRELIM) && defined(CFG_LBLAWAR7_PRELIM)
	im->sysconf.lblaw[7].bar = CFG_LBLAWBAR7_PRELIM;
	im->sysconf.lblaw[7].ar = CFG_LBLAWAR7_PRELIM;
#endif
#ifdef CFG_GPIO1_PRELIM
	im->pgio[0].dir = CFG_GPIO1_DIR;
	im->pgio[0].dat = CFG_GPIO1_DAT;
#endif
#ifdef CFG_GPIO2_PRELIM
	im->pgio[1].dir = CFG_GPIO2_DIR;
	im->pgio[1].dat = CFG_GPIO2_DAT;
#endif
}


/*
 * Initialize higher level parts of CPU like time base and timers.
 */

int cpu_init_r (void)
{
	return 0;
}
