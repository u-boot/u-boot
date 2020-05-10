// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <dfu.h>
#include <image.h>
#include <asm/arch/stm32prog.h>
#include "stm32prog.h"

struct stm32prog_data *stm32prog_data;

static void enable_vidconsole(void)
{
#ifdef CONFIG_DM_VIDEO
	char *stdname;
	char buf[64];

	stdname = env_get("stdout");
	if (!stdname || !strstr(stdname, "vidconsole")) {
		if (!stdname)
			snprintf(buf, sizeof(buf), "serial,vidconsole");
		else
			snprintf(buf, sizeof(buf), "%s,vidconsole", stdname);
		env_set("stdout", buf);
	}

	stdname = env_get("stderr");
	if (!stdname || !strstr(stdname, "vidconsole")) {
		if (!stdname)
			snprintf(buf, sizeof(buf), "serial,vidconsole");
		else
			snprintf(buf, sizeof(buf), "%s,vidconsole", stdname);
		env_set("stderr", buf);
	}
#endif
}

static int do_stm32prog(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[])
{
	ulong	addr, size;
	int dev, ret;
	enum stm32prog_link_t link = LINK_UNDEFINED;
	bool reset = false;
	struct image_header_s header;
	struct stm32prog_data *data;
	u32 uimage, dtb;

	if (argc < 3 ||  argc > 5)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "usb"))
		link = LINK_USB;
	else if (!strcmp(argv[1], "serial"))
		link = LINK_SERIAL;

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

	/* check STM32IMAGE presence */
	if (size == 0 &&
	    !stm32prog_header_check((struct raw_header_s *)addr, &header)) {
		size = header.image_length + BL_HEADER_SIZE;

		/* uImage detected in STM32IMAGE, execute the script */
		if (IMAGE_FORMAT_LEGACY ==
		    genimg_get_format((void *)(addr + BL_HEADER_SIZE)))
			return image_source_script(addr + BL_HEADER_SIZE,
						   "script@1");
	}

	enable_vidconsole();

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
	case LINK_SERIAL:
		ret = stm32prog_serial_init(data, dev);
		if (ret)
			goto cleanup;
		reset = stm32prog_serial_loop(data);
		break;
	case LINK_USB:
		reset = stm32prog_usb_loop(data, dev);
		break;
	default:
		goto cleanup;
	}

	uimage = data->uimage;
	dtb = data->dtb;

	stm32prog_clean(data);
	free(stm32prog_data);
	stm32prog_data = NULL;

	puts("Download done\n");

	if (uimage) {
		char boot_addr_start[20];
		char dtb_addr[20];
		char *bootm_argv[5] = {
			"bootm", boot_addr_start, "-", dtb_addr, NULL
		};
		if (!dtb)
			bootm_argv[3] = env_get("fdtcontroladdr");
		else
			snprintf(dtb_addr, sizeof(dtb_addr) - 1,
				 "0x%x", dtb);

		snprintf(boot_addr_start, sizeof(boot_addr_start) - 1,
			 "0x%x", uimage);
		printf("Booting kernel at %s - %s...\n\n\n",
		       boot_addr_start, bootm_argv[3]);
		/* Try bootm for legacy and FIT format image */
		if (genimg_get_format((void *)uimage) != IMAGE_FORMAT_INVALID)
			do_bootm(cmdtp, 0, 4, bootm_argv);
		else if CONFIG_IS_ENABLED(CMD_BOOTZ)
			do_bootz(cmdtp, 0, 4, bootm_argv);
	}

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
	   "<link> = serial|usb\n"
	   "<dev>  = device instance\n"
	   "<addr> = address of flashlayout\n"
	   "<size> = size of flashlayout\n"
);

bool stm32prog_get_tee_partitions(void)
{
	if (stm32prog_data)
		return stm32prog_data->tee_detected;

	return false;
}

bool stm32prog_get_fsbl_nor(void)
{
	if (stm32prog_data)
		return stm32prog_data->fsbl_nor_detected;

	return false;
}
