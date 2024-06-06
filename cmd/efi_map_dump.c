// SPDX-License-Identifier: GPL-2.0+
/*
 *  Allocate and Free EFI memory
 *
 *  Copyright (c) 2024 Linaro Limited
 */

#include <command.h>
#include <efi_loader.h>

static int do_efi_map_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
//	printf("%s: argc => %d\n", __func__, argc);

	if (argc != 1)
		return CMD_RET_USAGE;

	dump_efi_memory_map();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	efi_map_dump, 1, 0, do_efi_map_dump,
	"Dump the EFI memory map",
	""
);
