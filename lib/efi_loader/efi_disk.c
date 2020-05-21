// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <efi_loader.h>
#include <fs.h>
#include <part.h>
#include <malloc.h>

struct efi_system_partition efi_system_partition;

const efi_guid_t efi_block_io_guid = EFI_BLOCK_IO_PROTOCOL_GUID;

/**
 * struct efi_disk_obj - EFI disk object
 *
 * @header:	EFI object header
 * @ops:	EFI disk I/O protocol interface
 * @ifname:	interface name for block device
 * @dev_index:	device index of block device
 * @media:	block I/O media information
 * @dp:		device path to the block device
 * @part:	partition
 * @volume:	simple file system protocol of the partition
 * @offset:	offset into disk for simple partition
 * @desc:	internal block device descriptor
 */
struct efi_disk_obj {
	struct efi_object header;
	struct efi_block_io ops;
	const char *ifname;
	int dev_index;
	struct efi_block_io_media media;
	struct efi_device_path *dp;
	unsigned int part;
	struct efi_simple_file_system_protocol *volume;
	lbaint_t offset;
	struct blk_desc *desc;
};

/**
 * efi_disk_reset() - reset block device
 *
 * This function implements the Reset service of the EFI_BLOCK_IO_PROTOCOL.
 *
 * As U-Boot's block devices do not have a reset function simply return
 * EFI_SUCCESS.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * @extended_verification:	extended verification
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_reset(struct efi_block_io *this,
			char extended_verification)
{
	EFI_ENTRY("%p, %x", this, extended_verification);
	return EFI_EXIT(EFI_SUCCESS);
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

	EFI_PRINT("blocks=%x lba=%llx blksz=%x dir=%d\n",
		  blocks, lba, blksz, direction);

	/* We only support full block access */
	if (buffer_size & (blksz - 1))
		return EFI_BAD_BUFFER_SIZE;

	if (direction == EFI_DISK_READ)
		n = blk_dread(desc, lba, blocks, buffer);
	else
		n = blk_dwrite(desc, lba, blocks, buffer);

	/* We don't do interrupts, so check for timers cooperatively */
	efi_timer_check();

	EFI_PRINT("n=%lx blocks=%x\n", n, blocks);

	if (n != blocks)
		return EFI_DEVICE_ERROR;

	return EFI_SUCCESS;
}

/**
 * efi_disk_read_blocks() - reads blocks from device
 *
 * This function implements the ReadBlocks service of the EFI_BLOCK_IO_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * @media_id:			id of the medium to be read from
 * @lba:			starting logical block for reading
 * @buffer_size:		size of the read buffer
 * @buffer:			pointer to the destination buffer
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_read_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer)
{
	void *real_buffer = buffer;
	efi_status_t r;

	if (!this)
		return EFI_INVALID_PARAMETER;
	/* TODO: check for media changes */
	if (media_id != this->media->media_id)
		return EFI_MEDIA_CHANGED;
	if (!this->media->media_present)
		return EFI_NO_MEDIA;
	/* media->io_align is a power of 2 */
	if ((uintptr_t)buffer & (this->media->io_align - 1))
		return EFI_INVALID_PARAMETER;
	if (lba * this->media->block_size + buffer_size >
	    this->media->last_block * this->media->block_size)
		return EFI_INVALID_PARAMETER;

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

	EFI_ENTRY("%p, %x, %llx, %zx, %p", this, media_id, lba,
		  buffer_size, buffer);

	r = efi_disk_rw_blocks(this, media_id, lba, buffer_size, real_buffer,
			       EFI_DISK_READ);

	/* Copy from bounce buffer to real buffer if necessary */
	if ((r == EFI_SUCCESS) && (real_buffer != buffer))
		memcpy(buffer, real_buffer, buffer_size);

	return EFI_EXIT(r);
}

/**
 * efi_disk_write_blocks() - writes blocks to device
 *
 * This function implements the WriteBlocks service of the
 * EFI_BLOCK_IO_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * @media_id:			id of the medium to be written to
 * @lba:			starting logical block for writing
 * @buffer_size:		size of the write buffer
 * @buffer:			pointer to the source buffer
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_write_blocks(struct efi_block_io *this,
			u32 media_id, u64 lba, efi_uintn_t buffer_size,
			void *buffer)
{
	void *real_buffer = buffer;
	efi_status_t r;

	if (!this)
		return EFI_INVALID_PARAMETER;
	if (this->media->read_only)
		return EFI_WRITE_PROTECTED;
	/* TODO: check for media changes */
	if (media_id != this->media->media_id)
		return EFI_MEDIA_CHANGED;
	if (!this->media->media_present)
		return EFI_NO_MEDIA;
	/* media->io_align is a power of 2 */
	if ((uintptr_t)buffer & (this->media->io_align - 1))
		return EFI_INVALID_PARAMETER;
	if (lba * this->media->block_size + buffer_size >
	    this->media->last_block * this->media->block_size)
		return EFI_INVALID_PARAMETER;

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

	EFI_ENTRY("%p, %x, %llx, %zx, %p", this, media_id, lba,
		  buffer_size, buffer);

	/* Populate bounce buffer if necessary */
	if (real_buffer != buffer)
		memcpy(real_buffer, buffer, buffer_size);

	r = efi_disk_rw_blocks(this, media_id, lba, buffer_size, real_buffer,
			       EFI_DISK_WRITE);

	return EFI_EXIT(r);
}

/**
 * efi_disk_flush_blocks() - flushes modified data to the device
 *
 * This function implements the FlushBlocks service of the
 * EFI_BLOCK_IO_PROTOCOL.
 *
 * As we always write synchronously nothing is done here.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @this:			pointer to the BLOCK_IO_PROTOCOL
 * Return:			status code
 */
static efi_status_t EFIAPI efi_disk_flush_blocks(struct efi_block_io *this)
{
	EFI_ENTRY("%p", this);
	return EFI_EXIT(EFI_SUCCESS);
}

static const struct efi_block_io block_io_disk_template = {
	.reset = &efi_disk_reset,
	.read_blocks = &efi_disk_read_blocks,
	.write_blocks = &efi_disk_write_blocks,
	.flush_blocks = &efi_disk_flush_blocks,
};

/**
 * efi_fs_from_path() - retrieve simple file system protocol
 *
 * Gets the simple file system protocol for a file device path.
 *
 * The full path provided is split into device part and into a file
 * part. The device part is used to find the handle on which the
 * simple file system protocol is installed.
 *
 * @full_path:	device path including device and file
 * Return:	simple file system protocol
 */
struct efi_simple_file_system_protocol *
efi_fs_from_path(struct efi_device_path *full_path)
{
	struct efi_object *efiobj;
	struct efi_handler *handler;
	struct efi_device_path *device_path;
	struct efi_device_path *file_path;
	efi_status_t ret;

	/* Split the path into a device part and a file part */
	ret = efi_dp_split_file_path(full_path, &device_path, &file_path);
	if (ret != EFI_SUCCESS)
		return NULL;
	efi_free_pool(file_path);

	/* Get the EFI object for the partition */
	efiobj = efi_dp_find_obj(device_path, NULL);
	efi_free_pool(device_path);
	if (!efiobj)
		return NULL;

	/* Find the simple file system protocol */
	ret = efi_search_protocol(efiobj, &efi_simple_file_system_protocol_guid,
				  &handler);
	if (ret != EFI_SUCCESS)
		return NULL;

	/* Return the simple file system protocol for the partition */
	return handler->protocol_interface;
}

/**
 * efi_fs_exists() - check if a partition bears a file system
 *
 * @desc:	block device descriptor
 * @part:	partition number
 * Return:	1 if a file system exists on the partition
 *		0 otherwise
 */
static int efi_fs_exists(struct blk_desc *desc, int part)
{
	if (fs_set_blk_dev_with_part(desc, part))
		return 0;

	if (fs_get_type() == FS_TYPE_ANY)
		return 0;

	fs_close();

	return 1;
}

/**
 * efi_disk_add_dev() - create a handle for a partition or disk
 *
 * @parent:		parent handle
 * @dp_parent:		parent device path
 * @if_typename:	interface name for block device
 * @desc:		internal block device
 * @dev_index:		device index for block device
 * @offset:		offset into disk for simple partitions
 * @part:		partition
 * @disk:		pointer to receive the created handle
 * Return:		disk object
 */
static efi_status_t efi_disk_add_dev(
				efi_handle_t parent,
				struct efi_device_path *dp_parent,
				const char *if_typename,
				struct blk_desc *desc,
				int dev_index,
				lbaint_t offset,
				unsigned int part,
				struct efi_disk_obj **disk)
{
	struct efi_disk_obj *diskobj;
	struct efi_object *handle;
	efi_status_t ret;

	/* Don't add empty devices */
	if (!desc->lba)
		return EFI_NOT_READY;

	diskobj = calloc(1, sizeof(*diskobj));
	if (!diskobj)
		return EFI_OUT_OF_RESOURCES;

	/* Hook up to the device list */
	efi_add_handle(&diskobj->header);

	/* Fill in object data */
	if (part) {
		struct efi_device_path *node = efi_dp_part_node(desc, part);

		diskobj->dp = efi_dp_append_node(dp_parent, node);
		efi_free_pool(node);
	} else {
		diskobj->dp = efi_dp_from_part(desc, part);
	}
	diskobj->part = part;

	/*
	 * Install the device path and the block IO protocol.
	 *
	 * InstallMultipleProtocolInterfaces() checks if the device path is
	 * already installed on an other handle and returns EFI_ALREADY_STARTED
	 * in this case.
	 */
	handle = &diskobj->header;
	ret = EFI_CALL(efi_install_multiple_protocol_interfaces(
			&handle, &efi_guid_device_path, diskobj->dp,
			&efi_block_io_guid, &diskobj->ops, NULL));
	if (ret != EFI_SUCCESS)
		return ret;

	/*
	 * On partitions or whole disks without partitions install the
	 * simple file system protocol if a file system is available.
	 */
	if ((part || desc->part_type == PART_TYPE_UNKNOWN) &&
	    efi_fs_exists(desc, part)) {
		diskobj->volume = efi_simple_file_system(desc, part,
							 diskobj->dp);
		ret = efi_add_protocol(&diskobj->header,
				       &efi_simple_file_system_protocol_guid,
				       diskobj->volume);
		if (ret != EFI_SUCCESS)
			return ret;
	}
	diskobj->ops = block_io_disk_template;
	diskobj->ifname = if_typename;
	diskobj->dev_index = dev_index;
	diskobj->offset = offset;
	diskobj->desc = desc;

	/* Fill in EFI IO Media info (for read/write callbacks) */
	diskobj->media.removable_media = desc->removable;
	diskobj->media.media_present = 1;
	/*
	 * MediaID is just an arbitrary counter.
	 * We have to change it if the medium is removed or changed.
	 */
	diskobj->media.media_id = 1;
	diskobj->media.block_size = desc->blksz;
	diskobj->media.io_align = desc->blksz;
	diskobj->media.last_block = desc->lba - offset;
	if (part)
		diskobj->media.logical_partition = 1;
	diskobj->ops.media = &diskobj->media;
	if (disk)
		*disk = diskobj;

	/* Store first EFI system partition */
	if (part && !efi_system_partition.if_type) {
		int r;
		struct disk_partition info;

		r = part_get_info(desc, part, &info);
		if (r)
			return EFI_DEVICE_ERROR;
		if (info.bootable & PART_EFI_SYSTEM_PARTITION) {
			efi_system_partition.if_type = desc->if_type;
			efi_system_partition.devnum = desc->devnum;
			efi_system_partition.part = part;
			EFI_PRINT("EFI system partition: %s %d:%d\n",
				  blk_get_if_type_name(desc->if_type),
				  desc->devnum, part);
		}
	}
	return EFI_SUCCESS;
}

/**
 * efi_disk_create_partitions() - create handles and protocols for partitions
 *
 * Create handles and protocols for the partitions of a block device.
 *
 * @parent:		handle of the parent disk
 * @desc:		block device
 * @if_typename:	interface type
 * @diskid:		device number
 * @pdevname:		device name
 * Return:		number of partitions created
 */
int efi_disk_create_partitions(efi_handle_t parent, struct blk_desc *desc,
			       const char *if_typename, int diskid,
			       const char *pdevname)
{
	int disks = 0;
	char devname[32] = { 0 }; /* dp->str is u16[32] long */
	struct disk_partition info;
	int part;
	struct efi_device_path *dp = NULL;
	efi_status_t ret;
	struct efi_handler *handler;

	/* Get the device path of the parent */
	ret = efi_search_protocol(parent, &efi_guid_device_path, &handler);
	if (ret == EFI_SUCCESS)
		dp = handler->protocol_interface;

	/* Add devices for each partition */
	for (part = 1; part <= MAX_SEARCH_PARTITIONS; part++) {
		if (part_get_info(desc, part, &info))
			continue;
		snprintf(devname, sizeof(devname), "%s:%d", pdevname,
			 part);
		ret = efi_disk_add_dev(parent, dp, if_typename, desc, diskid,
				       info.start, part, NULL);
		if (ret != EFI_SUCCESS) {
			printf("Adding partition %s failed\n", pdevname);
			continue;
		}
		disks++;
	}

	return disks;
}

/**
 * efi_disk_register() - register block devices
 *
 * U-Boot doesn't have a list of all online disk devices. So when running our
 * EFI payload, we scan through all of the potentially available ones and
 * store them in our object pool.
 *
 * This function is called in efi_init_obj_list().
 *
 * TODO(sjg@chromium.org): Actually with CONFIG_BLK, U-Boot does have this.
 * Consider converting the code to look up devices as needed. The EFI device
 * could be a child of the UCLASS_BLK block device, perhaps.
 *
 * Return:	status code
 */
efi_status_t efi_disk_register(void)
{
	struct efi_disk_obj *disk;
	int disks = 0;
	efi_status_t ret;
#ifdef CONFIG_BLK
	struct udevice *dev;

	for (uclass_first_device_check(UCLASS_BLK, &dev); dev;
	     uclass_next_device_check(&dev)) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);
		const char *if_typename = blk_get_if_type_name(desc->if_type);

		/* Add block device for the full device */
		printf("Scanning disk %s...\n", dev->name);
		ret = efi_disk_add_dev(NULL, NULL, if_typename,
					desc, desc->devnum, 0, 0, &disk);
		if (ret == EFI_NOT_READY) {
			printf("Disk %s not ready\n", dev->name);
			continue;
		}
		if (ret) {
			printf("ERROR: failure to add disk device %s, r = %lu\n",
			       dev->name, ret & ~EFI_ERROR_MASK);
			return ret;
		}
		disks++;

		/* Partitions show up as block devices in EFI */
		disks += efi_disk_create_partitions(
					&disk->header, desc, if_typename,
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
			ret = efi_disk_add_dev(NULL, NULL, if_typename, desc,
					       i, 0, 0, &disk);
			if (ret == EFI_NOT_READY) {
				printf("Disk %s not ready\n", devname);
				continue;
			}
			if (ret) {
				printf("ERROR: failure to add disk device %s, r = %lu\n",
				       devname, ret & ~EFI_ERROR_MASK);
				return ret;
			}
			disks++;

			/* Partitions show up as block devices in EFI */
			disks += efi_disk_create_partitions
						(&disk->header, desc,
						 if_typename, i, devname);
		}
	}
#endif
	printf("Found %d disks\n", disks);

	return EFI_SUCCESS;
}

/**
 * efi_disk_is_system_part() - check if handle refers to an EFI system partition
 *
 * @handle:	handle of partition
 *
 * Return:	true if handle refers to an EFI system partition
 */
bool efi_disk_is_system_part(efi_handle_t handle)
{
	struct efi_handler *handler;
	struct efi_disk_obj *diskobj;
	struct disk_partition info;
	efi_status_t ret;
	int r;

	/* check if this is a block device */
	ret = efi_search_protocol(handle, &efi_block_io_guid, &handler);
	if (ret != EFI_SUCCESS)
		return false;

	diskobj = container_of(handle, struct efi_disk_obj, header);

	r = part_get_info(diskobj->desc, diskobj->part, &info);
	if (r)
		return false;

	return !!(info.bootable & PART_EFI_SYSTEM_PARTITION);
}
