// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2024, Advanced Micro Devices, Inc.
 */
#include <command.h>
#include <errno.h>
#include <tee.h>
#include <vsprintf.h>
#include <linux/string.h>

#define TA_HELLO_WORLD_CMD_INC_VALUE 0
/* This needs to match the UUID of the Hello World TA. */
#define TA_HELLO_WORLD_UUID \
	{ 0x8aaaf200, 0x2450, 0x11e4, \
	{ 0xab, 0xe2, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }

static int hello_world_ta(unsigned int value)
{
	const struct tee_optee_ta_uuid uuid = TA_HELLO_WORLD_UUID;
	struct tee_open_session_arg session_arg;
	struct udevice *tee = NULL;
	struct tee_invoke_arg arg;
	struct tee_param param[2];
	int rc;

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		return -ENODEV;

	memset(&session_arg, 0, sizeof(session_arg));
	tee_optee_ta_uuid_to_octets(session_arg.uuid, &uuid);
	rc = tee_open_session(tee, &session_arg, 0, NULL);
	if (rc) {
		printf("tee_open_session(): failed(%d)\n", rc);
		return rc;
	}

	arg.func = TA_HELLO_WORLD_CMD_INC_VALUE;
	arg.session = session_arg.session;

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INOUT;
	param[0].u.value.a = value;

	printf("Value before: 0x%x\n", (int)param[0].u.value.a);
	printf("Calling TA\n");
	tee_invoke_func(tee, &arg, 1, param);

	printf("Value after: 0x%x\n", (int)param[0].u.value.a);
	return tee_close_session(tee, session_arg.session);
}

static int do_optee_hello_world_ta(struct cmd_tbl *cmdtp, int flag, int argc,
				   char * const argv[])
{
	int ret, value = 0;

	if (argc > 1)
		value = hextoul(argv[1], NULL);

	ret = hello_world_ta(value);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(optee,
	"hello [<value>]   Invoke the OP-TEE 'Hello World' TA\n");

U_BOOT_CMD_WITH_SUBCMDS(optee, "OP-TEE commands", optee_help_text,
	U_BOOT_SUBCMD_MKENT(hello, 2, 1, do_optee_hello_world_ta));
