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


int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
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
		printf ("usage: fatload <interface> <dev[:part]> <addr> <filename> [bytes]\n");
		return 1;
	}
	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc=get_dev(argv[1],dev);
	if (dev_desc==NULL) {
		puts ("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf ("\n** Unable to use %s %d:%d for fatload **\n",argv[1],dev,part);
		return 1;
	}
	offset = simple_strtoul (argv[3], NULL, 16);
	if (argc == 6)
		count = simple_strtoul (argv[5], NULL, 16);
	else
		count = 0;
	size = file_fat_read (argv[4], (unsigned char *) offset, count);

	if(size==-1) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n",argv[4],argv[1],dev,part);
		return 1;
	}

	printf ("\n%ld bytes read\n", size);

	sprintf(buf, "%lX", size);
	setenv("filesize", buf);

	return 0;
}


U_BOOT_CMD(
	fatload,	6,	0,	do_fat_fsload,
	"fatload - load binary file from a dos filesystem\n",
	"<interface> <dev[:part]>  <addr> <filename> [bytes]\n"
	"    - load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from dos filesystem\n"
);

int do_fat_ls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *filename = "/";
	int ret;
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;

	if (argc < 3) {
		printf ("usage: fatls <interface> <dev[:part]> [directory]\n");
		return (0);
	}
	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc=get_dev(argv[1],dev);
	if (dev_desc==NULL) {
		puts ("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf ("\n** Unable to use %s %d:%d for fatls **\n",argv[1],dev,part);
		return 1;
	}
	if (argc == 4)
		ret = file_fat_ls (argv[3]);
	else
		ret = file_fat_ls (filename);

	if(ret!=0)
		printf("No Fat FS detected\n");
	return (ret);
}

U_BOOT_CMD(
	fatls,	4,	1,	do_fat_ls,
	"fatls   - list files in a directory (default /)\n",
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'\n"
);

int do_fat_fsinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;

	if (argc < 2) {
		printf ("usage: fatinfo <interface> <dev[:part]>\n");
		return (0);
	}
	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc=get_dev(argv[1],dev);
	if (dev_desc==NULL) {
		puts ("\n** Invalid boot device **\n");
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}
	if (fat_register_device(dev_desc,part)!=0) {
		printf ("\n** Unable to use %s %d:%d for fatinfo **\n",argv[1],dev,part);
		return 1;
	}
	return (file_fat_detectfs ());
}

U_BOOT_CMD(
	fatinfo,	3,	1,	do_fat_fsinfo,
	"fatinfo - print information about filesystem\n",
	"<interface> <dev[:part]>\n"
	"    - print information about filesystem from 'dev' on 'interface'\n"
);

#ifdef NOT_IMPLEMENTED_YET
/* find first device whose first partition is a DOS filesystem */
int find_fat_partition (void)
{
	int i, j;
	block_dev_desc_t *dev_desc;
	unsigned char *part_table;
	unsigned char buffer[ATA_BLOCKSIZE];

	for (i = 0; i < CFG_IDE_MAXDEVICE; i++) {
		dev_desc = ide_get_dev (i);
		if (!dev_desc) {
			debug ("couldn't get ide device!\n");
			return (-1);
		}
		if (dev_desc->part_type == PART_TYPE_DOS) {
			if (dev_desc->
				block_read (dev_desc->dev, 0, 1, (ulong *) buffer) != 1) {
				debug ("can't perform block_read!\n");
				return (-1);
			}
			part_table = &buffer[0x1be];	/* start with partition #4 */
			for (j = 0; j < 4; j++) {
				if ((part_table[4] == 1 ||	/* 12-bit FAT */
				     part_table[4] == 4 ||	/* 16-bit FAT */
				     part_table[4] == 6) &&	/* > 32Meg part */
				    part_table[0] == 0x80) {	/* bootable? */
					curr_dev = i;
					part_offset = part_table[11];
					part_offset <<= 8;
					part_offset |= part_table[10];
					part_offset <<= 8;
					part_offset |= part_table[9];
					part_offset <<= 8;
					part_offset |= part_table[8];
					debug ("found partition start at %ld\n", part_offset);
					return (0);
				}
				part_table += 16;
			}
		}
	}

	debug ("no valid devices found!\n");
	return (-1);
}

int
do_fat_dump (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	__u8 block[1024];
	int ret;
	int bknum;

	ret = 0;

	if (argc != 2) {
		printf ("needs an argument!\n");
		return (0);
	}

	bknum = simple_strtoul (argv[1], NULL, 10);

	if (disk_read (0, bknum, block) != 0) {
		printf ("Error: reading block\n");
		return -1;
	}
	printf ("FAT dump: %d\n", bknum);
	hexdump (512, block);

	return (ret);
}

int disk_read (__u32 startblock, __u32 getsize, __u8 *bufptr)
{
	ulong tot;
	block_dev_desc_t *dev_desc;

	if (curr_dev < 0) {
		if (find_fat_partition () != 0)
			return (-1);
	}

	dev_desc = ide_get_dev (curr_dev);
	if (!dev_desc) {
		debug ("couldn't get ide device\n");
		return (-1);
	}

	tot = dev_desc->block_read (0, startblock + part_offset,
				    getsize, (ulong *) bufptr);

	/* should we do this here?
	   flush_cache ((ulong)buf, cnt*ide_dev_desc[device].blksz);
	 */

	if (tot == getsize)
		return (0);

	debug ("unable to read from device!\n");

	return (-1);
}


static int isprint (unsigned char ch)
{
	if (ch >= 32 && ch < 127)
		return (1);

	return (0);
}


void hexdump (int cnt, unsigned char *data)
{
	int i;
	int run;
	int offset;

	offset = 0;
	while (cnt) {
		printf ("%04X : ", offset);
		if (cnt >= 16)
			run = 16;
		else
			run = cnt;
		cnt -= run;
		for (i = 0; i < run; i++)
			printf ("%02X ", (unsigned int) data[i]);
		printf (": ");
		for (i = 0; i < run; i++)
			printf ("%c", isprint (data[i]) ? data[i] : '.');
		printf ("\n");
		data = &data[16];
		offset += run;
	}
}
#endif	/* NOT_IMPLEMENTED_YET */
