/*
 * Unsorted Block Image commands
 *
 *  Copyright (C) 2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Copyright 2008-2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <exports.h>
#include <memalign.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/err.h>
#include <ubi_uboot.h>
#include <asm/errno.h>
#include <jffs2/load_kernel.h>

#undef ubi_msg
#define ubi_msg(fmt, ...) printf("UBI: " fmt "\n", ##__VA_ARGS__)

#define DEV_TYPE_NONE		0
#define DEV_TYPE_NAND		1
#define DEV_TYPE_ONENAND	2
#define DEV_TYPE_NOR		3

/* Private own data */
static struct ubi_device *ubi;
static char buffer[80];
static int ubi_initialized;

struct selected_dev {
	char part_name[80];
	int selected;
	int nr;
	struct mtd_info *mtd_info;
};

static struct selected_dev ubi_dev;

#ifdef CONFIG_CMD_UBIFS
int ubifs_is_mounted(void);
void cmd_ubifs_umount(void);
#endif

static void display_volume_info(struct ubi_device *ubi)
{
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		if (!ubi->volumes[i])
			continue;	/* Empty record */
		ubi_dump_vol_info(ubi->volumes[i]);
	}
}

static void display_ubi_info(struct ubi_device *ubi)
{
	ubi_msg("MTD device name:            \"%s\"", ubi->mtd->name);
	ubi_msg("MTD device size:            %llu MiB", ubi->flash_size >> 20);
	ubi_msg("physical eraseblock size:   %d bytes (%d KiB)",
			ubi->peb_size, ubi->peb_size >> 10);
	ubi_msg("logical eraseblock size:    %d bytes", ubi->leb_size);
	ubi_msg("number of good PEBs:        %d", ubi->good_peb_count);
	ubi_msg("number of bad PEBs:         %d", ubi->bad_peb_count);
	ubi_msg("smallest flash I/O unit:    %d", ubi->min_io_size);
	ubi_msg("VID header offset:          %d (aligned %d)",
			ubi->vid_hdr_offset, ubi->vid_hdr_aloffset);
	ubi_msg("data offset:                %d", ubi->leb_start);
	ubi_msg("max. allowed volumes:       %d", ubi->vtbl_slots);
	ubi_msg("wear-leveling threshold:    %d", CONFIG_MTD_UBI_WL_THRESHOLD);
	ubi_msg("number of internal volumes: %d", UBI_INT_VOL_COUNT);
	ubi_msg("number of user volumes:     %d",
			ubi->vol_count - UBI_INT_VOL_COUNT);
	ubi_msg("available PEBs:             %d", ubi->avail_pebs);
	ubi_msg("total number of reserved PEBs: %d", ubi->rsvd_pebs);
	ubi_msg("number of PEBs reserved for bad PEB handling: %d",
			ubi->beb_rsvd_pebs);
	ubi_msg("max/mean erase counter: %d/%d", ubi->max_ec, ubi->mean_ec);
}

static int ubi_info(int layout)
{
	if (layout)
		display_volume_info(ubi);
	else
		display_ubi_info(ubi);

	return 0;
}

static int ubi_check_volumename(const struct ubi_volume *vol, char *name)
{
	return strcmp(vol->name, name);
}

static int ubi_check(char *name)
{
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		if (!ubi->volumes[i])
			continue;	/* Empty record */

		if (!ubi_check_volumename(ubi->volumes[i], name))
			return 0;
	}

	return 1;
}


static int verify_mkvol_req(const struct ubi_device *ubi,
			    const struct ubi_mkvol_req *req)
{
	int n, err = EINVAL;

	if (req->bytes < 0 || req->alignment < 0 || req->vol_type < 0 ||
	    req->name_len < 0)
		goto bad;

	if ((req->vol_id < 0 || req->vol_id >= ubi->vtbl_slots) &&
	    req->vol_id != UBI_VOL_NUM_AUTO)
		goto bad;

	if (req->alignment == 0)
		goto bad;

	if (req->bytes == 0) {
		printf("No space left in UBI device!\n");
		err = ENOMEM;
		goto bad;
	}

	if (req->vol_type != UBI_DYNAMIC_VOLUME &&
	    req->vol_type != UBI_STATIC_VOLUME)
		goto bad;

	if (req->alignment > ubi->leb_size)
		goto bad;

	n = req->alignment % ubi->min_io_size;
	if (req->alignment != 1 && n)
		goto bad;

	if (req->name_len > UBI_VOL_NAME_MAX) {
		printf("Name too long!\n");
		err = ENAMETOOLONG;
		goto bad;
	}

	return 0;
bad:
	return err;
}

static int ubi_create_vol(char *volume, int64_t size, int dynamic)
{
	struct ubi_mkvol_req req;
	int err;

	if (dynamic)
		req.vol_type = UBI_DYNAMIC_VOLUME;
	else
		req.vol_type = UBI_STATIC_VOLUME;

	req.vol_id = UBI_VOL_NUM_AUTO;
	req.alignment = 1;
	req.bytes = size;

	strcpy(req.name, volume);
	req.name_len = strlen(volume);
	req.name[req.name_len] = '\0';
	req.padding1 = 0;
	/* It's duplicated at drivers/mtd/ubi/cdev.c */
	err = verify_mkvol_req(ubi, &req);
	if (err) {
		printf("verify_mkvol_req failed %d\n", err);
		return err;
	}
	printf("Creating %s volume %s of size %lld\n",
		dynamic ? "dynamic" : "static", volume, size);
	/* Call real ubi create volume */
	return ubi_create_volume(ubi, &req);
}

static struct ubi_volume *ubi_find_volume(char *volume)
{
	struct ubi_volume *vol = NULL;
	int i;

	for (i = 0; i < ubi->vtbl_slots; i++) {
		vol = ubi->volumes[i];
		if (vol && !strcmp(vol->name, volume))
			return vol;
	}

	printf("Volume %s not found!\n", volume);
	return NULL;
}

static int ubi_remove_vol(char *volume)
{
	int err, reserved_pebs, i;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	printf("Remove UBI volume %s (id %d)\n", vol->name, vol->vol_id);

	if (ubi->ro_mode) {
		printf("It's read-only mode\n");
		err = EROFS;
		goto out_err;
	}

	err = ubi_change_vtbl_record(ubi, vol->vol_id, NULL);
	if (err) {
		printf("Error changing Vol tabel record err=%x\n", err);
		goto out_err;
	}
	reserved_pebs = vol->reserved_pebs;
	for (i = 0; i < vol->reserved_pebs; i++) {
		err = ubi_eba_unmap_leb(ubi, vol, i);
		if (err)
			goto out_err;
	}

	kfree(vol->eba_tbl);
	ubi->volumes[vol->vol_id]->eba_tbl = NULL;
	ubi->volumes[vol->vol_id] = NULL;

	ubi->rsvd_pebs -= reserved_pebs;
	ubi->avail_pebs += reserved_pebs;
	i = ubi->beb_rsvd_level - ubi->beb_rsvd_pebs;
	if (i > 0) {
		i = ubi->avail_pebs >= i ? i : ubi->avail_pebs;
		ubi->avail_pebs -= i;
		ubi->rsvd_pebs += i;
		ubi->beb_rsvd_pebs += i;
		if (i > 0)
			ubi_msg("reserve more %d PEBs", i);
	}
	ubi->vol_count -= 1;

	return 0;
out_err:
	ubi_err(ubi, "cannot remove volume %s, error %d", volume, err);
	if (err < 0)
		err = -err;
	return err;
}

static int ubi_volume_continue_write(char *volume, void *buf, size_t size)
{
	int err = 1;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	err = ubi_more_update_data(ubi, vol, buf, size);
	if (err < 0) {
		printf("Couldnt or partially wrote data\n");
		return -err;
	}

	if (err) {
		size = err;

		err = ubi_check_volume(ubi, vol->vol_id);
		if (err < 0)
			return -err;

		if (err) {
			ubi_warn(ubi, "volume %d on UBI device %d is corrupt",
				 vol->vol_id, ubi->ubi_num);
			vol->corrupted = 1;
		}

		vol->checked = 1;
		ubi_gluebi_updated(vol);
	}

	return 0;
}

int ubi_volume_begin_write(char *volume, void *buf, size_t size,
	size_t full_size)
{
	int err = 1;
	int rsvd_bytes = 0;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	rsvd_bytes = vol->reserved_pebs * (ubi->leb_size - vol->data_pad);
	if (size < 0 || size > rsvd_bytes) {
		printf("size > volume size! Aborting!\n");
		return EINVAL;
	}

	err = ubi_start_update(ubi, vol, full_size);
	if (err < 0) {
		printf("Cannot start volume update\n");
		return -err;
	}

	return ubi_volume_continue_write(volume, buf, size);
}

int ubi_volume_write(char *volume, void *buf, size_t size)
{
	return ubi_volume_begin_write(volume, buf, size, size);
}

int ubi_volume_read(char *volume, char *buf, size_t size)
{
	int err, lnum, off, len, tbuf_size;
	void *tbuf;
	unsigned long long tmp;
	struct ubi_volume *vol;
	loff_t offp = 0;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	if (vol->updating) {
		printf("updating");
		return EBUSY;
	}
	if (vol->upd_marker) {
		printf("damaged volume, update marker is set");
		return EBADF;
	}
	if (offp == vol->used_bytes)
		return 0;

	if (size == 0) {
		printf("No size specified -> Using max size (%lld)\n", vol->used_bytes);
		size = vol->used_bytes;
	}

	if (vol->corrupted)
		printf("read from corrupted volume %d", vol->vol_id);
	if (offp + size > vol->used_bytes)
		size = vol->used_bytes - offp;

	tbuf_size = vol->usable_leb_size;
	if (size < tbuf_size)
		tbuf_size = ALIGN(size, ubi->min_io_size);
	tbuf = malloc_cache_aligned(tbuf_size);
	if (!tbuf) {
		printf("NO MEM\n");
		return ENOMEM;
	}
	len = size > tbuf_size ? tbuf_size : size;

	tmp = offp;
	off = do_div(tmp, vol->usable_leb_size);
	lnum = tmp;
	do {
		if (off + len >= vol->usable_leb_size)
			len = vol->usable_leb_size - off;

		err = ubi_eba_read_leb(ubi, vol, lnum, tbuf, off, len, 0);
		if (err) {
			printf("read err %x\n", err);
			err = -err;
			break;
		}
		off += len;
		if (off == vol->usable_leb_size) {
			lnum += 1;
			off -= vol->usable_leb_size;
		}

		size -= len;
		offp += len;

		memcpy(buf, tbuf, len);

		buf += len;
		len = size > tbuf_size ? tbuf_size : size;
	} while (size);

	free(tbuf);
	return err;
}

static int ubi_dev_scan(struct mtd_info *info, char *ubidev,
		const char *vid_header_offset)
{
	struct mtd_device *dev;
	struct part_info *part;
	struct mtd_partition mtd_part;
	char ubi_mtd_param_buffer[80];
	u8 pnum;
	int err;

	if (find_dev_and_part(ubidev, &dev, &pnum, &part) != 0)
		return 1;

	sprintf(buffer, "mtd=%d", pnum);
	memset(&mtd_part, 0, sizeof(mtd_part));
	mtd_part.name = buffer;
	mtd_part.size = part->size;
	mtd_part.offset = part->offset;
	add_mtd_partitions(info, &mtd_part, 1);

	strcpy(ubi_mtd_param_buffer, buffer);
	if (vid_header_offset)
		sprintf(ubi_mtd_param_buffer, "mtd=%d,%s", pnum,
				vid_header_offset);
	err = ubi_mtd_param_parse(ubi_mtd_param_buffer, NULL);
	if (err) {
		del_mtd_partitions(info);
		return -err;
	}

	err = ubi_init();
	if (err) {
		del_mtd_partitions(info);
		return -err;
	}

	ubi_initialized = 1;

	return 0;
}

int ubi_detach(void)
{
	if (mtdparts_init() != 0) {
		printf("Error initializing mtdparts!\n");
		return 1;
	}

#ifdef CONFIG_CMD_UBIFS
	/*
	 * Automatically unmount UBIFS partition when user
	 * changes the UBI device. Otherwise the following
	 * UBIFS commands will crash.
	 */
	if (ubifs_is_mounted())
		cmd_ubifs_umount();
#endif

	/*
	 * Call ubi_exit() before re-initializing the UBI subsystem
	 */
	if (ubi_initialized) {
		ubi_exit();
		del_mtd_partitions(ubi_dev.mtd_info);
		ubi_initialized = 0;
	}

	ubi_dev.selected = 0;
	return 0;
}

int ubi_part(char *part_name, const char *vid_header_offset)
{
	int err = 0;
	char mtd_dev[16];
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	ubi_detach();
	/*
	 * Search the mtd device number where this partition
	 * is located
	 */
	if (find_dev_and_part(part_name, &dev, &pnum, &part)) {
		printf("Partition %s not found!\n", part_name);
		return 1;
	}
	sprintf(mtd_dev, "%s%d", MTD_DEV_TYPE(dev->id->type), dev->id->num);
	ubi_dev.mtd_info = get_mtd_device_nm(mtd_dev);
	if (IS_ERR(ubi_dev.mtd_info)) {
		printf("Partition %s not found on device %s!\n", part_name,
		       mtd_dev);
		return 1;
	}

	ubi_dev.selected = 1;

	strcpy(ubi_dev.part_name, part_name);
	err = ubi_dev_scan(ubi_dev.mtd_info, ubi_dev.part_name,
			vid_header_offset);
	if (err) {
		printf("UBI init error %d\n", err);
		ubi_dev.selected = 0;
		return err;
	}

	ubi = ubi_devices[0];

	return 0;
}

static int do_ubi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int64_t size = 0;
	ulong addr = 0;

	if (argc < 2)
		return CMD_RET_USAGE;


	if (strcmp(argv[1], "detach") == 0) {
		if (argc < 2)
			return CMD_RET_USAGE;

		return ubi_detach();
	}


	if (strcmp(argv[1], "part") == 0) {
		const char *vid_header_offset = NULL;

		/* Print current partition */
		if (argc == 2) {
			if (!ubi_dev.selected) {
				printf("Error, no UBI device/partition selected!\n");
				return 1;
			}

			printf("Device %d: %s, partition %s\n",
			       ubi_dev.nr, ubi_dev.mtd_info->name, ubi_dev.part_name);
			return 0;
		}

		if (argc < 3)
			return CMD_RET_USAGE;

		if (argc > 3)
			vid_header_offset = argv[3];

		return ubi_part(argv[2], vid_header_offset);
	}

	if ((strcmp(argv[1], "part") != 0) && (!ubi_dev.selected)) {
		printf("Error, no UBI device/partition selected!\n");
		return 1;
	}

	if (strcmp(argv[1], "info") == 0) {
		int layout = 0;
		if (argc > 2 && !strncmp(argv[2], "l", 1))
			layout = 1;
		return ubi_info(layout);
	}

	if (strcmp(argv[1], "check") == 0) {
		if (argc > 2)
			return ubi_check(argv[2]);

		printf("Error, no volume name passed\n");
		return 1;
	}

	if (strncmp(argv[1], "create", 6) == 0) {
		int dynamic = 1;	/* default: dynamic volume */

		/* Use maximum available size */
		size = 0;

		/* E.g., create volume size type */
		if (argc == 5) {
			if (strncmp(argv[4], "s", 1) == 0)
				dynamic = 0;
			else if (strncmp(argv[4], "d", 1) != 0) {
				printf("Incorrect type\n");
				return 1;
			}
			argc--;
		}
		/* E.g., create volume size */
		if (argc == 4) {
			size = simple_strtoull(argv[3], NULL, 16);
			argc--;
		}
		/* Use maximum available size */
		if (!size) {
			size = (int64_t)ubi->avail_pebs * ubi->leb_size;
			printf("No size specified -> Using max size (%lld)\n", size);
		}
		/* E.g., create volume */
		if (argc == 3)
			return ubi_create_vol(argv[2], size, dynamic);
	}

	if (strncmp(argv[1], "remove", 6) == 0) {
		/* E.g., remove volume */
		if (argc == 3)
			return ubi_remove_vol(argv[2]);
	}

	if (strncmp(argv[1], "write", 5) == 0) {
		int ret;

		if (argc < 5) {
			printf("Please see usage\n");
			return 1;
		}

		addr = simple_strtoul(argv[2], NULL, 16);
		size = simple_strtoul(argv[4], NULL, 16);

		if (strlen(argv[1]) == 10 &&
		    strncmp(argv[1] + 5, ".part", 5) == 0) {
			if (argc < 6) {
				ret = ubi_volume_continue_write(argv[3],
						(void *)addr, size);
			} else {
				size_t full_size;
				full_size = simple_strtoul(argv[5], NULL, 16);
				ret = ubi_volume_begin_write(argv[3],
						(void *)addr, size, full_size);
			}
		} else {
			ret = ubi_volume_write(argv[3], (void *)addr, size);
		}
		if (!ret) {
			printf("%lld bytes written to volume %s\n", size,
			       argv[3]);
		}

		return ret;
	}

	if (strncmp(argv[1], "read", 4) == 0) {
		size = 0;

		/* E.g., read volume size */
		if (argc == 5) {
			size = simple_strtoul(argv[4], NULL, 16);
			argc--;
		}

		/* E.g., read volume */
		if (argc == 4) {
			addr = simple_strtoul(argv[2], NULL, 16);
			argc--;
		}

		if (argc == 3) {
			printf("Read %lld bytes from volume %s to %lx\n", size,
			       argv[3], addr);

			return ubi_volume_read(argv[3], (char *)addr, size);
		}
	}

	printf("Please see usage\n");
	return 1;
}

U_BOOT_CMD(
	ubi, 6, 1, do_ubi,
	"ubi commands",
	"detach"
		" - detach ubi from a mtd partition\n"
	"ubi part [part] [offset]\n"
		" - Show or set current partition (with optional VID"
		" header offset)\n"
	"ubi info [l[ayout]]"
		" - Display volume and ubi layout information\n"
	"ubi check volumename"
		" - check if volumename exists\n"
	"ubi create[vol] volume [size] [type]"
		" - create volume name with size\n"
	"ubi write[vol] address volume size"
		" - Write volume from address with size\n"
	"ubi write.part address volume size [fullsize]\n"
		" - Write part of a volume from address\n"
	"ubi read[vol] address volume [size]"
		" - Read volume to address with size\n"
	"ubi remove[vol] volume"
		" - Remove volume\n"
	"[Legends]\n"
	" volume: character name\n"
	" size: specified in bytes\n"
	" type: s[tatic] or d[ynamic] (default=dynamic)"
);
