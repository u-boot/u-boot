/*
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * based on: cmd_jffs2.c
 *
 * 	Add support for a CRAMFS located in RAM
 */


/*
 * CRAMFS support
 */
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <jffs2/jffs2.h>
#include <jffs2/load_kernel.h>
#include <cramfs/cramfs_fs.h>

/* enable/disable debugging messages */
#define	DEBUG_CRAMFS
#undef	DEBUG_CRAMFS

#ifdef  DEBUG_CRAMFS
# define DEBUGF(fmt, args...)	printf(fmt ,##args)
#else
# define DEBUGF(fmt, args...)
#endif

#include <flash.h>

#ifdef CONFIG_SYS_NO_FLASH
# define OFFSET_ADJUSTMENT	0
#else
# define OFFSET_ADJUSTMENT	(flash_info[id.num].start[0])
#endif

#ifndef CONFIG_CMD_JFFS2
#include <linux/stat.h>
char *mkmodestr(unsigned long mode, char *str)
{
	static const char *l = "xwr";
	int mask = 1, i;
	char c;

	switch (mode & S_IFMT) {
		case S_IFDIR:    str[0] = 'd'; break;
		case S_IFBLK:    str[0] = 'b'; break;
		case S_IFCHR:    str[0] = 'c'; break;
		case S_IFIFO:    str[0] = 'f'; break;
		case S_IFLNK:    str[0] = 'l'; break;
		case S_IFSOCK:   str[0] = 's'; break;
		case S_IFREG:    str[0] = '-'; break;
		default:         str[0] = '?';
	}

	for(i = 0; i < 9; i++) {
		c = l[i%3];
		str[9-i] = (mode & mask)?c:'-';
		mask = mask<<1;
	}

	if(mode & S_ISUID) str[3] = (mode & S_IXUSR)?'s':'S';
	if(mode & S_ISGID) str[6] = (mode & S_IXGRP)?'s':'S';
	if(mode & S_ISVTX) str[9] = (mode & S_IXOTH)?'t':'T';
	str[10] = '\0';
	return str;
}
#endif /* CONFIG_CMD_JFFS2 */

extern int cramfs_check (struct part_info *info);
extern int cramfs_load (char *loadoffset, struct part_info *info, char *filename);
extern int cramfs_ls (struct part_info *info, char *filename);
extern int cramfs_info (struct part_info *info);

/***************************************************/
/* U-Boot commands				   */
/***************************************************/

/**
 * Routine implementing fsload u-boot command. This routine tries to load
 * a requested file from cramfs filesystem at location 'cramfsaddr'.
 * cramfsaddr is an evironment variable.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
int do_cramfs_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename;
	int size;
	ulong offset = load_addr;

	struct part_info part;
	struct mtd_device dev;
	struct mtdids id;

	ulong addr;
	addr = simple_strtoul(getenv("cramfsaddr"), NULL, 16);

	/* hack! */
	/* cramfs_* only supports NOR flash chips */
	/* fake the device type */
	id.type = MTD_DEV_TYPE_NOR;
	id.num = 0;
	dev.id = &id;
	part.dev = &dev;
	/* fake the address offset */
	part.offset = addr - OFFSET_ADJUSTMENT;

	/* pre-set Boot file name */
	if ((filename = getenv("bootfile")) == NULL) {
		filename = "uImage";
	}

	if (argc == 2) {
		filename = argv[1];
	}
	if (argc == 3) {
		offset = simple_strtoul(argv[1], NULL, 0);
		load_addr = offset;
		filename = argv[2];
	}

	size = 0;
	if (cramfs_check(&part))
		size = cramfs_load ((char *) offset, &part, filename);

	if (size > 0) {
		printf("### CRAMFS load complete: %d bytes loaded to 0x%lx\n",
			size, offset);
		setenv_hex("filesize", size);
	} else {
		printf("### CRAMFS LOAD ERROR<%x> for %s!\n", size, filename);
	}

	return !(size > 0);
}

/**
 * Routine implementing u-boot ls command which lists content of a given
 * directory at location 'cramfsaddr'.
 * cramfsaddr is an evironment variable.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
int do_cramfs_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename = "/";
	int ret;
	struct part_info part;
	struct mtd_device dev;
	struct mtdids id;

	ulong addr;
	addr = simple_strtoul(getenv("cramfsaddr"), NULL, 16);

	/* hack! */
	/* cramfs_* only supports NOR flash chips */
	/* fake the device type */
	id.type = MTD_DEV_TYPE_NOR;
	id.num = 0;
	dev.id = &id;
	part.dev = &dev;
	/* fake the address offset */
	part.offset = addr - OFFSET_ADJUSTMENT;

	if (argc == 2)
		filename = argv[1];

	ret = 0;
	if (cramfs_check(&part))
		ret = cramfs_ls (&part, filename);

	return ret ? 0 : 1;
}

/* command line only */

/***************************************************/
U_BOOT_CMD(
	cramfsload,	3,	0,	do_cramfs_load,
	"load binary file from a filesystem image",
	"[ off ] [ filename ]\n"
	"    - load binary file from address 'cramfsaddr'\n"
	"      with offset 'off'\n"
);
U_BOOT_CMD(
	cramfsls,	2,	1,	do_cramfs_ls,
	"list files in a directory (default /)",
	"[ directory ]\n"
	"    - list files in a directory.\n"
);
