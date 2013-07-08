/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <commproc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_ALLOC_DPRAM

int dpram_init (void)
{
	/* Reclaim the DP memory for our use. */
	gd->arch.dp_alloc_base = CPM_DATAONLY_BASE;
	gd->arch.dp_alloc_top  = CPM_DATAONLY_BASE + CPM_DATAONLY_SIZE;

	return (0);
}

/* Allocate some memory from the dual ported ram.  We may want to
 * enforce alignment restrictions, but right now everyone is a good
 * citizen.
 */
uint dpram_alloc (uint size)
{
	uint addr = gd->arch.dp_alloc_base;

	if ((gd->arch.dp_alloc_base + size) >= gd->arch.dp_alloc_top)
		return (CPM_DP_NOSPACE);

	gd->arch.dp_alloc_base += size;

	return addr;
}

uint dpram_base (void)
{
	return gd->arch.dp_alloc_base;
}

/* Allocate some memory from the dual ported ram.  We may want to
 * enforce alignment restrictions, but right now everyone is a good
 * citizen.
 */
uint dpram_alloc_align (uint size, uint align)
{
	uint addr, mask = align - 1;

	addr = (gd->arch.dp_alloc_base + mask) & ~mask;

	if ((addr + size) >= gd->arch.dp_alloc_top)
		return (CPM_DP_NOSPACE);

	gd->arch.dp_alloc_base = addr + size;

	return addr;
}

uint dpram_base_align (uint align)
{
	uint mask = align - 1;

	return (gd->arch.dp_alloc_base + mask) & ~mask;
}
#endif	/* CONFIG_SYS_ALLOC_DPRAM */
