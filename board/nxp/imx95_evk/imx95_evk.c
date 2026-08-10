// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	return 0;
}

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *fdt)
{
	/* Remove nodes based on fuses. */
	return imx9_uboot_fixup_by_fuse(fdt);
}
#endif
