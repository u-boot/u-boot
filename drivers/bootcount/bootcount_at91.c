// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_gpbr.h>
#include <bootcount.h>

/*
 * We combine the CONFIG_SYS_BOOTCOUNT_MAGIC and bootcount in one 32-bit
 * register. This is done so we need to use only one of the four GPBR
 * registers.
 */
void bootcount_store(ulong a)
{
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	writel((CONFIG_SYS_BOOTCOUNT_MAGIC & BOOTCOUNT_MAGIC_MASK) | (a & BOOTCOUNT_COUNT_MASK),
	       &gpbr->reg[AT91_GPBR_INDEX_BOOTCOUNT]);
}

ulong bootcount_load(void)
{
	at91_gpbr_t *gpbr = (at91_gpbr_t *) ATMEL_BASE_GPBR;

	ulong val = readl(&gpbr->reg[AT91_GPBR_INDEX_BOOTCOUNT]);
	if ((val & BOOTCOUNT_MAGIC_MASK) != (CONFIG_SYS_BOOTCOUNT_MAGIC & BOOTCOUNT_MAGIC_MASK))
		return 0;
	else
		return val & BOOTCOUNT_COUNT_MASK;
}
