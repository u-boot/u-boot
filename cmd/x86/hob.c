// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <efi.h>
#include <uuid.h>
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

static int do_hob(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct hob_header *hdr;
	uint type;
	char *desc;
	int i = 0;
	efi_guid_t *guid;
	char uuid[UUID_STR_LEN + 1];
	int seq = -1;	/* Show all by default */

	argc--;
	argv++;
	if (argc)
		seq = simple_strtol(*argv, NULL, 16);
	hdr = gd->arch.hob_list;

	printf("HOB list address: 0x%08x\n\n", (unsigned int)hdr);

	printf("#  | Address  | Type      | Len  | ");
	printf("%36s\n", "GUID");
	printf("---|----------|-----------|------|-");
	printf("------------------------------------\n");
	for (i = 0; !end_of_hob(hdr); i++, hdr = get_next_hob(hdr)) {
		if (seq != -1 && seq != i)
			continue;
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
	}

	return 0;
}

U_BOOT_CMD(hob, 2, 1, do_hob,
	   "[seq]  Print Hand-Off Block (HOB) information"
	   "   seq - Record # to show (all by default)",
	   ""
);
