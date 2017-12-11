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
	struct efi_device_path *dp;
	/* partition # */
	unsigned int part;
	/* handle to filesys proto (for partition objects) */
	struct efi_simple_file_system_protocol *volume;
	/* Offset into disk for simple partitions */
	lbaint_t offset;
	/* Internal block device */
	struct blk_desc *desc;
};

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

static efi_status_t efi_disk_rw_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, unsigned long buffer_size,
			void *buffer, enum efi_disk_direction direction)
{
	struct efi_disk_obj *diskobj;
	struct blk_desc *desc;
	int blksz;
	int blocks;
	unsigned long n;

	diskobj = container_of(this, struct efi_disk_obj, ops);
	desc = (struct blk_desc *) diskobj->desc;
	blksz = desc->blksz;
	blocks = buffer_size / blksz;
	lba += diskobj->offset;

	debug("EFI: %s:%d blocks=%x lba=%"PRIx64" blksz=%x dir=%d\n", __func__,
	      __LINE__, blocks, lba, blksz, direction);

	/* We only support full block access */
	if (buffer_size & (blksz - 1))
		return EFI_DEVICE_ERROR;

	if (direction == EFI_DISK_READ)
		n = blk_dread(desc, lba, blocks, buffer);
	else
		n = blk_dwrite(desc, lba, blocks, buffer);

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	debug("EFI: %s:%d n=%lx blocks=%x\n", __func__, __LINE__, n, blocks);

	if (n != blocks)
		return EFI_DEVICE_ERROR;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_disk_read_blocks(struct efi_block_io *this,
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

static efi_status_t EFIAPI efi_disk_write_blocks(struct efi_block_io *this,
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

/*
 * Find filesystem from a device-path.  The passed in path 'p' probably
 * contains one or more /File(name) nodes, so the comparison stops at
 * the first /File() node, and returns the pointer to that via 'rp'.
 * This is mostly intended to be a helper to map a device-path to an
 * efi_file_handle object.
 */
struct efi_simple_file_system_protocol *
efi_fs_from_path(struct efi_device_path *fp)
{
	struct efi_object *efiobj;
	struct efi_disk_obj *diskobj;

	efiobj = efi_dp_find_obj(fp, NULL);
	if (!efiobj)
		return NULL;

	diskobj = container_of(efiobj, struct efi_disk_obj, parent);

	return diskobj->volume;
}

/*
 * Create a device for a disk
 *
 * @name	not used
 * @if_typename interface name for block device
 * @desc	internal block device
 * @dev_index   device index for block device
 * @offset	offset into disk for simple partitions
 */
static void efi_disk_add_dev(const char *name,
			     const char *if_typename,
			     struct blk_desc *desc,
			     int dev_index,
			     lbaint_t offset,
			     unsigned int part)
{
	struct efi_disk_obj *diskobj;
	efi_status_t ret;

	/* Don't add empty devices */
	if (!desc->lba)
		return;

	diskobj = calloc(1, sizeof(*diskobj));
	if (!diskobj)
		goto out_of_memory;

	/* Hook up to the device list */
	efi_add_handle(&diskobj->parent);

	/* Fill in object data */
	diskobj->dp = efi_dp_from_part(desc, part);
	diskobj->part = part;
	ret = efi_add_protocol(diskobj->parent.handle, &efi_block_io_guid,
			       &diskobj->ops);
	if (ret != EFI_SUCCESS)
		goto out_of_memory;
	ret = efi_add_protocol(diskobj->parent.handle, &efi_guid_device_path,
			       diskobj->dp);
	if (ret != EFI_SUCCESS)
		goto out_of_memory;
	if (part >= 1) {
		diskobj->volume = efi_simple_file_system(desc, part,
							 diskobj->dp);
		ret = efi_add_protocol(diskobj->parent.handle,
				       &efi_simple_file_system_protocol_guid,
				       &diskobj->volume);
		if (ret != EFI_SUCCESS)
			goto out_of_memory;
	}
	diskobj->ops = block_io_disk_template;
	diskobj->ifname = if_typename;
	diskobj->dev_index = dev_index;
	diskobj->offset = offset;
	diskobj->desc = desc;

	/* Fill in EFI IO Media info (for read/write callbacks) */
	diskobj->media.removable_media = desc->removable;
	diskobj->media.media_present = 1;
	diskobj->media.block_size = desc->blksz;
	diskobj->media.io_align = desc->blksz;
	diskobj->media.last_block = desc->lba - offset;
	if (part != 0)
		diskobj->media.logical_partition = 1;
	diskobj->ops.media = &diskobj->media;
	return;
out_of_memory:
	printf("ERROR: Out of memory\n");
}

static int efi_disk_create_partitions(struct blk_desc *desc,
				      const char *if_typename,
				      int diskid,
				      const char *pdevname)
{
	int disks = 0;
	char devname[32] = { 0 }; /* dp->str is u16[32] long */
	disk_partition_t info;
	int part;

	/* Add devices for each partition */
	for (part = 1; part <= MAX_SEARCH_PARTITIONS; part++) {
		if (part_get_info(desc, part, &info))
			continue;
		snprintf(devname, sizeof(devname), "%s:%d", pdevname,
			 part);
		efi_disk_add_dev(devname, if_typename, desc, diskid,
				 info.start, part);
		disks++;
	}

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

	for (uclass_first_device_check(UCLASS_BLK, &dev);
	     dev;
	     uclass_next_device_check(&dev)) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);
		const char *if_typename = dev->driver->name;

		printf("Scanning disk %s...\n", dev->name);

		/* Add block device for the full device */
		efi_disk_add_dev(dev->name, if_typename, desc,
				 desc->devnum, 0, 0);

		disks++;

		/* Partitions show up as block devices in EFI */
		disks += efi_disk_create_partitions(desc, if_typename,
						    desc->devnum, dev->name);
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

			/* Add block device for the full device */
			efi_disk_add_dev(devname, if_typename, desc, i, 0, 0);
			disks++;

			/* Partitions show up as block devices in EFI */
			disks += efi_disk_create_partitions(desc, if_typename,
							    i, devname);
		}
	}
#endif
	printf("Found %d disks\n", disks);

	return 0;
}
