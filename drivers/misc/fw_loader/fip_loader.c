// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Christian Marangi <ansuelsmth@gmail.com>
 *
 */

#define LOG_CATEGORY UCLASS_FIP_FIRMWARE_LOADER

#include <dm.h>
#include <div64.h>
#include <env.h>
#include <errno.h>
#include <blk.h>
#include <fs.h>
#include <fs_loader.h>
#include <log.h>
#include <mapmem.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <u-boot/uuid.h>

#include "internal.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TOC_HEADER_NAME	0xaa640001

struct fip_toc_header {
	u32 name;
	u32 serial_number;
	u64 flags;
};

struct fip_toc_entry {
	struct uuid uuid;
	u64 offset_address;
	u64 size;
	u64 flags;
};

enum fip_storage_interface {
	FIP_STORAGE_INTERFACE_BLK,
	FIP_STORAGE_INTERFACE_UBI,
};

struct fip_storage_info {
	enum fip_storage_interface storage_interface;

	/* BLK info */
	struct disk_partition part_info;
	struct blk_desc *desc;
	unsigned int part_offset;

	/* UBI info */
	char *ubi_volume;
};

static bool validate_fip_toc_header(struct fip_toc_header *hdr)
{
	if (hdr->name != TOC_HEADER_NAME) {
		log_err("Invalid FIP header\n");
		return false;
	}

	return true;
}

static int firmware_name_to_uuid(struct firmware *firmwarep,
				 struct uuid *uuid)
{
	const char *uuid_str = firmwarep->name;
	int ret;

	ret = uuid_str_to_bin(uuid_str, (unsigned char *)uuid,
			      UUID_STR_FORMAT_STD);
	if (ret)
		log_err("Invalid UUID str: %s\n", uuid_str);

	return ret;
}

static int check_fip_toc_entry(struct fip_toc_entry *ent,
			       struct uuid *uuid,
			       struct fip_toc_entry *dent)
{
	struct uuid uuid_null = { };

	/* NULL uuid. We parsed every entry */
	if (!memcmp(&ent->uuid, &uuid_null, sizeof(uuid_null)))
		return -ENOENT;

	/* We found the related uuid */
	if (!memcmp(&ent->uuid, uuid, sizeof(*uuid))) {
		log_debug("Found matching FIP entry. offset: 0x%llx size: %lld\n",
			  ent->offset_address, ent->size);
		memcpy(dent, ent, sizeof(*ent));
		return 0;
	}

	return -EAGAIN;
}

static int blk_read_fip_toc_header(struct blk_desc *desc, u32 offset,
				   char *buf, struct fip_toc_header *hdr)
{
	unsigned int blkcnt = BLOCK_CNT(sizeof(*hdr), desc);
	size_t read = 0;
	int i, ret;

	for (i = 0; i < blkcnt && read < sizeof(*hdr); i++) {
		unsigned int to_read = MIN(desc->blksz,
					   sizeof(*hdr) - read);

		ret = blk_dread(desc, offset + i, 1, buf);
		if (ret != 1)
			return -EINVAL;

		memcpy((u8 *)hdr + read, buf, to_read);
		read += to_read;
	}

	return read;
}

static int blk_read_fip_toc_entry(struct blk_desc *desc, u32 offset,
				  int pos, char *buf,
				  struct fip_toc_entry *ent)
{
	unsigned int left, consumed, to_read, read = 0;
	unsigned int blkstart, blkcnt;
	int i, ret;

	consumed = pos % desc->blksz;
	left = desc->blksz - consumed;
	to_read = MIN(left, sizeof(*ent));

	blkstart = BLOCK_CNT(pos, desc);
	blkcnt = BLOCK_CNT(sizeof(*ent) - to_read, desc);

	/* Read data from previous cached block if present */
	if (left) {
		memcpy(ent, buf + consumed, to_read);
		read += to_read;
	}

	for (i = 0; i < blkcnt && read < sizeof(*ent); i++) {
		to_read = MIN(desc->blksz, sizeof(*ent) - read);

		ret = blk_dread(desc, offset + blkstart + i, 1, buf);
		if (ret != 1)
			return -EINVAL;

		memcpy((u8 *)ent + read, buf, to_read);
		read += to_read;
	}

	return read;
}

static int blk_parse_fip_firmware(struct firmware *firmwarep,
				  struct blk_desc *desc,
				  struct disk_partition *part_info,
				  unsigned int part_offset,
				  struct fip_toc_entry *dent)
{
	unsigned int offset = part_info->start + part_offset;
	struct fip_toc_header hdr;
	struct fip_toc_entry ent;
	struct uuid uuid;
	unsigned int pos;
	char *read_buf;
	int ret;

	/* Allocate a Scratch Buffer for FIP parsing */
	read_buf = malloc(desc->blksz);
	if (!read_buf)
		return -ENOMEM;

	pos = blk_read_fip_toc_header(desc, offset, read_buf, &hdr);
	if (pos < 0) {
		ret = -EINVAL;
		goto out;
	}

	if (!validate_fip_toc_header(&hdr)) {
		ret = -EINVAL;
		goto out;
	}

	ret = firmware_name_to_uuid(firmwarep, &uuid);
	if (ret)
		goto out;

	/* Loop for every FIP entry searching for uuid */
	while (true) {
		ret = blk_read_fip_toc_entry(desc, offset, pos,
					     read_buf, &ent);
		if (ret < 0)
			goto out;

		pos += ret;

		ret = check_fip_toc_entry(&ent, &uuid, dent);
		if (ret != -EAGAIN)
			break;
	}

out:
	free(read_buf);
	return ret;
}

#ifdef CONFIG_CMD_UBIFS
static int ubi_parse_fip_firmware(struct firmware *firmwarep,
				  char *ubi_vol,
				  struct fip_toc_entry *dent)
{
	struct fip_toc_header hdr;
	struct fip_toc_entry ent;
	struct uuid uuid;
	unsigned int pos;
	int ret;

	ret = ubi_volume_read(ubi_vol, (char *)&hdr, 0, sizeof(hdr));
	if (ret)
		return ret;

	pos = sizeof(hdr);

	if (!validate_fip_toc_header(&hdr))
		return -EINVAL;

	ret = firmware_name_to_uuid(firmwarep, &uuid);
	if (ret)
		return ret;

	/* Loop for every FIP entry searching for uuid */
	while (true) {
		ret = ubi_volume_read(ubi_vol, (char *)&ent, pos,
				      sizeof(ent));
		if (ret)
			return ret;

		ret = check_fip_toc_entry(&ent, &uuid, dent);
		if (ret != -EAGAIN)
			break;

		pos += sizeof(ent);
	}

	return ret;
}
#endif

static int parse_fip_firmware(struct firmware *firmwarep,
			      struct fip_storage_info *info,
			      struct fip_toc_entry *dent)
{
	switch (info->storage_interface) {
	case FIP_STORAGE_INTERFACE_BLK:
		return blk_parse_fip_firmware(firmwarep, info->desc,
					      &info->part_info,
					      info->part_offset,
					      dent);
#ifdef CONFIG_CMD_UBIFS
	case FIP_STORAGE_INTERFACE_UBI:
		return ubi_parse_fip_firmware(firmwarep,
					      info->ubi_volume,
					      dent);
#endif
	default:
		return -EINVAL;
	}
}

static int blk_read_fip_firmware(struct firmware *firmwarep,
				 struct blk_desc *desc,
				 struct disk_partition *part_info,
				 unsigned int part_offset,
				 const struct fip_toc_entry *ent)
{
	unsigned int offset = part_info->start + part_offset;
	unsigned int pos, to_read, read = 0;
	unsigned long long blkstart;
	size_t size = ent->size;
	unsigned int blkcnt;
	char *read_buf;
	int i, ret;

	read_buf = malloc(desc->blksz);
	if (!read_buf)
		return -ENOMEM;

	blkcnt = BLOCK_CNT(size + firmwarep->offset, desc);
	blkstart = ent->offset_address + firmwarep->offset;
	pos = do_div(blkstart, desc->blksz);

	/* Read data in the middle of a block */
	if (pos) {
		to_read = MIN(desc->blksz - pos, size);
		ret = blk_dread(desc, offset + blkstart, 1, read_buf);
		if (ret != 1) {
			ret = -EINVAL;
			goto out;
		}

		memcpy((u8 *)firmwarep->data, read_buf + pos, to_read);
		read += to_read;
		blkstart++;
	}

	/* Consume all the remaining block */
	for (i = 0; i < blkcnt && read < size; i++) {
		to_read = MIN(desc->blksz, size - read);
		ret = blk_dread(desc, offset + blkstart + i, 1, read_buf);
		if (ret != 1) {
			ret = -EINVAL;
			goto out;
		}

		memcpy((u8 *)firmwarep->data + read, read_buf, to_read);
		read += to_read;
	}

	ret = read;

out:
	free(read_buf);
	return ret;
}

#ifdef CONFIG_CMD_UBIFS
static int ubi_read_fip_firmware(struct firmware *firmwarep,
				 char *ubi_vol,
				 const struct fip_toc_entry *ent)
{
	unsigned int offset = firmwarep->offset;
	size_t size = ent->size;
	int ret;

	ret = ubi_volume_read(ubi_vol,
			      (u8 *)firmwarep->data,
			      ent->offset_address + offset,
			      size - offset);
	if (ret)
		return ret;

	return size - firmwarep->offset;
}
#endif

static int read_fip_firmware(struct firmware *firmwarep,
			     struct fip_storage_info *info,
			     const struct fip_toc_entry *dent)
{
	switch (info->storage_interface) {
	case FIP_STORAGE_INTERFACE_BLK:
		return blk_read_fip_firmware(firmwarep, info->desc,
					     &info->part_info,
					     info->part_offset,
					     dent);
#ifdef CONFIG_CMD_UBIFS
	case FIP_STORAGE_INTERFACE_UBI:
		return ubi_read_fip_firmware(firmwarep,
					     info->ubi_volume,
					     dent);
#endif
	default:
		return -EINVAL;
	}
}

static int fw_parse_storage_info(struct udevice *dev,
				 struct fip_storage_info *info)
{
	char *storage_interface, *dev_part, *ubi_mtdpart, *ubi_volume;
	struct device_plat *plat = dev_get_plat(dev);
	int ret;

	storage_interface = env_get("storage_interface");
	dev_part = env_get("fw_dev_part");
	ubi_mtdpart = env_get("fw_ubi_mtdpart");
	ubi_volume = env_get("fw_ubi_volume");
	info->part_offset = env_get_hex("fw_partoffset", 0);

	if (storage_interface && dev_part) {
		int part;

		part = part_get_info_by_dev_and_name_or_num(storage_interface,
							    dev_part,
							    &info->desc,
							    &info->part_info, 1);
		if (part < 0)
			return part;

		info->storage_interface = FIP_STORAGE_INTERFACE_BLK;

		return 0;
	}

	if (storage_interface && ubi_mtdpart && ubi_volume) {
		if (strcmp("ubi", storage_interface))
			return -ENODEV;

		ret = generic_fw_loader_ubi_select(ubi_mtdpart);
		if (ret)
			return ret;

		info->ubi_volume = ubi_volume;
		info->storage_interface = FIP_STORAGE_INTERFACE_UBI;

		return 0;
	}

	info->part_offset = plat->partoffset;

	if (plat->phandlepart.phandle) {
		struct udevice *disk_dev;
		ofnode node;
		int part;

		node = ofnode_get_by_phandle(plat->phandlepart.phandle);

		ret = device_get_global_by_ofnode(node, &disk_dev);
		if (ret)
			return ret;

		info->desc = blk_get_by_device(disk_dev);
		if (!info->desc)
			return -ENODEV;

		part = plat->phandlepart.partition;
		if (part >= 1)
			ret = part_get_info(info->desc, part,
					    &info->part_info);
		else
			ret = part_get_info_whole_disk(info->desc,
						       &info->part_info);

		info->storage_interface = FIP_STORAGE_INTERFACE_BLK;

		return ret;
	}

	if (plat->mtdpart && plat->ubivol) {
		ret = generic_fw_loader_ubi_select(plat->mtdpart);
		if (ret)
			return ret;

		info->ubi_volume = plat->ubivol;
		info->storage_interface = FIP_STORAGE_INTERFACE_UBI;

		return 0;
	}

	return -EINVAL;
}

/**
 * fw_get_fip_firmware - load firmware into an allocated buffer.
 * @dev: An instance of a driver.
 *
 * Return: Size of total read, negative value when error.
 */
static int fw_get_fip_firmware(struct udevice *dev)
{
	struct fip_toc_entry ent;
	struct fip_storage_info info = { };
	int ret;

	ret = fw_parse_storage_info(dev, &info);
	if (ret)
		return ret;

	struct firmware *firmwarep = dev_get_priv(dev);

	if (!firmwarep)
		return -EINVAL;

	ret = parse_fip_firmware(firmwarep, &info, &ent);
	if (ret)
		return ret;

	if (ent.size + firmwarep->offset > firmwarep->size) {
		log_err("Not enough space to read firmware\n");
		return -ENOMEM;
	}

	ret = read_fip_firmware(firmwarep, &info, &ent);
	if (ret < 0)
		log_err("Failed to read %s from FIP: %d.\n",
			firmwarep->name, ret);

	return ret;
}

/**
 * fw_get_fip_firmware_size - get firmware size.
 * @dev: An instance of a driver.
 *
 * Return: Size of firmware, negative value when error.
 */
static int fw_get_fip_firmware_size(struct udevice *dev)
{
	struct fip_toc_entry ent;
	struct fip_storage_info info = { };
	int ret;

	ret = fw_parse_storage_info(dev, &info);
	if (ret)
		return ret;

	struct firmware *firmwarep = dev_get_priv(dev);

	if (!firmwarep)
		return -EINVAL;

	ret = parse_fip_firmware(firmwarep, &info, &ent);
	if (ret)
		return ret;

	return ent.size;
}

static int fip_loader_probe(struct udevice *dev)
{
	struct device_plat *plat = dev_get_plat(dev);
	int ret;

	ret = generic_fw_loader_probe(dev);
	if (ret)
		return ret;

	plat->get_firmware = fw_get_fip_firmware;
	plat->get_size = fw_get_fip_firmware_size;

	return 0;
};

static int fip_loader_of_to_plat(struct udevice *dev)
{
	struct device_plat *plat = dev_get_plat(dev);
	ofnode fip_loader_node = dev_ofnode(dev);
	int ret;

	ret = generic_fw_loader_of_to_plat(dev);
	if (ret)
		return ret;

	/* Node validation is already done by the generic function */
	ofnode_read_u32(fip_loader_node, "partoffset",
			&plat->partoffset);

	return 0;
}

static const struct udevice_id fip_loader_ids[] = {
	{ .compatible = "u-boot,fip-loader"},
	{ }
};

U_BOOT_DRIVER(fip_loader) = {
	.name		= "fip-loader",
	.id		= UCLASS_FIP_FIRMWARE_LOADER,
	.of_match	= fip_loader_ids,
	.probe		= fip_loader_probe,
	.of_to_plat	= fip_loader_of_to_plat,
	.plat_auto	= sizeof(struct device_plat),
	.priv_auto	= sizeof(struct firmware),
};

UCLASS_DRIVER(fip_loader) = {
	.id		= UCLASS_FIP_FIRMWARE_LOADER,
	.name		= "fip-loader",
};
