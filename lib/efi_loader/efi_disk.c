/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
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

	diskobj = container_of(this, struct efi_disk_obj, ops);
	if (!(desc = blk_get_dev(diskobj->ifname, diskobj->dev_index)))
		return EFI_EXIT(EFI_DEVICE_ERROR);
	blksz = desc->blksz;
	blocks = buffer_size / blksz;
	lba += diskobj->offset;

	debug("EFI: %s:%d blocks=%x lba=%"PRIx64" blksz=%x dir=%d\n", __func__,
	      __LINE__, blocks, lba, blksz, direction);

	/* We only support full block access */
	if (buffer_size & (blksz - 1))
		return EFI_EXIT(EFI_DEVICE_ERROR);

	if (direction == EFI_DISK_READ)
		n = blk_dread(desc, lba, blocks, buffer);
	else
		n = blk_dwrite(desc, lba, blocks, buffer);

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	debug("EFI: %s:%d n=%lx blocks=%x\n", __func__, __LINE__, n, blocks);

	if (n != blocks)
		return EFI_EXIT(EFI_DEVICE_ERROR);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t efi_disk_read_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer)
{
	void *real_buffer = buffer;
	efi_status_t r;

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	if (buffer_size > EFI_LOADER_BOUNCE_BUFFER_SIZE) {
		r = efi_disk_read_blocks(this, media_id, lba,
			EFI_LOADER_BOUNCE_BUFFER_SIZE, buffer);
		if (r != EFI_SUCCESS)
			return r;
		return efi_disk_read_blocks(this, media_id, lba +
			EFI_LOADER_BOUNCE_BUFFER_SIZE / this->media->block_size,
			buffer_size - EFI_LOADER_BOUNCE_BUFFER_SIZE,
			buffer + EFI_LOADER_BOUNCE_BUFFER_SIZE);
	}

	real_buffer = efi_bounce_buffer;
#endif

	EFI_ENTRY("%p, %x, %"PRIx64", %lx, %p", this, media_id, lba,
		  buffer_size, buffer);

	r = efi_disk_rw_blocks(this, media_id, lba, buffer_size, real_buffer,
			       EFI_DISK_READ);

	/* Copy from bounce buffer to real buffer if necessary */
	if ((r == EFI_SUCCESS) && (real_buffer != buffer))
		memcpy(buffer, real_buffer, buffer_size);

	return EFI_EXIT(r);
}

static efi_status_t efi_disk_write_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer)
{
	void *real_buffer = buffer;
	efi_status_t r;

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	if (buffer_size > EFI_LOADER_BOUNCE_BUFFER_SIZE) {
		r = efi_disk_write_blocks(this, media_id, lba,
			EFI_LOADER_BOUNCE_BUFFER_SIZE, buffer);
		if (r != EFI_SUCCESS)
			return r;
		return efi_disk_write_blocks(this, media_id, lba +
			EFI_LOADER_BOUNCE_BUFFER_SIZE / this->media->block_size,
			buffer_size - EFI_LOADER_BOUNCE_BUFFER_SIZE,
			buffer + EFI_LOADER_BOUNCE_BUFFER_SIZE);
	}

	real_buffer = efi_bounce_buffer;
#endif

	EFI_ENTRY("%p, %x, %"PRIx64", %lx, %p", this, media_id, lba,
		  buffer_size, buffer);

	/* Populate bounce buffer if necessary */
	if (real_buffer != buffer)
		memcpy(real_buffer, buffer, buffer_size);

	r = efi_disk_rw_blocks(this, media_id, lba, buffer_size, real_buffer,
			       EFI_DISK_WRITE);

	return EFI_EXIT(r);
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

static void efi_disk_add_dev(const char *name,
			     const char *if_typename,
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
	diskobj->ifname = if_typename;
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
				    const char *if_typename,
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
		snprintf(devname, sizeof(devname), "%s%d:%d", if_typename,
			 diskid, part);
		efi_disk_add_dev(devname, if_typename, desc, diskid,
				 info.start);
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
 * TODO(sjg@chromium.org): Actually with CONFIG_BLK, U-Boot does have this.
 * Consider converting the code to look up devices as needed. The EFI device
 * could be a child of the UCLASS_BLK block device, perhaps.
 *
 * This gets called from do_bootefi_exec().
 */
int efi_disk_register(void)
{
	int disks = 0;
#ifdef CONFIG_BLK
	struct udevice *dev;

	for (uclass_first_device(UCLASS_BLK, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);
		const char *if_typename = dev->driver->name;

		printf("Scanning disk %s...\n", dev->name);
		efi_disk_add_dev(dev->name, if_typename, desc, desc->devnum, 0);
		disks++;

		/*
		* El Torito images show up as block devices in an EFI world,
		* so let's create them here
		*/
		disks += efi_disk_create_eltorito(desc, if_typename,
						  desc->devnum);
	}
#else
	int i, if_type;

	/* Search for all available disk devices */
	for (if_type = 0; if_type < IF_TYPE_COUNT; if_type++) {
		const struct blk_driver *cur_drvr;
		const char *if_typename;

		cur_drvr = blk_driver_lookup_type(if_type);
		if (!cur_drvr)
			continue;

		if_typename = cur_drvr->if_typename;
		printf("Scanning disks on %s...\n", if_typename);
		for (i = 0; i < 4; i++) {
			struct blk_desc *desc;
			char devname[32] = { 0 }; /* dp->str is u16[32] long */

			desc = blk_get_devnum_by_type(if_type, i);
			if (!desc)
				continue;
			if (desc->type == DEV_TYPE_UNKNOWN)
				continue;

			snprintf(devname, sizeof(devname), "%s%d",
				 if_typename, i);
			efi_disk_add_dev(devname, if_typename, desc, i, 0);
			disks++;

			/*
			 * El Torito images show up as block devices
			 * in an EFI world, so let's create them here
			 */
			disks += efi_disk_create_eltorito(desc, if_typename, i);
		}
	}
#endif
	printf("Found %d disks\n", disks);

	return 0;
}
