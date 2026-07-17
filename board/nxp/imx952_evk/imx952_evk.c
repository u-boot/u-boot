// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025-2026 NXP
 */

#include <env.h>
#include <init.h>
#include <asm/arch/sys_proto.h>

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

	return 0;
}

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *fdt)
{
	/* Remove nodes based on fuses. */
	return imx9_uboot_fixup_by_fuse(fdt);
}
#endif
