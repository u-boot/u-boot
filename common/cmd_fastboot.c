/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <g_dnl.h>

static int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	ret = g_dnl_register("usb_dnl_fastboot");
	if (ret)
		return ret;

	while (1) {
		if (ctrlc())
			break;
		usb_gadget_handle_interrupts();
	}

	g_dnl_unregister();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	fastboot,	1,	1,	do_fastboot,
	"fastboot - enter USB Fastboot protocol",
	""
);
