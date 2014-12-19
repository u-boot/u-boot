/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <linux/compiler.h>
#include <asm/arch/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

static char *hob_type[] = {
	"reserved",
	"Hand-off",
	"Memory Allocation",
	"Resource Descriptor",
	"GUID Extension",
	"Firmware Volume",
	"CPU",
	"Memory Pool",
	"reserved",
	"Firmware Volume 2",
	"Load PEIM Unused",
	"UEFI Capsule",
};

int do_hob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	union hob_pointers hob;
	u16 type;
	char *desc;
	int i = 0;

	hob.raw = (u8 *)gd->arch.hob_list;

	printf("HOB list address: 0x%08x\n\n", (unsigned int)hob.raw);

	printf("No. | Address  | Type                | Length in Bytes\n");
	printf("----|----------|---------------------|----------------\n");
	while (!end_of_hob(hob)) {
		printf("%-3d | %08x | ", i, (unsigned int)hob.raw);
		type = get_hob_type(hob);
		if (type == HOB_TYPE_UNUSED)
			desc = "*Unused*";
		else if (type == HOB_TYPE_EOH)
			desc = "*END OF HOB*";
		else if (type >= 0 && type <= ARRAY_SIZE(hob_type))
			desc = hob_type[type];
		else
			desc = "*Invalid Type*";
		printf("%-19s | %-15d\n", desc, get_hob_length(hob));
		hob.raw = get_next_hob(hob);
		i++;
	}

	return 0;
}

U_BOOT_CMD(
	hob,	1,	1,	do_hob,
	"print Firmware Support Package (FSP) Hand-Off Block information",
	""
);
