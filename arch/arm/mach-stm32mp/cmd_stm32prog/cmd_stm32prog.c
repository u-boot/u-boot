// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <dfu.h>
#include "stm32prog.h"

struct stm32prog_data *stm32prog_data;

static int do_stm32prog(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	ulong	addr, size;
	int dev, ret;
	enum stm32prog_link_t link = LINK_UNDEFINED;
	bool reset = false;
	struct stm32prog_data *data;

	if (argc < 3 ||  argc > 5)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "usb"))
		link = LINK_USB;

	if (link == LINK_UNDEFINED) {
		pr_err("not supported link=%s\n", argv[1]);
		return CMD_RET_USAGE;
	}
	dev = (int)simple_strtoul(argv[2], NULL, 10);

	addr = STM32_DDR_BASE;
	size = 0;
	if (argc > 3) {
		addr = simple_strtoul(argv[3], NULL, 16);
		if (!addr)
			return CMD_RET_FAILURE;
	}
	if (argc > 4)
		size = simple_strtoul(argv[4], NULL, 16);

	data = (struct stm32prog_data *)malloc(sizeof(*data));

	if (!data) {
		pr_err("Alloc failed.");
		return CMD_RET_FAILURE;
	}
	stm32prog_data = data;

	ret = stm32prog_init(data, addr, size);
	if (ret)
		printf("Invalid or missing layout file.");

	/* prepare DFU for device read/write */
	ret = stm32prog_dfu_init(data);
	if (ret)
		goto cleanup;

	switch (link) {
	case LINK_USB:
		reset = stm32prog_usb_loop(data, dev);
		break;
	default:
		goto cleanup;
	}

	stm32prog_clean(data);
	free(stm32prog_data);
	stm32prog_data = NULL;

	puts("Download done\n");
	if (reset) {
		puts("Reset...\n");
		run_command("reset", 0);
	}

	return CMD_RET_SUCCESS;

cleanup:
	stm32prog_clean(data);
	free(stm32prog_data);
	stm32prog_data = NULL;

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(stm32prog, 5, 0, do_stm32prog,
	   "<link> <dev> [<addr>] [<size>]\n"
	   "start communication with tools STM32Cubeprogrammer on <link> with Flashlayout at <addr>",
	   "<link> = usb\n"
	   "<dev>  = device instance\n"
	   "<addr> = address of flashlayout\n"
	   "<size> = size of flashlayout\n"
);
