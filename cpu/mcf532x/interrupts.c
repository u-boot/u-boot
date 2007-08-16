/*
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

/* CPU specific interrupt routine */
#include <common.h>
#include <asm/immap.h>

int interrupt_init(void)
{
	volatile int0_t *intp = (int0_t *) (CFG_INTR_BASE);

	/* Make sure all interrupts are disabled */
	intp->imrh0 |= 0xFFFFFFFF;
	intp->imrl0 |= 0xFFFFFFFF;

	enable_interrupts();
	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	volatile int0_t *intp = (int0_t *) (CFG_INTR_BASE);

	intp->icr0[CFG_TMRINTR_NO] = CFG_TMRINTR_PRI;
	intp->imrh0 &= ~CFG_TMRINTR_MASK;
}
#endif
