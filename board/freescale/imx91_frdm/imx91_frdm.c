// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <env.h>
#include <asm/arch/sys_proto.h>

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT))
		board_late_mmc_env_init();

	env_set("sec_boot", "no");

	if (IS_ENABLED(CONFIG_AHAB_BOOT))
		env_set("sec_boot", "yes");

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "11X11_FRDM");
		env_set("board_rev", "iMX91");
	}

	return 0;
}
