// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * Secure monitor calls.
 */

#include <common.h>
#include <asm/arch/sm.h>
#include <linux/kernel.h>

#define FN_GET_SHARE_MEM_INPUT_BASE	0x82000020
#define FN_GET_SHARE_MEM_OUTPUT_BASE	0x82000021
#define FN_EFUSE_READ			0x82000030
#define FN_EFUSE_WRITE			0x82000031
#define FN_CHIP_ID			0x82000044

static void *shmem_input;
static void *shmem_output;

static void meson_init_shmem(void)
{
	struct pt_regs regs;

	if (shmem_input && shmem_output)
		return;

	regs.regs[0] = FN_GET_SHARE_MEM_INPUT_BASE;
	smc_call(&regs);
	shmem_input = (void *)regs.regs[0];

	regs.regs[0] = FN_GET_SHARE_MEM_OUTPUT_BASE;
	smc_call(&regs);
	shmem_output = (void *)regs.regs[0];

	debug("Secure Monitor shmem: 0x%p 0x%p\n", shmem_input, shmem_output);
}

ssize_t meson_sm_read_efuse(uintptr_t offset, void *buffer, size_t size)
{
	struct pt_regs regs;

	meson_init_shmem();

	regs.regs[0] = FN_EFUSE_READ;
	regs.regs[1] = offset;
	regs.regs[2] = size;

	smc_call(&regs);

	if (regs.regs[0] == 0)
		return -1;

	memcpy(buffer, shmem_output, min(size, regs.regs[0]));

	return regs.regs[0];
}

#define SM_CHIP_ID_LENGTH	119
#define SM_CHIP_ID_OFFSET	4
#define SM_CHIP_ID_SIZE		12

int meson_sm_get_serial(void *buffer, size_t size)
{
	struct pt_regs regs;

	meson_init_shmem();

	regs.regs[0] = FN_CHIP_ID;
	regs.regs[1] = 0;
	regs.regs[2] = 0;

	smc_call(&regs);

	memcpy(buffer, shmem_output + SM_CHIP_ID_OFFSET,
	       min_t(size_t, size, SM_CHIP_ID_SIZE));

	return 0;
}

static int do_sm_serial(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	ulong address;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	address = simple_strtoul(argv[1], NULL, 0);

	ret = meson_sm_get_serial((void *)address, SM_CHIP_ID_SIZE);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_sm_sub[] = {
	U_BOOT_CMD_MKENT(serial, 2, 1, do_sm_serial, "", ""),
};

static int do_sm(cmd_tbl_t *cmdtp, int flag, int argc,
		 char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'sm' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_sm_sub[0], ARRAY_SIZE(cmd_sm_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sm, 5, 0, do_sm,
	"Secure Monitor Control",
	"serial <address> - read chip unique id to memory address"
);
