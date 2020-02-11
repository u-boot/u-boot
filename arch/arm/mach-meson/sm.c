// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * Secure monitor calls.
 */

#include <common.h>
#include <asm/arch/sm.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <regmap.h>
#include <syscon.h>

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

#define AO_SEC_SD_CFG15		0xfc
#define REBOOT_REASON_MASK	GENMASK(15, 12)

int meson_sm_get_reboot_reason(void)
{
	struct regmap *regmap;
	int nodeoffset;
	ofnode node;
	unsigned int reason;

	/* find the offset of compatible node */
	nodeoffset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						   "amlogic,meson-gx-ao-secure");
	if (nodeoffset < 0) {
		printf("%s: failed to get amlogic,meson-gx-ao-secure\n",
		       __func__);
		return -ENODEV;
	}

	/* get regmap from the syscon node */
	node = offset_to_ofnode(nodeoffset);
	regmap = syscon_node_to_regmap(node);
	if (IS_ERR(regmap)) {
		printf("%s: failed to get regmap\n", __func__);
		return -EINVAL;
	}

	regmap_read(regmap, AO_SEC_SD_CFG15, &reason);

	/* The SMC call is not used, we directly use AO_SEC_SD_CFG15 */
	return FIELD_GET(REBOOT_REASON_MASK, reason);
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

#define MAX_REBOOT_REASONS 14

static const char *reboot_reasons[MAX_REBOOT_REASONS] = {
	[REBOOT_REASON_COLD] = "cold_boot",
	[REBOOT_REASON_NORMAL] = "normal",
	[REBOOT_REASON_RECOVERY] = "recovery",
	[REBOOT_REASON_UPDATE] = "update",
	[REBOOT_REASON_FASTBOOT] = "fastboot",
	[REBOOT_REASON_SUSPEND_OFF] = "suspend_off",
	[REBOOT_REASON_HIBERNATE] = "hibernate",
	[REBOOT_REASON_BOOTLOADER] = "bootloader",
	[REBOOT_REASON_SHUTDOWN_REBOOT] = "shutdown_reboot",
	[REBOOT_REASON_RPMBP] = "rpmbp",
	[REBOOT_REASON_CRASH_DUMP] = "crash_dump",
	[REBOOT_REASON_KERNEL_PANIC] = "kernel_panic",
	[REBOOT_REASON_WATCHDOG_REBOOT] = "watchdog_reboot",
};

static int do_sm_reboot_reason(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	const char *reason_str;
	char *destarg = NULL;
	int reason;

	if (argc > 1)
		destarg = argv[1];

	reason = meson_sm_get_reboot_reason();
	if (reason < 0)
		return CMD_RET_FAILURE;

	if (reason >= MAX_REBOOT_REASONS ||
	    !reboot_reasons[reason])
		reason_str = "unknown";
	else
		reason_str = reboot_reasons[reason];

	if (destarg)
		env_set(destarg, reason_str);
	else
		printf("reboot reason: %s (%x)\n", reason_str, reason);

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_sm_sub[] = {
	U_BOOT_CMD_MKENT(serial, 2, 1, do_sm_serial, "", ""),
	U_BOOT_CMD_MKENT(reboot_reason, 1, 1, do_sm_reboot_reason, "", ""),
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
	"serial <address> - read chip unique id to memory address\n"
	"sm reboot_reason [name] - get reboot reason and store to to environment"
);
