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
	unsigned long count;
	char buf [12];
	block_dev_desc_t *dev_desc=NULL;
	int dev=0;
	int part=1;
	char *ep;

	if (argc < 5) {
		printf( "usage: fatload <interface> <dev[:part]> "
			"<addr> <filename> [bytes]\n");
		return 1;
	}

	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf("\n** Unable to use %s %d:%d for fatload **\n",
			argv[1], dev, part);
		return 1;
	}
	offset = simple_strtoul(argv[3], NULL, 16);
	if (argc == 6)
		count = simple_strtoul(argv[5], NULL, 16);
	else
		count = 0;
	size = file_fat_read(argv[4], (unsigned char *)offset, count);

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
	fatload,	6,	0,	do_fat_fsload,
	"load binary file from a dos filesystem",
	"<interface> <dev[:part]>  <addr> <filename> [bytes]\n"
	"    - load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from dos filesystem"
);

int do_fat_ls (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *filename = "/";
	int ret;
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;

	if (argc < 3) {
		printf("usage: fatls <interface> <dev[:part]> [directory]\n");
		return 0;
	}
	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
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
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'"
);

int do_fat_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;

	if (argc < 2) {
		printf("usage: fatinfo <interface> <dev[:part]>\n");
		return 0;
	}
	dev = (int)simple_strtoul(argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc == NULL) {
		puts("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
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
	"<interface> <dev[:part]>\n"
	"    - print information about filesystem from 'dev' on 'interface'"
);
