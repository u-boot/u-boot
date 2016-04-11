/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <inttypes.h>
#include <part.h>
#include <malloc.h>

static const efi_guid_t efi_block_io_guid = BLOCK_IO_GUID;

struct efi_disk_obj {
	/* Generic EFI object parent class data */
	struct efi_object parent;
	/* EFI Interface callback struct for block I/O */
	struct efi_block_io ops;
	/* U-Boot ifname for block device */
	const char *ifname;
	/* U-Boot dev_index for block device */
	int dev_index;
	/* EFI Interface Media descriptor struct, referenced by ops */
	struct efi_block_io_media media;
	/* EFI device path to this block device */
	struct efi_device_path_file_path *dp;
	/* Offset into disk for simple partitions */
	lbaint_t offset;
};

static efi_status_t efi_disk_open_block(void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct efi_disk_obj *diskobj = handle;

	*protocol_interface = &diskobj->ops;

	return EFI_SUCCESS;
}

static efi_status_t efi_disk_open_dp(void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct efi_disk_obj *diskobj = handle;

	*protocol_interface = diskobj->dp;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_disk_reset(struct efi_block_io *this,
			char extended_verification)
{
	EFI_ENTRY("%p, %x", this, extended_verification);
	return EFI_EXIT(EFI_DEVICE_ERROR);
}

enum efi_disk_direction {
	EFI_DISK_READ,
	EFI_DISK_WRITE,
};

static efi_status_t EFIAPI efi_disk_rw_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer, enum efi_disk_direction direction)
{
	struct efi_disk_obj *diskobj;
	struct blk_desc *desc;
	int blksz;
	int blocks;
	unsigned long n;

	EFI_ENTRY("%p, %x, %"PRIx64", %lx, %p", this, media_id, lba,
		  buffer_size, buffer);

	diskobj = container_of(this, struct efi_disk_obj, ops);
	if (!(desc = blk_get_dev(diskobj->ifname, diskobj->dev_index)))
		return EFI_EXIT(EFI_DEVICE_ERROR);
	blksz = desc->blksz;
	blocks = buffer_size / blksz;
	lba += diskobj->offset;

#ifdef DEBUG_EFI
	printf("EFI: %s:%d blocks=%x lba=%"PRIx64" blksz=%x dir=%d\n", __func__,
	       __LINE__, blocks, lba, blksz, direction);
#endif

	/* We only support full block access */
	if (buffer_size & (blksz - 1))
		return EFI_EXIT(EFI_DEVICE_ERROR);

	if (direction == EFI_DISK_READ)
		n = desc->block_read(desc, lba, blocks, buffer);
	else
		n = desc->block_write(desc, lba, blocks, buffer);

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

#ifdef DEBUG_EFI
	printf("EFI: %s:%d n=%lx blocks=%x\n", __func__, __LINE__, n, blocks);
#endif
	if (n != blocks)
		return EFI_EXIT(EFI_DEVICE_ERROR);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t efi_disk_read_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer)
{
	return efi_disk_rw_blocks(this, media_id, lba, buffer_size, buffer,
				  EFI_DISK_READ);
}

static efi_status_t efi_disk_write_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer)
{
	return efi_disk_rw_blocks(this, media_id, lba, buffer_size, buffer,
				  EFI_DISK_WRITE);
}

static efi_status_t EFIAPI efi_disk_flush_blocks(struct efi_block_io *this)
{
	/* We always write synchronously */
	EFI_ENTRY("%p", this);
	return EFI_EXIT(EFI_SUCCESS);
}

static const struct efi_block_io block_io_disk_template = {
	.reset = &efi_disk_reset,
	.read_blocks = &efi_disk_read_blocks,
	.write_blocks = &efi_disk_write_blocks,
	.flush_blocks = &efi_disk_flush_blocks,
};

static void efi_disk_add_dev(char *name,
			     const struct block_drvr *cur_drvr,
			     const struct blk_desc *desc,
			     int dev_index,
			     lbaint_t offset)
{
	struct efi_disk_obj *diskobj;
	struct efi_device_path_file_path *dp;
	int objlen = sizeof(*diskobj) + (sizeof(*dp) * 2);

	diskobj = calloc(1, objlen);

	/* Fill in object data */
	diskobj->parent.protocols[0].guid = &efi_block_io_guid;
	diskobj->parent.protocols[0].open = efi_disk_open_block;
	diskobj->parent.protocols[1].guid = &efi_guid_device_path;
	diskobj->parent.protocols[1].open = efi_disk_open_dp;
	diskobj->parent.handle = diskobj;
	diskobj->ops = block_io_disk_template;
	diskobj->ifname = cur_drvr->name;
	diskobj->dev_index = dev_index;
	diskobj->offset = offset;

	/* Fill in EFI IO Media info (for read/write callbacks) */
	diskobj->media.removable_media = desc->removable;
	diskobj->media.media_present = 1;
	diskobj->media.block_size = desc->blksz;
	diskobj->media.io_align = desc->blksz;
	diskobj->media.last_block = desc->lba;
	diskobj->ops.media = &diskobj->media;

	/* Fill in device path */
	dp = (void*)&diskobj[1];
	diskobj->dp = dp;
	dp[0].dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE;
	dp[0].dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH;
	dp[0].dp.length = sizeof(*dp);
	ascii2unicode(dp[0].str, name);

	dp[1].dp.type = DEVICE_PATH_TYPE_END;
	dp[1].dp.sub_type = DEVICE_PATH_SUB_TYPE_END;
	dp[1].dp.length = sizeof(*dp);

	/* Hook up to the device list */
	list_add_tail(&diskobj->parent.link, &efi_obj_list);
}

static int efi_disk_create_eltorito(struct blk_desc *desc,
				    const struct block_drvr *cur_drvr,
				    int diskid)
{
	int disks = 0;
#ifdef CONFIG_ISO_PARTITION
	char devname[32] = { 0 }; /* dp->str is u16[32] long */
	disk_partition_t info;
	int part = 1;

	if (desc->part_type != PART_TYPE_ISO)
		return 0;

	while (!part_get_info(desc, part, &info)) {
		snprintf(devname, sizeof(devname), "%s%d:%d", cur_drvr->name,
			 diskid, part);
		efi_disk_add_dev(devname, cur_drvr, desc, diskid, info.start);
		part++;
		disks++;
	}
#endif

	return disks;
}

/*
 * U-Boot doesn't have a list of all online disk devices. So when running our
 * EFI payload, we scan through all of the potentially available ones and
 * store them in our object pool.
 *
 * This gets called from do_bootefi_exec().
 */
int efi_disk_register(void)
{
	const struct block_drvr *cur_drvr;
	int i;
	int disks = 0;

	/* Search for all available disk devices */
	for (cur_drvr = block_drvr; cur_drvr->name; cur_drvr++) {
		printf("Scanning disks on %s...\n", cur_drvr->name);
		for (i = 0; i < 4; i++) {
			struct blk_desc *desc;
			char devname[32] = { 0 }; /* dp->str is u16[32] long */

			desc = blk_get_dev(cur_drvr->name, i);
			if (!desc)
				continue;
			if (desc->type == DEV_TYPE_UNKNOWN)
				continue;

			snprintf(devname, sizeof(devname), "%s%d",
				 cur_drvr->name, i);
			efi_disk_add_dev(devname, cur_drvr, desc, i, 0);
			disks++;

			/*
			 * El Torito images show up as block devices
			 * in an EFI world, so let's create them here
			 */
			disks += efi_disk_create_eltorito(desc, cur_drvr, i);
		}
	}
	printf("Found %d disks\n", disks);

	return 0;
}
