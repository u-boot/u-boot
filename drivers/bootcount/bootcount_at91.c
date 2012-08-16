/*
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
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_gpbr.h>

/*
 * We combine the BOOTCOUNT_MAGIC and bootcount in one 32-bit register.
 * This is done so we need to use only one of the four GPBR registers.
 */
void bootcount_store(ulong a)
{
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	writel((BOOTCOUNT_MAGIC & 0xffff0000) | (a & 0x0000ffff),
		&gpbr->reg[AT91_GPBR_INDEX_BOOTCOUNT]);
}

ulong bootcount_load(void)
{
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	ulong val = readl(&gpbr->reg[AT91_GPBR_INDEX_BOOTCOUNT]);
	if ((val & 0xffff0000) != (BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return val & 0x0000ffff;
}
