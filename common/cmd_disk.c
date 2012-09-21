/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <common.h>
#include <command.h>

int common_diskboot(cmd_tbl_t *cmdtp, const char *intf, int argc,
		    char *const argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev, part = 0;
	ulong addr, cnt;
	disk_partition_t info;
	image_header_t *hdr;
	block_dev_desc_t *dev_desc;

#if defined(CONFIG_FIT)
	const void *fit_hdr = NULL;
#endif

	bootstage_mark(BOOTSTAGE_ID_IDE_START);
	switch (argc) {
	case 1:
		addr = CONFIG_SYS_LOAD_ADDR;
		boot_device = getenv("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	default:
		bootstage_error(BOOTSTAGE_ID_IDE_ADDR);
		return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_ADDR);

	if (!boot_device) {
		puts("\n** No boot device **\n");
		bootstage_error(BOOTSTAGE_ID_IDE_BOOT_DEVICE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_BOOT_DEVICE);

	dev = simple_strtoul(boot_device, &ep, 16);

	dev_desc = get_dev(intf, dev);
	if (dev_desc->type == DEV_TYPE_UNKNOWN) {
		printf("\n** Device %d not available\n", dev);
		bootstage_error(BOOTSTAGE_ID_IDE_TYPE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_TYPE);

	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			bootstage_error(BOOTSTAGE_ID_IDE_PART);
			return 1;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART);

	if (get_partition_info(dev_desc, part, &info)) {
		bootstage_error(BOOTSTAGE_ID_IDE_PART_INFO);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_INFO);

	if ((strncmp((char *)info.type, BOOT_PART_TYPE, sizeof(info.type)) != 0)
	    &&
	    (strncmp((char *)info.type, BOOT_PART_COMP, sizeof(info.type)) != 0)
	   ) {
		printf("\n** Invalid partition type \"%.32s\"" " (expect \""
			BOOT_PART_TYPE "\")\n",
			info.type);
		bootstage_error(BOOTSTAGE_ID_IDE_PART_TYPE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_TYPE);

	printf("\nLoading from disk device %d, partition %d: "
	       "Name: %.32s  Type: %.32s\n", dev, part, info.name, info.type);

	debug("First Block: %ld,  # of blocks: %ld, Block Size: %ld\n",
	      info.start, info.size, info.blksz);

	if (dev_desc->block_read(dev, info.start, 1, (ulong *) addr) != 1) {
		printf("** Read error on %d:%d\n", dev, part);
		bootstage_error(BOOTSTAGE_ID_IDE_PART_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_READ);

	switch (genimg_get_format((void *) addr)) {
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *) addr;

		bootstage_mark(BOOTSTAGE_ID_IDE_FORMAT);

		if (!image_check_hcrc(hdr)) {
			puts("\n** Bad Header Checksum **\n");
			bootstage_error(BOOTSTAGE_ID_IDE_CHECKSUM);
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_IDE_CHECKSUM);

		image_print_contents(hdr);

		cnt = image_get_image_size(hdr);
		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *) addr;
		puts("Fit image detected...\n");

		cnt = fit_get_size(fit_hdr);
		break;
#endif
	default:
		bootstage_error(BOOTSTAGE_ID_IDE_FORMAT);
		puts("** Unknown image type\n");
		return 1;
	}

	cnt += info.blksz - 1;
	cnt /= info.blksz;
	cnt -= 1;

	if (dev_desc->block_read(dev, info.start + 1, cnt,
					 (ulong *)(addr + info.blksz)) != cnt) {
		printf("** Read error on %d:%d\n", dev, part);
		bootstage_error(BOOTSTAGE_ID_IDE_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_READ);

#if defined(CONFIG_FIT)
	/* This cannot be done earlier,
	 * we need complete FIT image in RAM first */
	if (genimg_get_format((void *) addr) == IMAGE_FORMAT_FIT) {
		if (!fit_check_format(fit_hdr)) {
			bootstage_error(BOOTSTAGE_ID_IDE_FIT_READ);
			puts("** Bad FIT image format\n");
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_IDE_FIT_READ_OK);
		fit_print_contents(fit_hdr);
	}
#endif

	flush_cache(addr, (cnt+1)*info.blksz);

	/* Loading ok, update default load address */
	load_addr = addr;

	return bootm_maybe_autostart(cmdtp, argv[0]);
}
