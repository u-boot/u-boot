/*
 *
 * Common functions for OMAP4/5 based boards
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/cache.h>

DECLARE_GLOBAL_DATA_PTR;

#define ARMV7_DCACHE_WRITEBACK  0xe
#define ARMV7_DOMAIN_CLIENT	1
#define ARMV7_DOMAIN_MASK	(0x3 << 0)

void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}

void dram_bank_mmu_setup(int bank)
{
	bd_t *bd = gd->bd;
	int	i;

	u32 start = bd->bi_dram[bank].start >> 20;
	u32 size = bd->bi_dram[bank].size >> 20;
	u32 end = start + size;

	debug("%s: bank: %d\n", __func__, bank);
	for (i = start; i < end; i++)
		set_section_dcache(i, ARMV7_DCACHE_WRITEBACK);
}

void arm_init_domains(void)
{
	u32 reg;

	reg = get_dacr();
	/*
	* Set DOMAIN to client access so that all permissions
	* set in pagetables are validated by the mmu.
	*/
	reg &= ~ARMV7_DOMAIN_MASK;
	reg |= ARMV7_DOMAIN_CLIENT;
	set_dacr(reg);
}
