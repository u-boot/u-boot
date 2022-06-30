// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/tag.h>
#include <event.h>
#include <efi_loader.h>
#include <fs.h>
#include <log.h>
#include <part.h>
#include <malloc.h>

struct efi_system_partition efi_system_partition;

const efi_guid_t efi_block_io_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
const efi_guid_t efi_system_partition_guid = PARTITION_SYSTEM_GUID;

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
 * @dev:	associated DM device
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
	struct udevice *dev; /* TODO: move it to efi_object */
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

/**
 * efi_disk_is_removable() - check if the device is removable media
 * @handle:		efi object handle;
 *
 * Examine the device and determine if the device is a local block device
 * and removable media.
 *
 * Return:		true if removable, false otherwise
 */
bool efi_disk_is_removable(efi_handle_t handle)
{
	struct efi_handler *handler;
	struct efi_block_io *io;
	efi_status_t ret;

	ret = efi_search_protocol(handle, &efi_block_io_guid, &handler);
	if (ret != EFI_SUCCESS)
		return false;

	io = handler->protocol_interface;

	if (!io || !io->media)
		return false;

	return (bool)io->media->removable_media;
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
	int blksz;
	int blocks;
	unsigned long n;

	diskobj = container_of(this, struct efi_disk_obj, ops);
	blksz = diskobj->media.block_size;
	blocks = buffer_size / blksz;

	EFI_PRINT("blocks=%x lba=%llx blksz=%x dir=%d\n",
		  blocks, lba, blksz, direction);

	/* We only support full block access */
	if (buffer_size & (blksz - 1))
		return EFI_BAD_BUFFER_SIZE;

	if (CONFIG_IS_ENABLED(PARTITIONS) &&
	    device_get_uclass_id(diskobj->dev) == UCLASS_PARTITION) {
		if (direction == EFI_DISK_READ)
			n = dev_read(diskobj->dev, lba, blocks, buffer);
		else
			n = dev_write(diskobj->dev, lba, blocks, buffer);
	} else {
		/* dev is a block device (UCLASS_BLK) */
		struct blk_desc *desc;

		desc = dev_get_uclass_plat(diskobj->dev);
		if (direction == EFI_DISK_READ)
			n = blk_dread(desc, lba, blocks, buffer);
		else
			n = blk_dwrite(desc, lba, blocks, buffer);
	}

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
	/* media->io_align is a power of 2 or 0 */
	if (this->media->io_align &&
	    (uintptr_t)buffer & (this->media->io_align - 1))
		return EFI_INVALID_PARAMETER;
	if (lba * this->media->block_size + buffer_size >
	    (this->media->last_block + 1) * this->media->block_size)
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
	/* media->io_align is a power of 2 or 0 */
	if (this->media->io_align &&
	    (uintptr_t)buffer & (this->media->io_align - 1))
		return EFI_INVALID_PARAMETER;
	if (lba * this->media->block_size + buffer_size >
	    (this->media->last_block + 1) * this->media->block_size)
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
	efiobj = efi_dp_find_obj(device_path, NULL, NULL);
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
 * @part_info:		partition info
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
				struct disk_partition *part_info,
				unsigned int part,
				struct efi_disk_obj **disk)
{
	struct efi_disk_obj *diskobj;
	struct efi_object *handle;
	const efi_guid_t *guid = NULL;
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
	if (part_info) {
		struct efi_device_path *node = efi_dp_part_node(desc, part);
		struct efi_handler *handler;
		void *protocol_interface;

		/* Parent must expose EFI_BLOCK_IO_PROTOCOL */
		ret = efi_search_protocol(parent, &efi_block_io_guid, &handler);
		if (ret != EFI_SUCCESS)
			goto error;

		/*
		 * Link the partition (child controller) to the block device
		 * (controller).
		 */
		ret = efi_protocol_open(handler, &protocol_interface, NULL,
					&diskobj->header,
					EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
		if (ret != EFI_SUCCESS)
				goto error;

		diskobj->dp = efi_dp_append_node(dp_parent, node);
		efi_free_pool(node);
		diskobj->media.last_block = part_info->size - 1;
		if (part_info->bootable & PART_EFI_SYSTEM_PARTITION)
			guid = &efi_system_partition_guid;
	} else {
		diskobj->dp = efi_dp_from_part(desc, part);
		diskobj->media.last_block = desc->lba - 1;
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
			&efi_block_io_guid, &diskobj->ops,
			guid, NULL, NULL));
	if (ret != EFI_SUCCESS)
		goto error;

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
	if (part)
		diskobj->media.logical_partition = 1;
	diskobj->ops.media = &diskobj->media;
	if (disk)
		*disk = diskobj;

	EFI_PRINT("BlockIO: part %u, present %d, logical %d, removable %d"
		  ", last_block %llu\n",
		  diskobj->part,
		  diskobj->media.media_present,
		  diskobj->media.logical_partition,
		  diskobj->media.removable_media,
		  diskobj->media.last_block);

	/* Store first EFI system partition */
	if (part && !efi_system_partition.if_type) {
		if (part_info->bootable & PART_EFI_SYSTEM_PARTITION) {
			efi_system_partition.if_type = desc->if_type;
			efi_system_partition.devnum = desc->devnum;
			efi_system_partition.part = part;
			EFI_PRINT("EFI system partition: %s %x:%x\n",
				  blk_get_if_type_name(desc->if_type),
				  desc->devnum, part);
		}
	}
	return EFI_SUCCESS;
error:
	efi_delete_handle(&diskobj->header);
	return ret;
}

/*
 * Create a handle for a whole raw disk
 *
 * @dev		uclass device (UCLASS_BLK)
 *
 * Create an efi_disk object which is associated with @dev.
 * The type of @dev must be UCLASS_BLK.
 *
 * @return	0 on success, -1 otherwise
 */
static int efi_disk_create_raw(struct udevice *dev)
{
	struct efi_disk_obj *disk;
	struct blk_desc *desc;
	const char *if_typename;
	int diskid;
	efi_status_t ret;

	desc = dev_get_uclass_plat(dev);
	if_typename = blk_get_if_type_name(desc->if_type);
	diskid = desc->devnum;

	ret = efi_disk_add_dev(NULL, NULL, if_typename, desc,
			       diskid, NULL, 0, &disk);
	if (ret != EFI_SUCCESS) {
		if (ret == EFI_NOT_READY)
			log_notice("Disk %s not ready\n", dev->name);
		else
			log_err("Adding disk for %s failed\n", dev->name);

		return -1;
	}
	disk->dev = dev;
	if (dev_tag_set_ptr(dev, DM_TAG_EFI, &disk->header)) {
		efi_free_pool(disk->dp);
		efi_delete_handle(&disk->header);

		return -1;
	}

	return 0;
}

/*
 * Create a handle for a disk partition
 *
 * @dev		uclass device (UCLASS_PARTITION)
 *
 * Create an efi_disk object which is associated with @dev.
 * The type of @dev must be UCLASS_PARTITION.
 *
 * @return	0 on success, -1 otherwise
 */
static int efi_disk_create_part(struct udevice *dev)
{
	efi_handle_t parent;
	struct blk_desc *desc;
	const char *if_typename;
	struct disk_part *part_data;
	struct disk_partition *info;
	unsigned int part;
	int diskid;
	struct efi_handler *handler;
	struct efi_device_path *dp_parent;
	struct efi_disk_obj *disk;
	efi_status_t ret;

	if (dev_tag_get_ptr(dev_get_parent(dev), DM_TAG_EFI, (void **)&parent))
		return -1;

	desc = dev_get_uclass_plat(dev_get_parent(dev));
	if_typename = blk_get_if_type_name(desc->if_type);
	diskid = desc->devnum;

	part_data = dev_get_uclass_plat(dev);
	part = part_data->partnum;
	info = &part_data->gpt_part_info;

	ret = efi_search_protocol(parent, &efi_guid_device_path, &handler);
	if (ret != EFI_SUCCESS)
		return -1;
	dp_parent = (struct efi_device_path *)handler->protocol_interface;

	ret = efi_disk_add_dev(parent, dp_parent, if_typename, desc, diskid,
			       info, part, &disk);
	if (ret != EFI_SUCCESS) {
		log_err("Adding partition for %s failed\n", dev->name);
		return -1;
	}
	disk->dev = dev;
	if (dev_tag_set_ptr(dev, DM_TAG_EFI, &disk->header)) {
		efi_free_pool(disk->dp);
		efi_delete_handle(&disk->header);

		return -1;
	}

	return 0;
}

/*
 * Create efi_disk objects for a block device
 *
 * @dev		uclass device (UCLASS_BLK)
 *
 * Create efi_disk objects for partitions as well as a raw disk
 * which is associated with @dev.
 * The type of @dev must be UCLASS_BLK.
 * This function is expected to be called at EV_PM_POST_PROBE.
 *
 * @return	0 on success, -1 otherwise
 */
static int efi_disk_probe(void *ctx, struct event *event)
{
	struct udevice *dev;
	enum uclass_id id;
	struct blk_desc *desc;
	struct udevice *child;
	int ret;

	dev = event->data.dm.dev;
	id = device_get_uclass_id(dev);

	/* TODO: We won't support partitions in a partition */
	if (id != UCLASS_BLK)
		return 0;

	/*
	 * avoid creating duplicated objects now that efi_driver
	 * has already created an efi_disk at this moment.
	 */
	desc = dev_get_uclass_plat(dev);
	if (desc->if_type != IF_TYPE_EFI_LOADER) {
		ret = efi_disk_create_raw(dev);
		if (ret)
			return -1;
	}

	device_foreach_child(child, dev) {
		ret = efi_disk_create_part(child);
		if (ret)
			return -1;
	}

	return 0;
}

/*
 * Delete an efi_disk object for a whole raw disk
 *
 * @dev		uclass device (UCLASS_BLK)
 *
 * Delete an efi_disk object which is associated with @dev.
 * The type of @dev must be UCLASS_BLK.
 *
 * @return	0 on success, -1 otherwise
 */
static int efi_disk_delete_raw(struct udevice *dev)
{
	efi_handle_t handle;
	struct blk_desc *desc;
	struct efi_disk_obj *diskobj;

	if (dev_tag_get_ptr(dev, DM_TAG_EFI, (void **)&handle))
		return -1;

	desc = dev_get_uclass_plat(dev);
	if (desc->if_type != IF_TYPE_EFI_LOADER) {
		diskobj = container_of(handle, struct efi_disk_obj, header);
		efi_free_pool(diskobj->dp);
	}

	efi_delete_handle(handle);
	dev_tag_del(dev, DM_TAG_EFI);

	return 0;
}

/*
 * Delete an efi_disk object for a disk partition
 *
 * @dev		uclass device (UCLASS_PARTITION)
 *
 * Delete an efi_disk object which is associated with @dev.
 * The type of @dev must be UCLASS_PARTITION.
 *
 * @return	0 on success, -1 otherwise
 */
static int efi_disk_delete_part(struct udevice *dev)
{
	efi_handle_t handle;
	struct efi_disk_obj *diskobj;

	if (dev_tag_get_ptr(dev, DM_TAG_EFI, (void **)&handle))
		return -1;

	diskobj = container_of(handle, struct efi_disk_obj, header);

	efi_free_pool(diskobj->dp);
	efi_delete_handle(handle);
	dev_tag_del(dev, DM_TAG_EFI);

	return 0;
}

/*
 * Delete an efi_disk object for a block device
 *
 * @dev		uclass device (UCLASS_BLK or UCLASS_PARTITION)
 *
 * Delete an efi_disk object which is associated with @dev.
 * The type of @dev must be either UCLASS_BLK or UCLASS_PARTITION.
 * This function is expected to be called at EV_PM_PRE_REMOVE.
 *
 * @return	0 on success, -1 otherwise
 */
static int efi_disk_remove(void *ctx, struct event *event)
{
	enum uclass_id id;
	struct udevice *dev;

	dev = event->data.dm.dev;
	id = device_get_uclass_id(dev);

	if (id == UCLASS_BLK)
		return efi_disk_delete_raw(dev);
	else if (id == UCLASS_PARTITION)
		return efi_disk_delete_part(dev);
	else
		return 0;
}

efi_status_t efi_disk_init(void)
{
	int ret;

	ret = event_register("efi_disk add", EVT_DM_POST_PROBE,
			     efi_disk_probe, NULL);
	if (ret) {
		log_err("Event registration for efi_disk add failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	ret = event_register("efi_disk del", EVT_DM_PRE_REMOVE,
			     efi_disk_remove, NULL);
	if (ret) {
		log_err("Event registration for efi_disk del failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	return EFI_SUCCESS;
}
