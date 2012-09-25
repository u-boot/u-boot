/*
 * (C) Copyright 2008-2009 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

int romfs_check (int info);
int romfs_load (int *loadoffset, int info, char *filename);
int romfs_ls (int info, char *filename);
int romfs_info (int info);
int romfs_cat (int info, char *filename);

u32 address; /* physical address of fs */

/**
 * Routine implementing fsload u-boot command. This routine tries to load
 * a requested file from jffs2/cramfs filesystem on a current partition.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
/* FIXME here is not clean handling with load_addr */
int do_romfs_fsload(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *fsname = NULL;
	char *filename = NULL;
	int size = 0;

	ulong offset = 0;

	switch (argc) {
	case 4:
		address = simple_strtoul(argv[3], NULL, 16);
	case 3:
		filename = argv[2];
		offset = simple_strtoul(argv[1], NULL, 16);
		break;
	default:
		cmd_usage(cmdtp);
		return 1;
	}

	/* check partition type for romfs */
	if (romfs_check(address))
		fsname = "ROMFS";
	else
		puts ("error\n");

	printf("### %s loading '%s' to 0x%lx\n", fsname, filename, offset);

	if (romfs_check(address))
		size = romfs_load ((int *) offset, address, filename);

	if (size > 0) {
		char buf[10];
		printf("### %s load complete: %d bytes loaded to 0x%lx\n",
			fsname, size, offset);
		sprintf(buf, "%x", size);
		setenv("filesize", buf);
	} else
		printf("### %s LOAD ERROR<%x> for %s!\n",
					fsname, size, filename);

	return !(size > 0);
}

/**
 * Routine implementing u-boot ls command which lists content of a given
 * directory on a current partition.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
int do_romfs_ls (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename = "/";
	int loc_addr = 0;

	if (argc >= 2)
		filename = argv[1];

	if (argc > 2)
		loc_addr = simple_strtoul (argv[2], NULL, 16);
	else
		loc_addr = address;

	/* check partition type for cramfs */
	if (romfs_check (loc_addr)) {
		address = loc_addr;
		return (romfs_ls (loc_addr, filename) ? 0 : 1);
	}
	return 1;
}



/**
 * Routine implementing u-boot cat command which lists content of a given
 * directory on a current partition.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
int do_romfs_cat (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename = "/";
	int loc_addr = 0;

	if (argc >= 2)
		filename = argv[1];

	if (argc > 2)
		loc_addr = simple_strtoul (argv[2], NULL, 16);
	else
		loc_addr = address;

	/* check partition type for cramfs */
	if (romfs_check (loc_addr)) {
		address = loc_addr;
		return (romfs_cat (loc_addr, filename) ? 0 : 1);
	}
	return 1;
}


/**
 * Routine implementing u-boot fsinfo command. This routine prints out
 * miscellaneous filesystem informations/statistics.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
int do_romfs_fsinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int loc_addr;

	if (argc == 2)
		loc_addr = simple_strtoul(argv[1], NULL, 16);
	else
		loc_addr = address;

	/* check partition type for romfs */
	if (romfs_check(loc_addr))
		if (romfs_info (loc_addr)) {
			address = loc_addr;
			return 1;
		}
	return 0;
}

/*
U_BOOT_CMD(
	ext2ls,	4,	1,	do_ext2ls,
	"list files in a directory (default /)",
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'"
);
*/

/***************************************************/
U_BOOT_CMD(
	rload,	4,	0,	do_romfs_fsload,
	"ROMFS: load binary file from a filesystem image",
	"[ off filename [fs_addr]]\n"
	"    - ROMFS: load binary file from flash bank\n"
	"      with offset 'off'"
);
U_BOOT_CMD(
	rls,	3,	1,	do_romfs_ls,
	"ROMFS: list files in a directory (default /)",
	"[directory [fs_addr]]\n"
	"    - ROMFS: list files in a directory"
);

U_BOOT_CMD(
	rcat,	3,	1,	do_romfs_cat,
	"ROMFS: cat text file (default /)",
	"[directory [fs_addr]]\n"
	"    - ROMFS: list files in a directory"
);

U_BOOT_CMD(
	rinfo,	2,	1,	do_romfs_fsinfo,
	"ROMFS: print information about filesystems",
	"[fs_addr]\n"
	"    - ROMFS: print information about filesystems"
);
/***************************************************/
