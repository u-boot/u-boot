// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2018 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

/*
 * This is a board specific file.  It's OK to include board specific
 * header files
 */

#include <common.h>
#include <config.h>
#include <init.h>
#include <dm/lists.h>
#include <fdtdec.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
};

int board_late_init(void)
{
	ulong max_size, lowmem_size;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SYSRESET_MICROBLAZE)
	int ret;

	ret = device_bind_driver(gd->dm_root, "mb_soft_reset",
				 "reset_soft", NULL);
	if (ret)
		printf("Warning: No reset driver: ret=%d\n", ret);
#endif

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	max_size = gd->start_addr_sp - CONFIG_STACK_SIZE;
	max_size = round_down(max_size, SZ_16M);

	/* Linux default LOWMEM_SIZE is 0x30000000 = 768MB */
	lowmem_size = gd->ram_base + 768 * 1024 * 1024;

	env_set_addr("initrd_high", (void *)min_t(ulong, max_size,
						  lowmem_size));
	env_set_addr("fdt_high", (void *)min_t(ulong, max_size, lowmem_size));

	return 0;
}
