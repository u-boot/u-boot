/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * CBFS commands
 */
#include <common.h>
#include <command.h>
#include <cbfs.h>

int do_cbfs_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uintptr_t end_of_rom = 0xffffffff;
	char *ep;

	if (argc > 2) {
		printf("usage: cbfsls [end of rom]>\n");
		return 0;
	}
	if (argc == 2) {
		end_of_rom = (int)simple_strtoul(argv[1], &ep, 16);
		if (*ep) {
			puts("\n** Invalid end of ROM **\n");
			return 1;
		}
	}
	file_cbfs_init(end_of_rom);
	if (file_cbfs_result != CBFS_SUCCESS) {
		printf("%s.\n", file_cbfs_error());
		return 1;
	}
	return 0;
}

U_BOOT_CMD(
	cbfsinit,	2,	0,	do_cbfs_init,
	"initialize the cbfs driver",
	"[end of rom]\n"
	"    - Initialize the cbfs driver. The optional 'end of rom'\n"
	"      parameter specifies where the end of the ROM is that the\n"
	"      CBFS is in. It defaults to 0xFFFFFFFF\n"
);

int do_cbfs_fsload(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const struct cbfs_cachenode *file;
	unsigned long offset;
	unsigned long count;
	char buf[12];
	long size;

	if (argc < 3) {
		printf("usage: cbfsload <addr> <filename> [bytes]\n");
		return 1;
	}

	/* parse offset and count */
	offset = simple_strtoul(argv[1], NULL, 16);
	if (argc == 4)
		count = simple_strtoul(argv[3], NULL, 16);
	else
		count = 0;

	file = file_cbfs_find(argv[2]);
	if (!file) {
		if (file_cbfs_result == CBFS_FILE_NOT_FOUND)
			printf("%s: %s\n", file_cbfs_error(), argv[2]);
		else
			printf("%s.\n", file_cbfs_error());
		return 1;
	}

	printf("reading %s\n", file_cbfs_name(file));

	size = file_cbfs_read(file, (void *)offset, count);

	printf("\n%ld bytes read\n", size);

	sprintf(buf, "%lX", size);
	setenv("filesize", buf);

	return 0;
}

U_BOOT_CMD(
	cbfsload,	4,	0,	do_cbfs_fsload,
	"load binary file from a cbfs filesystem",
	"<addr> <filename> [bytes]\n"
	"    - load binary file 'filename' from the cbfs to address 'addr'\n"
);

int do_cbfs_ls(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const struct cbfs_cachenode *file = file_cbfs_get_first();
	int files = 0;

	if (!file) {
		printf("%s.\n", file_cbfs_error());
		return 1;
	}

	printf("     size              type  name\n");
	printf("------------------------------------------\n");
	while (file) {
		u32 type = file_cbfs_type(file);
		char *type_name = NULL;
		const char *filename = file_cbfs_name(file);

		printf(" %8d", file_cbfs_size(file));

		switch (type) {
		case CBFS_TYPE_STAGE:
			type_name = "stage";
			break;
		case CBFS_TYPE_PAYLOAD:
			type_name = "payload";
			break;
		case CBFS_TYPE_OPTIONROM:
			type_name = "option rom";
			break;
		case CBFS_TYPE_BOOTSPLASH:
			type_name = "boot splash";
			break;
		case CBFS_TYPE_RAW:
			type_name = "raw";
			break;
		case CBFS_TYPE_VSA:
			type_name = "vsa";
			break;
		case CBFS_TYPE_MBI:
			type_name = "mbi";
			break;
		case CBFS_TYPE_MICROCODE:
			type_name = "microcode";
			break;
		case CBFS_COMPONENT_CMOS_DEFAULT:
			type_name = "cmos default";
			break;
		case CBFS_COMPONENT_CMOS_LAYOUT:
			type_name = "cmos layout";
			break;
		case -1UL:
			type_name = "null";
			break;
		}
		if (type_name)
			printf("  %16s", type_name);
		else
			printf("  %16d", type);

		if (filename[0])
			printf("  %s\n", filename);
		else
			printf("  %s\n", "(empty)");
		file_cbfs_get_next(&file);
		files++;
	}

	printf("\n%d file(s)\n\n", files);
	return 0;
}

U_BOOT_CMD(
	cbfsls,	1,	1,	do_cbfs_ls,
	"list files",
	"    - list the files in the cbfs\n"
);

int do_cbfs_fsinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const struct cbfs_header *header = file_cbfs_get_header();

	if (!header) {
		printf("%s.\n", file_cbfs_error());
		return 1;
	}

	printf("\n");
	printf("CBFS version: %#x\n", header->version);
	printf("ROM size: %#x\n", header->rom_size);
	printf("Boot block size: %#x\n", header->boot_block_size);
	printf("CBFS size: %#x\n",
		header->rom_size - header->boot_block_size - header->offset);
	printf("Alignment: %d\n", header->align);
	printf("Offset: %#x\n", header->offset);
	printf("\n");

	return 0;
}

U_BOOT_CMD(
	cbfsinfo,	1,	1,	do_cbfs_fsinfo,
	"print information about filesystem",
	"    - print information about the cbfs filesystem\n"
);
