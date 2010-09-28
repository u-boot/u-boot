/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Sergei Poselenov <sposelenov@emcraft.com>
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

#include <config.h>

#if defined(CONFIG_440EP) || \
    defined(CONFIG_440EPX)

#include <asm/processor.h>
#include <asm/ppc4xx.h>


int fpu_status(void)
{
	if (mfspr(SPRN_CCR0) & CCR0_DAPUIB)
		return 0; /* Disabled */
	else
		return 1; /* Enabled */
}


void fpu_disable(void)
{
	mtspr(SPRN_CCR0, mfspr(SPRN_CCR0) | CCR0_DAPUIB);
	mtmsr(mfmsr() & ~MSR_FP);
}


void fpu_enable(void)
{
	mtspr(SPRN_CCR0, mfspr(SPRN_CCR0) & ~CCR0_DAPUIB);
	mtmsr(mfmsr() | MSR_FP);
}

#endif
