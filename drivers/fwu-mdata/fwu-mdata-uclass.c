// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_FWU_MDATA

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>

#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

/**
 * fwu_get_mdata_part_num() - Get the FWU metadata partition numbers
 * @dev: FWU metadata device
 * @mdata_parts: array for storing the metadata partition numbers
 *
 * Get the partition numbers on the storage device on which the
 * FWU metadata is stored. Two partition numbers will be returned.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_mdata_part_num(struct udevice *dev, uint *mdata_parts)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->get_mdata_part_num) {
		log_debug("get_mdata_part_num() method not defined\n");
		return -ENOSYS;
	}

	return ops->get_mdata_part_num(dev, mdata_parts);
}

/**
 * fwu_read_mdata_partition() - Read the FWU metadata from a partition
 * @dev: FWU metadata device
 * @mdata: Copy of the FWU metadata
 * @part_num: Partition number from which FWU metadata is to be read
 *
 * Read the FWU metadata from the specified partition number
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_read_mdata_partition(struct udevice *dev, struct fwu_mdata *mdata,
			     uint part_num)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->read_mdata_partition) {
		log_debug("read_mdata_partition() method not defined\n");
		return -ENOSYS;
	}

	return ops->read_mdata_partition(dev, mdata, part_num);
}

/**
 * fwu_write_mdata_partition() - Write the FWU metadata to a partition
 * @dev: FWU metadata device
 * @mdata: Copy of the FWU metadata
 * @part_num: Partition number to which FWU metadata is to be written
 *
 * Write the FWU metadata to the specified partition number
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_write_mdata_partition(struct udevice *dev, struct fwu_mdata *mdata,
			      uint part_num)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->write_mdata_partition) {
		log_debug("write_mdata_partition() method not defined\n");
		return -ENOSYS;
	}

	return ops->write_mdata_partition(dev, mdata, part_num);
}

/**
 * fwu_mdata_check() - Check if the FWU metadata is valid
 * @dev: FWU metadata device
 *
 * Validate both copies of the FWU metadata. If one of the copies
 * has gone bad, restore it from the other copy.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_mdata_check(struct udevice *dev)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->check_mdata) {
		log_debug("check_mdata() method not defined\n");
		return -ENOSYS;
	}

	return ops->check_mdata(dev);
}

/**
 * fwu_get_mdata() - Get a FWU metadata copy
 * @dev: FWU metadata device
 * @mdata: Copy of the FWU metadata
 *
 * Get a valid copy of the FWU metadata.
 *
 * Note: This function is to be called first when modifying any fields
 * in the metadata. The sequence of calls to modify any field in the
 * metadata would  be 1) fwu_get_mdata 2) Modify metadata, followed by
 * 3) fwu_update_mdata
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_mdata(struct udevice *dev, struct fwu_mdata *mdata)
{
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->get_mdata) {
		log_debug("get_mdata() method not defined\n");
		return -ENOSYS;
	}

	return ops->get_mdata(dev, mdata);
}

/**
 * fwu_update_mdata() - Update the FWU metadata
 * @dev: FWU metadata device
 * @mdata: Copy of the FWU metadata
 *
 * Update the FWU metadata structure by writing to the
 * FWU metadata partitions.
 *
 * Note: This function is not to be called directly to update the
 * metadata fields. The sequence of function calls should be
 * 1) fwu_get_mdata() 2) Modify the medata fields 3) fwu_update_mdata()
 *
 * The sequence of updating the partitions should be, update the
 * primary metadata partition (first partition encountered), followed
 * by updating the secondary partition. With this update sequence, in
 * the rare scenario that the two metadata partitions are valid but do
 * not match, maybe due to power outage at the time of updating the
 * metadata copies, the secondary partition can be updated from the
 * primary.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_update_mdata(struct udevice *dev, struct fwu_mdata *mdata)
{
	void *buf;
	const struct fwu_mdata_ops *ops = device_get_ops(dev);

	if (!ops->update_mdata) {
		log_debug("get_mdata() method not defined\n");
		return -ENOSYS;
	}

	/*
	 * Calculate the crc32 for the updated FWU metadata
	 * and put the updated value in the FWU metadata crc32
	 * field
	 */
	buf = &mdata->version;
	mdata->crc32 = crc32(0, buf, sizeof(*mdata) - sizeof(u32));

	return ops->update_mdata(dev, mdata);
}

UCLASS_DRIVER(fwu_mdata) = {
	.id		= UCLASS_FWU_MDATA,
	.name		= "fwu-mdata",
};
