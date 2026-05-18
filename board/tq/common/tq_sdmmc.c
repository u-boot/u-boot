// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright (C) 2018-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 */

#include <command.h>
#include <env.h>
#include <mmc.h>
#include <stdbool.h>
#include <vsprintf.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <linux/errno.h>

#include "tq_bb.h"

static int check_mmc_autodetect(void)
{
	/* NO or unset: 0 / YES: 1 */
	return (env_get_yesno("mmcautodetect") > 0);
}

/* This should be defined for each board */
__weak int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	u32 dev_no;

	dev_no = mmc_get_env_dev();

	if (!check_mmc_autodetect())
		return;

	env_set_ulong("mmcdev", dev_no);
	env_set_ulong("mmcblkdev", mmc_map_to_kernel_blk(dev_no));

	snprintf(cmd, ARRAY_SIZE(cmd), "mmc dev %d", dev_no);
	run_command(cmd, 0);
}
