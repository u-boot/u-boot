// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <blk.h>
#include <dm.h>
#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <part_efi.h>

#include <dm/device-internal.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

#define PRIMARY_PART		BIT(0)
#define SECONDARY_PART		BIT(1)
#define BOTH_PARTS		(PRIMARY_PART | SECONDARY_PART)

#define MDATA_READ		BIT(0)
#define MDATA_WRITE		BIT(1)

static int gpt_get_mdata_partitions(struct blk_desc *desc,
				    u16 *primary_mpart,
				    u16 *secondary_mpart)
{
	int i, ret;
	u32 mdata_parts;
	efi_guid_t part_type_guid;
	struct disk_partition info;
	const efi_guid_t fwu_mdata_guid = FWU_MDATA_GUID;

	mdata_parts = 0;
	for (i = 1; i < MAX_SEARCH_PARTITIONS; i++) {
		if (part_get_info(desc, i, &info))
			continue;
		uuid_str_to_bin(info.type_guid, part_type_guid.b,
				UUID_STR_FORMAT_GUID);

		if (!guidcmp(&fwu_mdata_guid, &part_type_guid)) {
			++mdata_parts;
			if (!*primary_mpart)
				*primary_mpart = i;
			else
				*secondary_mpart = i;
		}
	}

	if (mdata_parts != 2) {
		log_err("Expect two copies of the FWU metadata instead of %d\n",
			mdata_parts);
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	return ret;
}

static int gpt_get_mdata_disk_part(struct blk_desc *desc,
				   struct disk_partition *info,
				   u32 part_num)
{
	int ret;
	char *mdata_guid_str = "8a7a84a0-8387-40f6-ab41-a8b9a5a60d23";

	ret = part_get_info(desc, part_num, info);
	if (ret < 0) {
		log_err("Unable to get the partition info for the FWU metadata part %d",
			part_num);
		return -1;
	}

	/* Check that it is indeed the FWU metadata partition */
	if (!strncmp(info->type_guid, mdata_guid_str, UUID_STR_LEN)) {
		/* Found the FWU metadata partition */
		return 0;
	}

	return -1;
}

static int gpt_read_write_mdata(struct blk_desc *desc,
				struct fwu_mdata *mdata,
				u8 access, u32 part_num)
{
	int ret;
	u32 len, blk_start, blkcnt;
	struct disk_partition info;

	ALLOC_CACHE_ALIGN_BUFFER_PAD(struct fwu_mdata, mdata_aligned, 1,
				     desc->blksz);

	ret = gpt_get_mdata_disk_part(desc, &info, part_num);
	if (ret < 0) {
		printf("Unable to get the FWU metadata partition\n");
		return -ENODEV;
	}

	len = sizeof(*mdata);
	blkcnt = BLOCK_CNT(len, desc);
	if (blkcnt > info.size) {
		log_err("Block count exceeds FWU metadata partition size\n");
		return -ERANGE;
	}

	blk_start = info.start;
	if (access == MDATA_READ) {
		if (blk_dread(desc, blk_start, blkcnt, mdata_aligned) != blkcnt) {
			log_err("Error reading FWU metadata from the device\n");
			return -EIO;
		}
		memcpy(mdata, mdata_aligned, sizeof(struct fwu_mdata));
	} else {
		if (blk_dwrite(desc, blk_start, blkcnt, mdata) != blkcnt) {
			log_err("Error writing FWU metadata to the device\n");
			return -EIO;
		}
	}

	return 0;
}

static int gpt_read_mdata(struct blk_desc *desc,
			  struct fwu_mdata *mdata, u32 part_num)
{
	return gpt_read_write_mdata(desc, mdata, MDATA_READ, part_num);
}

static int gpt_write_mdata_partition(struct blk_desc *desc,
					struct fwu_mdata *mdata,
					u32 part_num)
{
	return gpt_read_write_mdata(desc, mdata, MDATA_WRITE, part_num);
}

static int fwu_gpt_update_mdata(struct udevice *dev, struct fwu_mdata *mdata)
{
	int ret;
	struct blk_desc *desc;
	u16 primary_mpart = 0, secondary_mpart = 0;
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);

	desc = dev_get_uclass_plat(priv->blk_dev);
	if (!desc) {
		log_err("Block device not found\n");
		return -ENODEV;
	}

	ret = gpt_get_mdata_partitions(desc, &primary_mpart,
				       &secondary_mpart);

	if (ret < 0) {
		log_err("Error getting the FWU metadata partitions\n");
		return -ENODEV;
	}

	/* First write the primary partition*/
	ret = gpt_write_mdata_partition(desc, mdata, primary_mpart);
	if (ret < 0) {
		log_err("Updating primary FWU metadata partition failed\n");
		return ret;
	}

	/* And now the replica */
	ret = gpt_write_mdata_partition(desc, mdata, secondary_mpart);
	if (ret < 0) {
		log_err("Updating secondary FWU metadata partition failed\n");
		return ret;
	}

	return 0;
}

static int gpt_get_mdata(struct blk_desc *desc, struct fwu_mdata **mdata)
{
	int ret;
	u16 primary_mpart = 0, secondary_mpart = 0;

	ret = gpt_get_mdata_partitions(desc, &primary_mpart,
				       &secondary_mpart);

	if (ret < 0) {
		log_err("Error getting the FWU metadata partitions\n");
		return -ENODEV;
	}

	*mdata = malloc(sizeof(struct fwu_mdata));
	if (!*mdata) {
		log_err("Unable to allocate memory for reading FWU metadata\n");
		return -ENOMEM;
	}

	ret = gpt_read_mdata(desc, *mdata, primary_mpart);
	if (ret < 0) {
		log_err("Failed to read the FWU metadata from the device\n");
		return -EIO;
	}

	ret = fwu_verify_mdata(*mdata, 1);
	if (!ret)
		return 0;

	/*
	 * Verification of the primary FWU metadata copy failed.
	 * Try to read the replica.
	 */
	memset(*mdata, 0, sizeof(struct fwu_mdata));
	ret = gpt_read_mdata(desc, *mdata, secondary_mpart);
	if (ret < 0) {
		log_err("Failed to read the FWU metadata from the device\n");
		return -EIO;
	}

	ret = fwu_verify_mdata(*mdata, 0);
	if (!ret)
		return 0;

	/* Both the FWU metadata copies are corrupted. */
	return -1;
}

static int gpt_check_mdata_validity(struct udevice *dev)
{
	int ret;
	struct blk_desc *desc;
	struct fwu_mdata pri_mdata;
	struct fwu_mdata secondary_mdata;
	u16 primary_mpart = 0, secondary_mpart = 0;
	u16 valid_partitions, invalid_partitions;
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);

	desc = dev_get_uclass_plat(priv->blk_dev);
	if (!desc) {
		log_err("Block device not found\n");
		return -ENODEV;
	}

	/*
	 * Two FWU metadata partitions are expected.
	 * If we don't have two, user needs to create
	 * them first
	 */
	valid_partitions = 0;
	ret = gpt_get_mdata_partitions(desc, &primary_mpart,
				       &secondary_mpart);

	if (ret < 0) {
		log_err("Error getting the FWU metadata partitions\n");
		return -ENODEV;
	}

	ret = gpt_read_mdata(desc, &pri_mdata, primary_mpart);
	if (ret < 0) {
		log_err("Failed to read the FWU metadata from the device\n");
		goto secondary_read;
	}

	ret = fwu_verify_mdata(&pri_mdata, 1);
	if (!ret)
		valid_partitions |= PRIMARY_PART;

secondary_read:
	/* Now check the secondary partition */
	ret = gpt_read_mdata(desc, &secondary_mdata, secondary_mpart);
	if (ret < 0) {
		log_err("Failed to read the FWU metadata from the device\n");
		goto mdata_restore;
	}

	ret = fwu_verify_mdata(&secondary_mdata, 0);
	if (!ret)
		valid_partitions |= SECONDARY_PART;

mdata_restore:
	if (valid_partitions == (PRIMARY_PART | SECONDARY_PART)) {
		ret = -1;
		/*
		 * Before returning, check that both the
		 * FWU metadata copies are the same. If not,
		 * the FWU metadata copies need to be
		 * re-populated.
		 */
		if (!memcmp(&pri_mdata, &secondary_mdata,
			    sizeof(struct fwu_mdata))) {
			ret = 0;
		} else {
			log_err("Both FWU metadata copies are valid but do not match. Please check!\n");
		}
		goto out;
	}

	ret = -1;
	if (!(valid_partitions & BOTH_PARTS))
		goto out;

	invalid_partitions = valid_partitions ^ BOTH_PARTS;
	ret = gpt_write_mdata_partition(desc,
					(invalid_partitions == PRIMARY_PART) ?
					&secondary_mdata : &pri_mdata,
					(invalid_partitions == PRIMARY_PART) ?
					primary_mpart : secondary_mpart);

	if (ret < 0)
		log_err("Restoring %s FWU metadata partition failed\n",
			(invalid_partitions == PRIMARY_PART) ?
			"primary" : "secondary");

out:
	return ret;
}

static int fwu_gpt_mdata_check(struct udevice *dev)
{
	/*
	 * Check if both the copies of the FWU metadata are
	 * valid. If one has gone bad, restore it from the
	 * other good copy.
	 */
	return gpt_check_mdata_validity(dev);
}

static int fwu_gpt_get_mdata(struct udevice *dev, struct fwu_mdata **mdata)
{
	struct blk_desc *desc;
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);

	desc = dev_get_uclass_plat(priv->blk_dev);
	if (!desc) {
		log_err("Block device not found\n");
		return -ENODEV;
	}

	return gpt_get_mdata(desc, mdata);
}

static int fwu_get_mdata_device(struct udevice *dev, struct udevice **mdata_dev)
{
	u32 phandle;
	int ret, size;
	struct udevice *parent, *child;
	const fdt32_t *phandle_p = NULL;

	phandle_p = dev_read_prop(dev, "fwu-mdata-store", &size);
	if (!phandle_p) {
		log_err("fwu-mdata-store property not found\n");
		return -ENOENT;
	}

	phandle = fdt32_to_cpu(*phandle_p);

	ret = device_get_global_by_ofnode(ofnode_get_by_phandle(phandle),
					  &parent);
	if (ret)
		return ret;

	ret = -ENODEV;
	for (device_find_first_child(parent, &child); child;
	     device_find_next_child(&child)) {
		if (device_get_uclass_id(child) == UCLASS_BLK) {
			*mdata_dev = child;
			ret = 0;
		}
	}

	return ret;
}

static int fwu_mdata_gpt_blk_probe(struct udevice *dev)
{
	int ret;
	struct udevice *mdata_dev = NULL;
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);

	ret = fwu_get_mdata_device(dev, &mdata_dev);
	if (ret)
		return ret;

	priv->blk_dev = mdata_dev;

	return 0;
}

static const struct fwu_mdata_ops fwu_gpt_blk_ops = {
	.mdata_check = fwu_gpt_mdata_check,
	.get_mdata = fwu_gpt_get_mdata,
	.update_mdata = fwu_gpt_update_mdata,
};

static const struct udevice_id fwu_mdata_ids[] = {
	{ .compatible = "u-boot,fwu-mdata-gpt" },
	{ }
};

U_BOOT_DRIVER(fwu_mdata_gpt_blk) = {
	.name		= "fwu-mdata-gpt-blk",
	.id		= UCLASS_FWU_MDATA,
	.of_match	= fwu_mdata_ids,
	.ops		= &fwu_gpt_blk_ops,
	.probe		= fwu_mdata_gpt_blk_probe,
	.priv_auto	= sizeof(struct fwu_mdata_gpt_blk_priv),
};
