/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Ext4fs support
 * made from existing cmd_ext2.c file of Uboot
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * made from cmd_reiserfs by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
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
 * Changelog:
 *	0.1 - Newly created file for ext4fs support. Taken from cmd_ext2.c
 *	        file in uboot. Added ext4fs ls load and write support.
 */

#include <common.h>
#include <part.h>
#include <config.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <ext_common.h>
#include <ext4fs.h>
#include <linux/stat.h>
#include <malloc.h>

#if defined(CONFIG_CMD_USB) && defined(CONFIG_USB_STORAGE)
#include <usb.h>
#endif

#if !defined(CONFIG_DOS_PARTITION) && !defined(CONFIG_EFI_PARTITION)
#error DOS or EFI partition support must be selected
#endif

uint64_t total_sector;
uint64_t part_offset;
#if defined(CONFIG_CMD_EXT4_WRITE)
static uint64_t part_size;
static uint16_t cur_part = 1;
#endif

#define DOS_PART_MAGIC_OFFSET		0x1fe
#define DOS_FS_TYPE_OFFSET		0x36
#define DOS_FS32_TYPE_OFFSET		0x52

int do_ext4_load(cmd_tbl_t *cmdtp, int flag, int argc,
						char *const argv[])
{
	if (do_ext_load(cmdtp, flag, argc, argv))
		return -1;

	return 0;
}

int do_ext4_ls(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (do_ext_ls(cmdtp, flag, argc, argv))
		return -1;

	return 0;
}

#if defined(CONFIG_CMD_EXT4_WRITE)
static int ext4_register_device(block_dev_desc_t *dev_desc, int part_no)
{
	unsigned char buffer[SECTOR_SIZE];
	disk_partition_t info;

	if (!dev_desc->block_read)
		return -1;

	/* check if we have a MBR (on floppies we have only a PBR) */
	if (dev_desc->block_read(dev_desc->dev, 0, 1, (ulong *) buffer) != 1) {
		printf("** Can't read from device %d **\n", dev_desc->dev);
		return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 ||
	    buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) {
		/* no signature found */
		return -1;
	}

	/* First we assume there is a MBR */
	if (!get_partition_info(dev_desc, part_no, &info)) {
		part_offset = info.start;
		cur_part = part_no;
		part_size = info.size;
	} else if ((strncmp((char *)&buffer[DOS_FS_TYPE_OFFSET],
			    "FAT", 3) == 0) || (strncmp((char *)&buffer
							[DOS_FS32_TYPE_OFFSET],
							"FAT32", 5) == 0)) {
		/* ok, we assume we are on a PBR only */
		cur_part = 1;
		part_offset = 0;
	} else {
		printf("** Partition %d not valid on device %d **\n",
		       part_no, dev_desc->dev);
		return -1;
	}

	return 0;
}

int do_ext4_write(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	const char *filename = "/";
	int part_length;
	unsigned long part = 1;
	int dev;
	char *ep;
	unsigned long ram_address;
	unsigned long file_size;
	disk_partition_t info;
	struct ext_filesystem *fs;

	if (argc < 6)
		return cmd_usage(cmdtp);

	dev = (int)simple_strtoul(argv[2], &ep, 16);
	ext4_dev_desc = get_dev(argv[1], dev);
	if (ext4_dev_desc == NULL) {
		printf("Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}

	fs = get_fs();
	if (*ep) {
		if (*ep != ':') {
			puts("Invalid boot device, use `dev[:part]'\n");
			goto fail;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}

	/* get the filename */
	filename = argv[3];

	/* get the address in hexadecimal format (string to int) */
	ram_address = simple_strtoul(argv[4], NULL, 16);

	/* get the filesize in base 10 format */
	file_size = simple_strtoul(argv[5], NULL, 10);

	/* set the device as block device */
	part_length = ext4fs_set_blk_dev(ext4_dev_desc, part);
	if (part_length == 0) {
		printf("Bad partition - %s %d:%lu\n", argv[1], dev, part);
		goto fail;
	}

	/* register the device and partition */
	if (ext4_register_device(ext4_dev_desc, part) != 0) {
		printf("Unable to use %s %d:%lu for fattable\n",
		       argv[1], dev, part);
		goto fail;
	}

	/* get the partition information */
	if (!get_partition_info(ext4_dev_desc, part, &info)) {
		total_sector = (info.size * info.blksz) / SECTOR_SIZE;
		fs->total_sect = total_sector;
	} else {
		printf("error : get partition info\n");
		goto fail;
	}

	/* mount the filesystem */
	if (!ext4fs_mount(part_length)) {
		printf("Bad ext4 partition %s %d:%lu\n", argv[1], dev, part);
		goto fail;
	}

	/* start write */
	if (ext4fs_write(filename, (unsigned char *)ram_address, file_size)) {
		printf("** Error ext4fs_write() **\n");
		goto fail;
	}
	ext4fs_close();

	return 0;

fail:
	ext4fs_close();

	return 1;
}

U_BOOT_CMD(ext4write, 6, 1, do_ext4_write,
	"create a file in the root directory",
	"<interface> <dev[:part]> [Absolute filename path] [Address] [sizebytes]\n"
	"	  - create a file in / directory");

#endif

U_BOOT_CMD(ext4ls, 4, 1, do_ext4_ls,
	   "list files in a directory (default /)",
	   "<interface> <dev[:part]> [directory]\n"
	   "	  - list files from 'dev' on 'interface' in a 'directory'");

U_BOOT_CMD(ext4load, 6, 0, do_ext4_load,
	   "load binary file from a Ext4 filesystem",
	   "<interface> <dev[:part]> [addr] [filename] [bytes]\n"
	   "	  - load binary file 'filename' from 'dev' on 'interface'\n"
	   "		 to address 'addr' from ext4 filesystem");
