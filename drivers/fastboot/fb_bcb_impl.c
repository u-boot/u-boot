// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 GlobalLogic.
 * Roman Kovalivskyi <roman.kovalivskyi@globallogic.com>
 */

#include <common.h>
#include <fastboot.h>

/**
 * fastboot_set_reboot_flag() - Set flag to indicate reboot-bootloader
 *
 * Set flag which indicates that we should reboot into the bootloader
 * following the reboot that fastboot executes after this function.
 *
 * This function should be overridden in your board file with one
 * which sets whatever flag your board specific Android bootloader flow
 * requires in order to re-enter the bootloader.
 */
int fastboot_set_reboot_flag(enum fastboot_reboot_reason reason)
{
	char cmd[64];

	if (reason >= FASTBOOT_REBOOT_REASONS_COUNT)
		return -EINVAL;

	snprintf(cmd, sizeof(cmd), "bcb load %d misc",
		 CONFIG_FASTBOOT_FLASH_MMC_DEV);

	if (run_command(cmd, 0))
		return -ENODEV;

	snprintf(cmd, sizeof(cmd), "bcb set command %s",
		 fastboot_boot_cmds[reason]);

	if (run_command(cmd, 0))
		return -ENOEXEC;

	if (run_command("bcb store", 0))
		return -EIO;

	return 0;
}
