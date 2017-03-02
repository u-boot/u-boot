/*
 * Copyright (C) 2017 Xilinx Inc.
 *
 * Command for authenticating the images using RSA.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <rsa.h>

__weak int rsa_hw(u8 *key_ptr, u8 *src_ptr, u32 len)
{
	return CMD_RET_FAILURE;
}

/**
 * do_rsa() - Handle the "rsa" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_rsa(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uint32_t key_addr, src_addr, len;
	uint8_t *key_ptr, *src_ptr;
	int ret = CMD_RET_SUCCESS;

	if (argc != 5)
		return CMD_RET_USAGE;

	key_addr = simple_strtoul(argv[1], NULL, 16);
	src_addr = simple_strtoul(argv[2], NULL, 16);
	len = simple_strtoul(argv[3], NULL, 16);

	if (!strncmp(argv[4], "hw", 2))
		return CMD_RET_USAGE;

	key_ptr = (uint8_t *)(uintptr_t)key_addr;
	src_ptr = (uint8_t *)(uintptr_t)src_addr;

	ret = rsa_hw(key_ptr, src_ptr, len);

	return ret;
}

/***************************************************/
#ifdef CONFIG_SYS_LONGHELP
static char rsa_help_text[] =
	"key src len hw - Authenticate a block of data $len bytes\n"
	"                 long at address $src using a key at\n"
	"                 address $key address. The Key is a\n"
	"                 combination of signature of image to\n"
	"                 authenticate followed by public key to\n"
	"                 be used for authentication. The hw flag\n"
	"                 specifies to used hardware engine if supports\n";
#endif

U_BOOT_CMD(
	rsa, 5, 1, do_rsa,
	"Authentication of image using RSA",
	rsa_help_text
)
