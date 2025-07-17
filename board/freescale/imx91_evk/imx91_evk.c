// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 NXP
 */

#include <env.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>

int board_late_init(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "11X11_EVK");
	env_set("board_rev", "iMX93");
#endif
	return 0;
}
