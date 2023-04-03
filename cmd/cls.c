// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * cls - clear screen command
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <video_console.h>

#define CSI "\x1b["

static int do_video_clear(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	__maybe_unused struct udevice *dev;

	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	if (IS_ENABLED(CONFIG_VIDEO_ANSI))
		return 0;

	if (IS_ENABLED(CONFIG_VIDEO)) {
		if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
			return CMD_RET_FAILURE;
		if (vidconsole_clear_and_reset(dev))
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(cls,	1, 1, do_video_clear, "clear screen", "");
