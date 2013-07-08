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
#include <command.h>
#include <malloc.h>
#include <dfu.h>
#include <asm/errno.h>
#include <g_dnl.h>

static int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *str_env;
	char *s = "dfu";
	char *env_bkp;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;

	str_env = getenv("dfu_alt_info");
	if (str_env == NULL) {
		printf("%s: \"dfu_alt_info\" env variable not defined!\n",
		       __func__);
		return CMD_RET_FAILURE;
	}

	env_bkp = strdup(str_env);
	ret = dfu_config_entities(env_bkp, argv[1],
			    (int)simple_strtoul(argv[2], NULL, 10));
	if (ret)
		return CMD_RET_FAILURE;

	if (argc > 3 && strcmp(argv[3], "list") == 0) {
		dfu_show_entities();
		goto done;
	}

#ifdef CONFIG_TRATS
	board_usb_init();
#endif

	g_dnl_register(s);
	while (1) {
		if (ctrlc())
			goto exit;

		usb_gadget_handle_interrupts();
	}
exit:
	g_dnl_unregister();
done:
	dfu_free_entities();
	free(env_bkp);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	"<interface> <dev> [list]\n"
	"  - device firmware upgrade on a device <dev>\n"
	"    attached to interface <interface>\n"
	"    [list] - list available alt settings"
);
