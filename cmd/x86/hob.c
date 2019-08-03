// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <efi.h>
#include <asm/hob.h>

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

static int do_hob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const struct hob_header *hdr;
	uint type;
	char *desc;
	int i = 0;
	efi_guid_t *guid;
	char uuid[UUID_STR_LEN + 1];

	hdr = gd->arch.hob_list;

	printf("HOB list address: 0x%08x\n\n", (unsigned int)hdr);

	printf("#  | Address  | Type      | Len  | ");
	printf("%36s\n", "GUID");
	printf("---|----------|-----------|------|-");
	printf("------------------------------------\n");
	while (!end_of_hob(hdr)) {
		printf("%02x | %08x | ", i, (unsigned int)hdr);
		type = hdr->type;
		if (type == HOB_TYPE_UNUSED)
			desc = "*Unused*";
		else if (type == HOB_TYPE_EOH)
			desc = "*EOH*";
		else if (type >= 0 && type <= ARRAY_SIZE(hob_type))
			desc = hob_type[type];
		else
			desc = "*Invalid*";
		printf("%-9s | %04x | ", desc, hdr->len);

		if (type == HOB_TYPE_MEM_ALLOC || type == HOB_TYPE_RES_DESC ||
		    type == HOB_TYPE_GUID_EXT) {
			guid = (efi_guid_t *)(hdr + 1);
			uuid_bin_to_str(guid->b, uuid, UUID_STR_FORMAT_GUID);
			printf("%s", uuid);
		} else {
			printf("%36s", "Not Available");
		}
		printf("\n");
		hdr = get_next_hob(hdr);
		i++;
	}

	return 0;
}

U_BOOT_CMD(hob, 1, 1, do_hob,
	   "Print Hand-Off Block (HOB) information",
	   ""
);
