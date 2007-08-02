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
 *
 * Derived from the MPC83xx code.
 *
 */

#include <common.h>
#include <mpc512x.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Set up the memory map, initialize registers,
 */
void cpu_init_f (volatile immap_t * im)
{
	u32 ips_div;

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/* system performance tweaking */

#ifdef CFG_ACR_PIPE_DEP
	/* Arbiter pipeline depth */
	im->arbiter.acr = (im->arbiter.acr & ~ACR_PIPE_DEP) |
			  (CFG_ACR_PIPE_DEP << ACR_PIPE_DEP_SHIFT);
#endif

#ifdef CFG_ACR_RPTCNT
	/* Arbiter repeat count */
	im->arbiter.acr = ((im->arbiter.acr & ~(ACR_RPTCNT)) |
			   (CFG_ACR_RPTCNT << ACR_RPTCNT_SHIFT));
#endif

	/* RSR - Reset Status Register - clear all status */
	gd->reset_status = im->reset.rsr;
	im->reset.rsr = ~(RSR_RES);

	/*
	 * RMR - Reset Mode Register - enable checkstop reset
	 */
	im->reset.rmr = (RMR_CSRE & (1 << RMR_CSRE_SHIFT));

	/* Set IPS-CSB divider: IPS = 1/2 CSB */
	ips_div = im->clk.scfr[0];
	ips_div &= ~(SCFR1_IPS_DIV_MASK);
	ips_div |= SCFR1_IPS_DIV << SCFR1_IPS_DIV_SHIFT;
	im->clk.scfr[0] = ips_div;

	/*
	 * Enable Time Base/Decrementer
	 *
	 * NOTICE: TB needs to be enabled as early as possible in order to
	 * have udelay() working; if not enabled, usually leads to a hang, like
	 * during FLASH chip identification etc.
	 */
	im->sysconf.spcr |= SPCR_TBEN;
}

int cpu_init_r (void)
{
	return 0;
}
