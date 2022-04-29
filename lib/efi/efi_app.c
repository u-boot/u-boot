// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 *
 * EFI information obtained here:
 * http://wiki.phoenix.com/wiki/index.php/EFI_BOOT_SERVICES
 *
 * This file implements U-Boot running as an EFI application.
 */

#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <init.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/err.h>
#include <linux/types.h>
#include <efi.h>
#include <efi_api.h>
#include <sysreset.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

int efi_info_get(enum efi_entry_t type, void **datap, int *sizep)
{
	return -ENOSYS;
}

int efi_get_mmap(struct efi_mem_desc **descp, int *sizep, uint *keyp,
		 int *desc_sizep, uint *versionp)
{
	struct efi_priv *priv = efi_get_priv();
	struct efi_boot_services *boot = priv->sys_table->boottime;
	efi_uintn_t size, desc_size, key;
	struct efi_mem_desc *desc;
	efi_status_t ret;
	u32 version;

	/* Get the memory map so we can switch off EFI */
	size = 0;
	ret = boot->get_memory_map(&size, NULL, &key, &desc_size, &version);
	if (ret != EFI_BUFFER_TOO_SMALL)
		return log_msg_ret("get", -ENOMEM);

	desc = malloc(size);
	if (!desc)
		return log_msg_ret("mem", -ENOMEM);

	ret = boot->get_memory_map(&size, desc, &key, &desc_size, &version);
	if (ret)
		return log_msg_ret("get", -EINVAL);

	*descp = desc;
	*sizep = size;
	*desc_sizep = desc_size;
	*versionp = version;
	*keyp = key;

	return 0;
}

/**
 * efi_bind_block() - bind a new block device to an EFI device
 *
 * Binds a new top-level EFI_MEDIA device as well as a child block device so
 * that the block device can be accessed in U-Boot.
 *
 * The device can then be accessed using 'part list efi 0', 'fat ls efi 0:1',
 * for example, just like any other interface type.
 *
 * @handle: handle of the controller on which this driver is installed
 * @blkio: block io protocol proxied by this driver
 * @device_path: EFI device path structure for this
 * @len: Length of @device_path in bytes
 * @devp: Returns the bound device
 * Return: 0 if OK, -ve on error
 */
int efi_bind_block(efi_handle_t handle, struct efi_block_io *blkio,
		   struct efi_device_path *device_path, int len,
		   struct udevice **devp)
{
	struct efi_media_plat plat;
	struct udevice *dev;
	char name[18];
	int ret;

	plat.handle = handle;
	plat.blkio = blkio;
	plat.device_path = malloc(device_path->length);
	if (!plat.device_path)
		return log_msg_ret("path", -ENOMEM);
	memcpy(plat.device_path, device_path, device_path->length);
	ret = device_bind(dm_root(), DM_DRIVER_GET(efi_media), "efi_media",
			  &plat, ofnode_null(), &dev);
	if (ret)
		return log_msg_ret("bind", ret);

	snprintf(name, sizeof(name), "efi_media_%x", dev_seq(dev));
	device_set_name(dev, name);
	*devp = dev;

	return 0;
}

static efi_status_t setup_memory(struct efi_priv *priv)
{
	struct efi_boot_services *boot = priv->boot;
	efi_physical_addr_t addr;
	efi_status_t ret;
	int pages;

	/*
	 * Use global_data_ptr instead of gd since it is an assignment. There
	 * are very few assignments to global_data in U-Boot and this makes
	 * it easier to find them.
	 */
	global_data_ptr = efi_malloc(priv, sizeof(struct global_data), &ret);
	if (!global_data_ptr)
		return ret;
	memset(gd, '\0', sizeof(*gd));

	gd->malloc_base = (ulong)efi_malloc(priv, CONFIG_VAL(SYS_MALLOC_F_LEN),
					    &ret);
	if (!gd->malloc_base)
		return ret;
	pages = CONFIG_EFI_RAM_SIZE >> 12;

	/*
	 * Don't allocate any memory above 4GB. U-Boot is a 32-bit application
	 * so we want it to load below 4GB.
	 */
	addr = 1ULL << 32;
	ret = boot->allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				   priv->image_data_type, pages, &addr);
	if (ret) {
		log_info("(using pool %lx) ", ret);
		priv->ram_base = (ulong)efi_malloc(priv, CONFIG_EFI_RAM_SIZE,
						   &ret);
		if (!priv->ram_base)
			return ret;
		priv->use_pool_for_malloc = true;
	} else {
		log_info("(using allocated RAM address %lx) ", (ulong)addr);
		priv->ram_base = addr;
	}
	gd->ram_size = pages << 12;

	return 0;
}

/**
 * free_memory() - Free memory used by the U-Boot app
 *
 * This frees memory allocated in setup_memory(), in preparation for returning
 * to UEFI. It also zeroes the global_data pointer.
 *
 * @priv: Private EFI data
 */
static void free_memory(struct efi_priv *priv)
{
	struct efi_boot_services *boot = priv->boot;

	if (priv->use_pool_for_malloc)
		efi_free(priv, (void *)priv->ram_base);
	else
		boot->free_pages(priv->ram_base, gd->ram_size >> 12);

	efi_free(priv, (void *)gd->malloc_base);
	efi_free(priv, gd);
	global_data_ptr = NULL;
}

/**
 * devpath_is_partition() - Figure out if a device path is a partition
 *
 * Checks if a device path refers to a partition on some media device. This
 * works by checking for a valid partition number in a hard-driver media device
 * as the final component of the device path.
 *
 * @path:	device path
 * Return:	true if a partition, false if not
 *		(e.g. it might be media which contains partitions)
 */
static bool devpath_is_partition(const struct efi_device_path *path)
{
	const struct efi_device_path *p;
	bool was_part = false;

	for (p = path; p->type != DEVICE_PATH_TYPE_END;
	     p = (void *)p + p->length) {
		was_part = false;
		if (p->type == DEVICE_PATH_TYPE_MEDIA_DEVICE &&
		    p->sub_type == DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH) {
			struct efi_device_path_hard_drive_path *hd =
				(void *)path;

			if (hd->partition_number)
				was_part = true;
		}
	}

	return was_part;
}

/**
 * setup_block() - Find all block devices and setup EFI devices for them
 *
 * Partitions are ignored, since U-Boot has partition handling. Errors with
 * particular devices produce a warning but execution continues to try to
 * find others.
 *
 * Return: 0 if found, -ENOSYS if there is no boot-services table, -ENOTSUPP
 *	if a required protocol is not supported
 */
static int setup_block(void)
{
	efi_guid_t efi_blkio_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
	efi_guid_t efi_devpath_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
	efi_guid_t efi_pathutil_guid = EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID;
	efi_guid_t efi_pathtext_guid = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
	struct efi_boot_services *boot = efi_get_boot();
	struct efi_device_path_utilities_protocol *util;
	struct efi_device_path_to_text_protocol *text;
	struct efi_device_path *path;
	struct efi_block_io *blkio;
	efi_uintn_t num_handles;
	efi_handle_t *handle;
	int ret, i;

	if (!boot)
		return log_msg_ret("sys", -ENOSYS);

	/* Find all devices which support the block I/O protocol */
	ret = boot->locate_handle_buffer(BY_PROTOCOL, &efi_blkio_guid, NULL,
				  &num_handles, &handle);
	if (ret)
		return log_msg_ret("loc", -ENOTSUPP);
	log_debug("Found %d handles:\n", (int)num_handles);

	/* We need to look up the path size and convert it to text */
	ret = boot->locate_protocol(&efi_pathutil_guid, NULL, (void **)&util);
	if (ret)
		return log_msg_ret("util", -ENOTSUPP);
	ret = boot->locate_protocol(&efi_pathtext_guid, NULL, (void **)&text);
	if (ret)
		return log_msg_ret("text", -ENOTSUPP);

	for (i = 0; i < num_handles; i++) {
		struct udevice *dev;
		const u16 *name;
		bool is_part;
		int len;

		ret = boot->handle_protocol(handle[i], &efi_devpath_guid,
					    (void **)&path);
		if (ret) {
			log_warning("- devpath %d failed (ret=%d)\n", i, ret);
			continue;
		}

		ret = boot->handle_protocol(handle[i], &efi_blkio_guid,
					    (void **)&blkio);
		if (ret) {
			log_warning("- blkio %d failed (ret=%d)\n", i, ret);
			continue;
		}

		name = text->convert_device_path_to_text(path, true, false);
		is_part = devpath_is_partition(path);

		if (!is_part) {
			len = util->get_device_path_size(path);
			ret = efi_bind_block(handle[i], blkio, path, len, &dev);
			if (ret) {
				log_warning("- blkio bind %d failed (ret=%d)\n",
					    i, ret);
				continue;
			}
		} else {
			dev = NULL;
		}

		/*
		 * Show the device name if we created one. Otherwise indicate
		 * that it is a partition.
		 */
		printf("%2d: %-12s %ls\n", i, dev ? dev->name : "<partition>",
		       name);
	}
	boot->free_pool(handle);

	return 0;
}

/**
 * dm_scan_other() - Scan for UEFI devices that should be available to U-Boot
 *
 * This sets up block devices within U-Boot for those found in UEFI. With this,
 * U-Boot can access those devices
 *
 * @pre_reloc_only: true to only bind pre-relocation devices (ignored)
 * Returns: 0 on success, -ve on error
 */
int dm_scan_other(bool pre_reloc_only)
{
	if (gd->flags & GD_FLG_RELOC) {
		int ret;

		ret = setup_block();
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * efi_main() - Start an EFI image
 *
 * This function is called by our EFI start-up code. It handles running
 * U-Boot. If it returns, EFI will continue. Another way to get back to EFI
 * is via reset_cpu().
 */
efi_status_t EFIAPI efi_main(efi_handle_t image,
			     struct efi_system_table *sys_table)
{
	struct efi_priv local_priv, *priv = &local_priv;
	efi_status_t ret;

	/* Set up access to EFI data structures */
	ret = efi_init(priv, "App", image, sys_table);
	if (ret) {
		printf("Failed to set up U-Boot: err=%lx\n", ret);
		return ret;
	}
	efi_set_priv(priv);

	/*
	 * Set up the EFI debug UART so that printf() works. This is
	 * implemented in the EFI serial driver, serial_efi.c. The application
	 * can use printf() freely.
	 */
	debug_uart_init();

	ret = setup_memory(priv);
	if (ret) {
		printf("Failed to set up memory: ret=%lx\n", ret);
		return ret;
	}

	/*
	 * We could store the EFI memory map here, but it changes all the time,
	 * so this is only useful for debugging.
	 *
	 * ret = efi_store_memory_map(priv);
	 * if (ret)
	 *	return ret;
	 */

	printf("starting\n");

	board_init_f(GD_FLG_SKIP_RELOC);
	board_init_r(NULL, 0);
	free_memory(priv);

	return EFI_SUCCESS;
}

static void efi_exit(void)
{
	struct efi_priv *priv = efi_get_priv();

	free_memory(priv);
	printf("U-Boot EFI exiting\n");
	priv->boot->exit(priv->parent_image, EFI_SUCCESS, 0, NULL);
}

static int efi_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	efi_exit();

	return -EINPROGRESS;
}

static const struct udevice_id efi_sysreset_ids[] = {
	{ .compatible = "efi,reset" },
	{ }
};

static struct sysreset_ops efi_sysreset_ops = {
	.request = efi_sysreset_request,
};

U_BOOT_DRIVER(efi_sysreset) = {
	.name = "efi-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = efi_sysreset_ids,
	.ops = &efi_sysreset_ops,
};
