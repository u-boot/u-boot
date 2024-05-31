// SPDX-License-Identifier: GPL-2.0+
/*
 *  Allocate and Free EFI memory
 *
 *  Copyright (c) 2024 Linaro Limited
 */

#include <command.h>
#include <efi_loader.h>
#include <lmb.h>
#include <vsprintf.h>

#include <linux/types.h>

static int do_efi_mem_free(struct cmd_tbl *cmdtp, int flag, int argc,
			   char * const argv[])
{
	uint64_t addr = 0, size = 0;
	efi_uintn_t pages;
	efi_status_t status;

	if (argc != 3)
		return CMD_RET_USAGE;

	argc--; argv++;

	size = simple_strtoul(argv[0], NULL, 16);
	if (!size) {
		printf("Enter valid size for free in Hex\n");
		return CMD_RET_USAGE;
	}

	addr = simple_strtoul(argv[1], NULL, 16);
	if (!addr) {
		printf("Enter a valid address in Hex\n");
		return CMD_RET_USAGE;
	}

	pages = efi_size_in_pages(size + (addr & EFI_PAGE_MASK));

	status = efi_free_pages(addr, pages);
	if (status != EFI_SUCCESS) {
		printf("Unable to free memory, error (%#lx)\n", status);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_efi_mem_alloc(struct cmd_tbl *cmdtp, int flag, int argc,
			    char * const argv[])
{
	enum efi_allocate_type type;
	uint64_t addr = 0, size = 0;
	efi_uintn_t pages;
	efi_status_t status;
	bool max = false;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--; argv++;

	if (!strcmp("max", argv[0])) {
		if (argc != 3)
			return CMD_RET_USAGE;

		max = true;
		argv++;
		argc--;
	}

	size = simple_strtoul(argv[0], NULL, 16);
	if (!size) {
		printf("Enter valid size for allocation in Hex\n");
		return CMD_RET_USAGE;
	}

	if (max || argc == 2) {
		addr = simple_strtoul(argv[1], NULL, 16);
		if (!addr) {
			printf("Enter a valid address in Hex\n");
			return CMD_RET_USAGE;
		}
	}

	if (max)
		type = EFI_ALLOCATE_MAX_ADDRESS;
	else if (addr)
		type = EFI_ALLOCATE_ADDRESS;
	else
		type = EFI_ALLOCATE_ANY_PAGES;

	pages = efi_size_in_pages(size + (addr & EFI_PAGE_MASK));
	status = efi_allocate_pages(type, EFI_BOOT_SERVICES_DATA, pages,
				    &addr);
	if (status != EFI_SUCCESS) {
		printf("efi_allocate_pages failed %lx\n", status);
		return CMD_RET_FAILURE;
	} else {
		printf("Address returned %#llx\n", addr);
	}

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_efi_mem_sub[] = {
	U_BOOT_CMD_MKENT(alloc, 3, 0, do_efi_mem_alloc,
		"", ""),
	U_BOOT_CMD_MKENT(free, 2, 0, do_efi_mem_free,
		"", ""),
};

static int do_efi_mem(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct cmd_tbl *cp;
	efi_status_t r;

	if (argc < 3)
		return CMD_RET_USAGE;

	argc--; argv++;

	/* Initialize UEFI subsystem */
	r = efi_init_obj_list();
	if (r != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       r & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	cp = find_cmd_tbl(argv[0], cmd_efi_mem_sub,
			  ARRAY_SIZE(cmd_efi_mem_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_LONGHELP(efi_mem,
	"Functions to allocate and free memory\n"
	"\n"
	"efi_mem alloc <size> [addr]\n"
	"efi_mem alloc max <size> <max-addr>\n"
	"efi_mem free <size> <addr>\n"
	"\n"
);

U_BOOT_CMD(
	efi_mem, CONFIG_SYS_MAXARGS, 0, do_efi_mem,
	"Allocate and free EFI memory",
	efi_mem_help_text
);
