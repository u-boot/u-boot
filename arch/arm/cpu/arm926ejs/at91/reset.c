/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_rstc.h>

/* Reset the cpu by telling the reset controller to do so */
void reset_cpu(ulong ignored)
{
	at91_rstc_t *rstc = (at91_rstc_t *) ATMEL_BASE_RSTC;

	writel(AT91_RSTC_KEY
		| AT91_RSTC_CR_PROCRST	/* Processor Reset */
		| AT91_RSTC_CR_PERRST	/* Peripheral Reset */
#ifdef CONFIG_AT91RESET_EXTRST
		| AT91_RSTC_CR_EXTRST	/* External Reset (assert nRST pin) */
#endif
		, &rstc->cr);
	/* never reached */
	while (1)
		;
}
