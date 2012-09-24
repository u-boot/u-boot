/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT2/4 filesystem implementation in Uboot by
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

#define DOS_PART_MAGIC_OFFSET		0x1fe
#define DOS_FS_TYPE_OFFSET		0x36
#define DOS_FS32_TYPE_OFFSET		0x52

int do_ext_load(cmd_tbl_t *cmdtp, int flag, int argc,
						char *const argv[])
{
	char *filename = NULL;
	char *ep;
	int dev;
	unsigned long part = 1;
	ulong addr = 0;
	ulong part_length;
	int filelen;
	disk_partition_t info;
	struct ext_filesystem *fs;
	char buf[12];
	unsigned long count;
	const char *addr_str;

	count = 0;
	addr = simple_strtoul(argv[3], NULL, 16);
	filename = getenv("bootfile");
	switch (argc) {
	case 3:
		addr_str = getenv("loadaddr");
		if (addr_str != NULL)
			addr = simple_strtoul(addr_str, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;

		break;
	case 4:
		break;
	case 5:
		filename = argv[4];
		break;
	case 6:
		filename = argv[4];
		count = simple_strtoul(argv[5], NULL, 16);
		break;

	default:
		return cmd_usage(cmdtp);
	}

	if (!filename) {
		puts("** No boot file defined **\n");
		return 1;
	}

	dev = (int)simple_strtoul(argv[2], &ep, 16);
	ext4_dev_desc = get_dev(argv[1], dev);
	if (ext4_dev_desc == NULL) {
		printf("** Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}
	if (init_fs(ext4_dev_desc))
		return 1;

	fs = get_fs();
	if (*ep) {
		if (*ep != ':') {
			puts("** Invalid boot device, use `dev[:part]' **\n");
			goto fail;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}

	if (part != 0) {
		if (get_partition_info(fs->dev_desc, part, &info)) {
			printf("** Bad partition %lu **\n", part);
			goto fail;
		}

		if (strncmp((char *)info.type, BOOT_PART_TYPE,
			    strlen(BOOT_PART_TYPE)) != 0) {
			printf("** Invalid partition type \"%s\""
			       " (expect \"" BOOT_PART_TYPE "\")\n", info.type);
			goto fail;
		}
		printf("Loading file \"%s\" "
		       "from %s device %d:%lu %s\n",
		       filename, argv[1], dev, part, info.name);
	} else {
		printf("Loading file \"%s\" from %s device %d\n",
		       filename, argv[1], dev);
	}

	part_length = ext4fs_set_blk_dev(fs->dev_desc, part);
	if (part_length == 0) {
		printf("**Bad partition - %s %d:%lu **\n", argv[1], dev, part);
		ext4fs_close();
		goto fail;
	}

	if (!ext4fs_mount(part_length)) {
		printf("** Bad ext2 partition or disk - %s %d:%lu **\n",
		       argv[1], dev, part);
		ext4fs_close();
		goto fail;
	}

	filelen = ext4fs_open(filename);
	if (filelen < 0) {
		printf("** File not found %s\n", filename);
		ext4fs_close();
		goto fail;
	}
	if ((count < filelen) && (count != 0))
		filelen = count;

	if (ext4fs_read((char *)addr, filelen) != filelen) {
		printf("** Unable to read \"%s\" from %s %d:%lu **\n",
		       filename, argv[1], dev, part);
		ext4fs_close();
		goto fail;
	}

	ext4fs_close();
	deinit_fs(fs->dev_desc);
	/* Loading ok, update default load address */
	load_addr = addr;

	printf("%d bytes read\n", filelen);
	sprintf(buf, "%X", filelen);
	setenv("filesize", buf);

	return 0;
fail:
	deinit_fs(fs->dev_desc);
	return 1;
}

int do_ext_ls(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const char *filename = "/";
	int dev;
	unsigned long part = 1;
	char *ep;
	struct ext_filesystem *fs;
	int part_length;
	if (argc < 3)
		return cmd_usage(cmdtp);

	dev = (int)simple_strtoul(argv[2], &ep, 16);

	ext4_dev_desc = get_dev(argv[1], dev);

	if (ext4_dev_desc == NULL) {
		printf("\n** Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}

	if (init_fs(ext4_dev_desc))
		return 1;

	fs = get_fs();
	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			goto fail;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}

	if (argc == 4)
		filename = argv[3];

	part_length = ext4fs_set_blk_dev(fs->dev_desc, part);
	if (part_length == 0) {
		printf("** Bad partition - %s %d:%lu **\n", argv[1], dev, part);
		ext4fs_close();
		goto fail;
	}

	if (!ext4fs_mount(part_length)) {
		printf("** Bad ext2 partition or disk - %s %d:%lu **\n",
		       argv[1], dev, part);
		ext4fs_close();
		goto fail;
	}

	if (ext4fs_ls(filename)) {
		printf("** Error extfs_ls() **\n");
		ext4fs_close();
		goto fail;
	};

	ext4fs_close();
	deinit_fs(fs->dev_desc);
	return 0;

fail:
	deinit_fs(fs->dev_desc);
	return 1;
}
