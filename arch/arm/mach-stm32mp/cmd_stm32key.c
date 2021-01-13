// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <log.h>
#include <misc.h>
#include <dm/device.h>
#include <dm/uclass.h>

#define STM32_OTP_HASH_KEY_START 24
#define STM32_OTP_HASH_KEY_SIZE 8

static void read_hash_value(u32 addr)
{
	int i;

	for (i = 0; i < STM32_OTP_HASH_KEY_SIZE; i++) {
		printf("OTP value %i: %x\n", STM32_OTP_HASH_KEY_START + i,
		       __be32_to_cpu(*(u32 *)addr));
		addr += 4;
	}
}

static void fuse_hash_value(u32 addr, bool print)
{
	struct udevice *dev;
	u32 word, val;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret) {
		log_err("Can't find stm32mp_bsec driver\n");
		return;
	}

	for (i = 0; i < STM32_OTP_HASH_KEY_SIZE; i++) {
		if (print)
			printf("Fuse OTP %i : %x\n",
			       STM32_OTP_HASH_KEY_START + i,
			       __be32_to_cpu(*(u32 *)addr));

		word = STM32_OTP_HASH_KEY_START + i;
		val = __be32_to_cpu(*(u32 *)addr);
		misc_write(dev, STM32_BSEC_OTP(word), &val, 4);

		addr += 4;
	}
}

static int confirm_prog(void)
{
	puts("Warning: Programming fuses is an irreversible operation!\n"
			"         This may brick your system.\n"
			"         Use this command only if you are sure of what you are doing!\n"
			"\nReally perform this fuse programming? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Fuse programming aborted\n");
	return 0;
}

static int do_stm32key(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	u32 addr;
	const char *op = argc >= 2 ? argv[1] : NULL;
	int confirmed = argc > 3 && !strcmp(argv[2], "-y");

	argc -= 2 + confirmed;
	argv += 2 + confirmed;

	if (argc < 1)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[0], NULL, 16);
	if (!addr)
		return CMD_RET_USAGE;

	if (!strcmp(op, "read"))
		read_hash_value(addr);

	if (!strcmp(op, "fuse")) {
		if (!confirmed && !confirm_prog())
			return CMD_RET_FAILURE;
		fuse_hash_value(addr, !confirmed);
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(stm32key, 4, 1, do_stm32key,
	   "Fuse ST Hash key",
	   "read <addr>: Read the hash store at addr in memory\n"
	   "stm32key fuse [-y] <addr> : Fuse hash store at addr in otp\n");
