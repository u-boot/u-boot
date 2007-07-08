/*
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
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
 *
 */

/*
 * Reiserfs support
 */
#include <common.h>

#if defined(CONFIG_CMD_REISER)
#include <config.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <reiserfs.h>
#include <part.h>

#ifndef CONFIG_DOS_PARTITION
#error DOS partition support must be selected
#endif

/* #define	REISER_DEBUG */

#ifdef	REISER_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

int do_reiserls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *filename = "/";
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;
	int part_length;

	if (argc < 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);

	if (dev_desc == NULL) {
		printf ("\n** Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}

	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}

	if (argc == 4) {
	    filename = argv[3];
	}

	PRINTF("Using device %s %d:%d, directory: %s\n", argv[1], dev, part, filename);

	if ((part_length = reiserfs_set_blk_dev(dev_desc, part)) == 0) {
		printf ("** Bad partition - %s %d:%d **\n",  argv[1], dev, part);
		return 1;
	}

	if (!reiserfs_mount(part_length)) {
		printf ("** Bad Reisefs partition or disk - %s %d:%d **\n",  argv[1], dev, part);
		return 1;
	}

	if (reiserfs_ls (filename)) {
		printf ("** Error reiserfs_ls() **\n");
		return 1;
	};

	return 0;
}

U_BOOT_CMD(
	reiserls,	4,	1,	do_reiserls,
	"reiserls- list files in a directory (default /)\n",
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'\n"
);

/******************************************************************************
 * Reiserfs boot command intepreter. Derived from diskboot
 */
int do_reiserload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *filename = NULL;
	char *ep;
	int dev, part = 0;
	ulong addr = 0, part_length, filelen;
	disk_partition_t info;
	block_dev_desc_t *dev_desc = NULL;
	char buf [12];
	unsigned long count;
	char *addr_str;

	switch (argc) {
	case 3:
		addr_str = getenv("loadaddr");
		if (addr_str != NULL) {
			addr = simple_strtoul (addr_str, NULL, 16);
		} else {
			addr = CFG_LOAD_ADDR;
		}
		filename = getenv ("bootfile");
		count = 0;
		break;
	case 4:
		addr = simple_strtoul (argv[3], NULL, 16);
		filename = getenv ("bootfile");
		count = 0;
		break;
	case 5:
		addr = simple_strtoul (argv[3], NULL, 16);
		filename = argv[4];
		count = 0;
		break;
	case 6:
		addr = simple_strtoul (argv[3], NULL, 16);
		filename = argv[4];
		count = simple_strtoul (argv[5], NULL, 16);
		break;

	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (!filename) {
		puts ("\n** No boot file defined **\n");
		return 1;
	}

	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc = get_dev(argv[1],dev);
	if (dev_desc==NULL) {
		printf ("\n** Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}
	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}

	PRINTF("Using device %s%d, partition %d\n", argv[1], dev, part);

	if (part != 0) {
		if (get_partition_info (dev_desc, part, &info)) {
			printf ("** Bad partition %d **\n", part);
			return 1;
		}

		if (strncmp(info.type, BOOT_PART_TYPE, sizeof(info.type)) != 0) {
			printf ("\n** Invalid partition type \"%.32s\""
				" (expect \"" BOOT_PART_TYPE "\")\n",
				info.type);
			return 1;
		}
		PRINTF ("\nLoading from block device %s device %d, partition %d: "
			"Name: %.32s  Type: %.32s  File:%s\n",
			argv[1], dev, part, info.name, info.type, filename);
	} else {
		PRINTF ("\nLoading from block device %s device %d, File:%s\n",
			argv[1], dev, filename);
	}


	if ((part_length = reiserfs_set_blk_dev(dev_desc, part)) == 0) {
		printf ("** Bad partition - %s %d:%d **\n",  argv[1], dev, part);
		return 1;
	}

	if (!reiserfs_mount(part_length)) {
		printf ("** Bad Reisefs partition or disk - %s %d:%d **\n",  argv[1], dev, part);
		return 1;
	}

	filelen = reiserfs_open(filename);
	if (filelen < 0) {
		printf("** File not found %s\n", filename);
		return 1;
	}
	if ((count < filelen) && (count != 0)) {
	    filelen = count;
	}

	if (reiserfs_read((char *)addr, filelen) != filelen) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n", filename, argv[1], dev, part);
		return 1;
	}

	/* Loading ok, update default load address */
	load_addr = addr;

	printf ("\n%ld bytes read\n", filelen);
	sprintf(buf, "%lX", filelen);
	setenv("filesize", buf);

	return filelen;
}

U_BOOT_CMD(
	reiserload,	6,	0,	do_reiserload,
	"reiserload- load binary file from a Reiser filesystem\n",
	"<interface> <dev[:part]> [addr] [filename] [bytes]\n"
	"    - load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from dos filesystem\n"
);

#endif
