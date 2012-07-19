/*
 *
 * ZFS filesystem porting to Uboot by
 * Jorgen Lundman <lundman at lundman.net>
 *
 * zfsfs support
 * made from existing GRUB Sources by Sun, GNU and others.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <part.h>
#include <config.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <zfs_common.h>
#include <linux/stat.h>
#include <malloc.h>

#if defined(CONFIG_CMD_USB) && defined(CONFIG_USB_STORAGE)
#include <usb.h>
#endif

#if !defined(CONFIG_DOS_PARTITION) && !defined(CONFIG_EFI_PARTITION)
#error DOS or EFI partition support must be selected
#endif

#define DOS_PART_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET	0x36
#define DOS_FS32_TYPE_OFFSET	0x52

static int do_zfs_load(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *filename = NULL;
	char *ep;
	int dev;
	unsigned long part = 1;
	ulong addr = 0;
	ulong part_length;
	disk_partition_t info;
	char buf[12];
	unsigned long count;
	const char *addr_str;
	struct zfs_file zfile;
	struct device_s vdev;

	if (argc < 3)
		return CMD_RET_USAGE;

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
	zfs_dev_desc = get_dev(argv[1], dev);
	if (zfs_dev_desc == NULL) {
		printf("** Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}

	if (*ep) {
		if (*ep != ':') {
			puts("** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}

	if (part != 0) {
		if (get_partition_info(zfs_dev_desc, part, &info)) {
			printf("** Bad partition %lu **\n", part);
			return 1;
		}

		if (strncmp((char *)info.type, BOOT_PART_TYPE,
					strlen(BOOT_PART_TYPE)) != 0) {
			printf("** Invalid partition type \"%s\" (expect \"" BOOT_PART_TYPE "\")\n",
				   info.type);
			return 1;
		}
		printf("Loading file \"%s\" "
			   "from %s device %d:%lu %s\n",
			   filename, argv[1], dev, part, info.name);
	} else {
		printf("Loading file \"%s\" from %s device %d\n",
			   filename, argv[1], dev);
	}

	part_length = zfs_set_blk_dev(zfs_dev_desc, part);
	if (part_length == 0) {
		printf("**Bad partition - %s %d:%lu **\n", argv[1], dev, part);
		return 1;
	}

	vdev.part_length = part_length;

	memset(&zfile, 0, sizeof(zfile));
	zfile.device = &vdev;
	if (zfs_open(&zfile, filename)) {
		printf("** File not found %s\n", filename);
		return 1;
	}

	if ((count < zfile.size) && (count != 0))
		zfile.size = (uint64_t)count;

	if (zfs_read(&zfile, (char *)addr, zfile.size) != zfile.size) {
		printf("** Unable to read \"%s\" from %s %d:%lu **\n",
			   filename, argv[1], dev, part);
		zfs_close(&zfile);
		return 1;
	}

	zfs_close(&zfile);

	/* Loading ok, update default load address */
	load_addr = addr;

	printf("%llu bytes read\n", zfile.size);
	sprintf(buf, "%llX", zfile.size);
	setenv("filesize", buf);

	return 0;
}


int zfs_print(const char *entry, const struct zfs_dirhook_info *data)
{
	printf("%s %s\n",
		   data->dir ? "<DIR> " : "		 ",
		   entry);
	return 0; /* 0 continue, 1 stop */
}



static int do_zfs_ls(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *filename = "/";
	int dev;
	unsigned long part = 1;
	char *ep;
	int part_length;
	struct device_s vdev;

	if (argc < 3)
		return cmd_usage(cmdtp);

	dev = (int)simple_strtoul(argv[2], &ep, 16);
	zfs_dev_desc = get_dev(argv[1], dev);

	if (zfs_dev_desc == NULL) {
		printf("\n** Block device %s %d not supported\n", argv[1], dev);
		return 1;
	}

	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}

	if (argc == 4)
		filename = argv[3];

	part_length = zfs_set_blk_dev(zfs_dev_desc, part);
	if (part_length == 0) {
		printf("** Bad partition - %s %d:%lu **\n", argv[1], dev, part);
		return 1;
	}

	vdev.part_length = part_length;

	zfs_ls(&vdev, filename,
		   zfs_print);

	return 0;
}


U_BOOT_CMD(zfsls, 4, 1, do_zfs_ls,
		   "list files in a directory (default /)",
		   "<interface> <dev[:part]> [directory]\n"
		   "	  - list files from 'dev' on 'interface' in a '/DATASET/@/$dir/'");

U_BOOT_CMD(zfsload, 6, 0, do_zfs_load,
		   "load binary file from a ZFS filesystem",
		   "<interface> <dev[:part]> [addr] [filename] [bytes]\n"
		   "	  - load binary file '/DATASET/@/$dir/$file' from 'dev' on 'interface'\n"
		   "		 to address 'addr' from ZFS filesystem");
