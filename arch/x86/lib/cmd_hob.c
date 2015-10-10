/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <linux/compiler.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

static char *hob_type[] = {
	"reserved",
	"Hand-off",
	"Mem Alloc",
	"Res Desc",
	"GUID Ext",
	"FV",
	"CPU",
	"Mem Pool",
	"reserved",
	"FV2",
	"Load PEIM",
	"Capsule",
};

int do_hob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const struct hob_header *hdr;
	uint type;
	char *desc;
	int i = 0;

	hdr = gd->arch.hob_list;

	printf("HOB list address: 0x%08x\n\n", (unsigned int)hdr);

	printf("#  | Address  | Type      | Len  | ");
	printf("%42s\n", "GUID");
	printf("---|----------|-----------|------|-");
	printf("------------------------------------------\n");
	while (!end_of_hob(hdr)) {
		printf("%-2d | %08x | ", i, (unsigned int)hdr);
		type = hdr->type;
		if (type == HOB_TYPE_UNUSED)
			desc = "*Unused*";
		else if (type == HOB_TYPE_EOH)
			desc = "*EOH*";
		else if (type >= 0 && type <= ARRAY_SIZE(hob_type))
			desc = hob_type[type];
		else
			desc = "*Invalid*";
		printf("%-9s | %-4d | ", desc, hdr->len);

		if (type == HOB_TYPE_MEM_ALLOC || type == HOB_TYPE_RES_DESC ||
		    type == HOB_TYPE_GUID_EXT) {
			struct efi_guid *guid = (struct efi_guid *)(hdr + 1);
			int j;

			printf("%08x-%04x-%04x", guid->data1,
			       guid->data2, guid->data3);
			for (j = 0; j < ARRAY_SIZE(guid->data4); j++)
				printf("-%02x", guid->data4[j]);
		} else {
			printf("%42s", "Not Available");
		}
		printf("\n");
		hdr = get_next_hob(hdr);
		i++;
	}

	return 0;
}

U_BOOT_CMD(
	hob,	1,	1,	do_hob,
	"print Firmware Support Package (FSP) Hand-Off Block information",
	""
);
