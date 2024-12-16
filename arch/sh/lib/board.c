// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Vladimir Zapolskiy <vz@mleia.com>
 */

#include <config.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);

	return 0;
}

void relocate_code(ulong start_addr_sp, gd_t *new_gd, ulong relocaddr)
{
	if (new_gd->reloc_off)
		memcpy((void *)new_gd->relocaddr,
		       (void *)(new_gd->relocaddr - new_gd->reloc_off),
		       new_gd->mon_len);

	__asm__ __volatile__("mov.l %0, r15\n" : : "m" (new_gd->start_addr_sp));

	while (1)
		board_init_r(new_gd, 0x0);
}
