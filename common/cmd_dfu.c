/*
 * cmd_dfu.c -- dfu command
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dfu.h>
#include <g_dnl.h>

static int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s = "dfu";
	int ret, i = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	ret = dfu_init_env_entities(argv[1], simple_strtoul(argv[2], NULL, 10));
	if (ret)
		return ret;

	if (argc > 3 && strcmp(argv[3], "list") == 0) {
		dfu_show_entities();
		goto done;
	}

#ifdef CONFIG_TRATS
	board_usb_init();
#endif

	g_dnl_register(s);
	while (1) {
		if (dfu_reset())
			/*
			 * This extra number of usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			if (++i == 10)
				goto exit;

		if (ctrlc())
			goto exit;

		usb_gadget_handle_interrupts();
	}
exit:
	g_dnl_unregister();
done:
	dfu_free_entities();

	if (dfu_reset())
		run_command("reset", 0);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	"<interface> <dev> [list]\n"
	"  - device firmware upgrade on a device <dev>\n"
	"    attached to interface <interface>\n"
	"    [list] - list available alt settings"
);
