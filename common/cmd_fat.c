/*
 * (C) Copyright 2002
 * Richard Jones, rjones@nexus-tech.net
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
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <s_record.h>
#include <net.h>
#include <ata.h>
#include <part.h>
#include <fat.h>


int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	long size;
	unsigned long offset;
	unsigned long count = 0;
	unsigned long pos = 0;
	char buf [12];
	block_dev_desc_t *dev_desc=NULL;
	disk_partition_t info;
	int part, dev;

	if (argc < 5) {
		printf("usage: fatload <interface> [<dev[:part]>] "
			"<addr> <filename> [bytes [pos]]\n");
		return 1;
	}

	part = get_device_and_partition(argv[1], argv[2], &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->dev;
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatload **\n",
			argv[1], dev, part);
		return 1;
	}
	offset = simple_strtoul(argv[3], NULL, 16);
	if (argc >= 6)
		count = simple_strtoul(argv[5], NULL, 16);
	if (argc >= 7)
		pos = simple_strtoul(argv[6], NULL, 16);
	size = file_fat_read_at(argv[4], pos, (unsigned char *)offset, count);

	if(size==-1) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n",
			argv[4], argv[1], dev, part);
		return 1;
	}

	printf("\n%ld bytes read\n", size);

	sprintf(buf, "%lX", size);
	setenv("filesize", buf);

	return 0;
}


U_BOOT_CMD(
	fatload,	7,	0,	do_fat_fsload,
	"load binary file from a dos filesystem",
	"<interface> [<dev[:part]>]  <addr> <filename> [bytes [pos]]\n"
	"    - Load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from dos filesystem.\n"
	"      'pos' gives the file position to start loading from.\n"
	"      If 'pos' is omitted, 0 is used. 'pos' requires 'bytes'.\n"
	"      'bytes' gives the size to load. If 'bytes' is 0 or omitted,\n"
	"      the load stops on end of file."
);

int do_fat_ls (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename = "/";
	int ret, dev, part;
	block_dev_desc_t *dev_desc=NULL;
	disk_partition_t info;

	if (argc < 2) {
		printf("usage: fatls <interface> [<dev[:part]>] [directory]\n");
		return 0;
	}

	part = get_device_and_partition(argv[1], argv[2], &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->dev;
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatls **\n",
			argv[1], dev, part);
		return 1;
	}
	if (argc == 4)
		ret = file_fat_ls(argv[3]);
	else
		ret = file_fat_ls(filename);

	if(ret!=0)
		printf("No Fat FS detected\n");
	return ret;
}

U_BOOT_CMD(
	fatls,	4,	1,	do_fat_ls,
	"list files in a directory (default /)",
	"<interface> [<dev[:part]>] [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'"
);

int do_fat_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev, part;
	block_dev_desc_t *dev_desc;
	disk_partition_t info;

	if (argc < 2) {
		printf("usage: fatinfo <interface> [<dev[:part]>]\n");
		return 0;
	}

	part = get_device_and_partition(argv[1], argv[2], &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->dev;
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatinfo **\n",
			argv[1], dev, part);
		return 1;
	}
	return file_fat_detectfs();
}

U_BOOT_CMD(
	fatinfo,	3,	1,	do_fat_fsinfo,
	"print information about filesystem",
	"<interface> [<dev[:part]>]\n"
	"    - print information about filesystem from 'dev' on 'interface'"
);

#ifdef CONFIG_FAT_WRITE
static int do_fat_fswrite(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	long size;
	unsigned long addr;
	unsigned long count;
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t info;
	int dev = 0;
	int part = 1;

	if (argc < 5)
		return cmd_usage(cmdtp);

	part = get_device_and_partition(argv[1], argv[2], &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->dev;

	if (fat_register_device(dev_desc, part) != 0) {
		printf("\n** Unable to use %s %d:%d for fatwrite **\n",
			argv[1], dev, part);
		return 1;
	}
	addr = simple_strtoul(argv[3], NULL, 16);
	count = simple_strtoul(argv[5], NULL, 16);

	size = file_fat_write(argv[4], (void *)addr, count);
	if (size == -1) {
		printf("\n** Unable to write \"%s\" from %s %d:%d **\n",
			argv[4], argv[1], dev, part);
		return 1;
	}

	printf("%ld bytes written\n", size);

	return 0;
}

U_BOOT_CMD(
	fatwrite,	6,	0,	do_fat_fswrite,
	"write file into a dos filesystem",
	"<interface> <dev[:part]> <addr> <filename> <bytes>\n"
	"    - write file 'filename' from the address 'addr' in RAM\n"
	"      to 'dev' on 'interface'"
);
#endif
