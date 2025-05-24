// SPDX-License-Identifier: GPL-2.0+
/*
 *  For the code moved from cmd/bootefi.c
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <bootflow.h>
#include <charset.h>
#include <dm.h>
#include <efi.h>
#include <efi_device_path.h>
#include <efi_loader.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>

static struct efi_device_path *bootefi_image_path;
static struct efi_device_path *bootefi_device_path;
static void *image_addr;
static size_t image_size;

/**
 * efi_get_image_parameters() - return image parameters
 *
 * @img_addr:		address of loaded image in memory
 * @img_size:		size of loaded image
 */
void efi_get_image_parameters(void **img_addr, size_t *img_size)
{
	*img_addr = image_addr;
	*img_size = image_size;
}

/**
 * efi_clear_bootdev() - clear boot device
 */
void efi_clear_bootdev(void)
{
	efi_free_pool(bootefi_device_path);
	efi_free_pool(bootefi_image_path);
	bootefi_device_path = NULL;
	bootefi_image_path = NULL;
	image_addr = NULL;
	image_size = 0;
}

/**
 * calculate_paths() - Calculate the device and image patch from strings
 *
 * @dev:		device, e.g. "MMC"
 * @devnr:		number of the device, e.g. "1:2"
 * @path:		path to file loaded
 * @device_pathp:	returns EFI device path
 * @image_pathp:	returns EFI image path
 * Return: EFI_SUCCESS on success, else error code
 */
static efi_status_t calculate_paths(const char *dev, const char *devnr,
				    const char *path,
				    struct efi_device_path **device_pathp,
				    struct efi_device_path **image_pathp)
{
	struct efi_device_path *image, *device;
	efi_status_t ret;

#if IS_ENABLED(CONFIG_NETDEVICES)
	if (!strcmp(dev, "Net") || !strcmp(dev, "Http")) {
		ret = efi_net_new_dp(dev, devnr, eth_get_dev());
		if (ret != EFI_SUCCESS)
			return ret;
	}
#endif

	ret = efi_dp_from_name(dev, devnr, path, &device, &image);
	if (ret != EFI_SUCCESS)
		return ret;

	*device_pathp = device;
	if (image) {
		/* FIXME: image should not contain device */
		struct efi_device_path *image_tmp = image;

		efi_dp_split_file_path(image, &device, &image);
		efi_free_pool(image_tmp);
	}
	*image_pathp = image;
	log_debug("- boot device %pD\n", device);
	if (image)
		log_debug("- image %pD\n", image);

	return EFI_SUCCESS;
}

/**
 * efi_set_bootdev() - set boot device
 *
 * This function is called when a file is loaded, e.g. via the 'load' command.
 * We use the path to this file to inform the UEFI binary about the boot device.
 *
 * For a valid image, it sets:
 *    - image_addr to the provided buffer
 *    - image_size to the provided buffer_size
 *    - bootefi_device_path to the EFI device-path
 *    - bootefi_image_path to the EFI image-path
 *
 * @dev:		device, e.g. "MMC"
 * @devnr:		number of the device, e.g. "1:2"
 * @path:		path to file loaded
 * @buffer:		buffer with file loaded
 * @buffer_size:	size of file loaded
 */
void efi_set_bootdev(const char *dev, const char *devnr, const char *path,
		     void *buffer, size_t buffer_size)
{
	efi_status_t ret;

	log_debug("dev=%s, devnr=%s, path=%s, buffer=%p, size=%zx\n", dev,
		  devnr, path, buffer, buffer_size);

	/* Forget overwritten image */
	if (buffer + buffer_size >= image_addr &&
	    image_addr + image_size >= buffer)
		efi_clear_bootdev();

	/* Remember only PE-COFF and FIT images */
	if (efi_check_pe(buffer, buffer_size, NULL) != EFI_SUCCESS) {
		if (IS_ENABLED(CONFIG_FIT) &&
		    !fit_check_format(buffer, IMAGE_SIZE_INVAL)) {
			/*
			 * FIT images of type EFI_OS are started via command
			 * bootm. We should not use their boot device with the
			 * bootefi command.
			 */
			buffer = 0;
			buffer_size = 0;
		} else {
			log_debug("- not remembering image\n");
			return;
		}
	}

	/* efi_set_bootdev() is typically called repeatedly, recover memory */
	efi_clear_bootdev();

	image_addr = buffer;
	image_size = buffer_size;

	ret = calculate_paths(dev, devnr, path, &bootefi_device_path,
			      &bootefi_image_path);
	if (ret) {
		log_debug("- efi_dp_from_name() failed, err=%lx\n", ret);
		efi_clear_bootdev();
	}
}

/**
 * efi_run_image() - run loaded UEFI image
 *
 * @source_buffer:	memory address of the UEFI image
 * @source_size:	size of the UEFI image
 * @dp_dev:		EFI device-path
 * @dp_img:		EFI image-path
 * Return:		status code
 */
static efi_status_t efi_run_image(void *source_buffer, efi_uintn_t source_size,
				  struct efi_device_path *dp_dev,
				  struct efi_device_path *dp_img)
{
	efi_handle_t handle;
	struct efi_device_path *msg_path, *file_path;
	efi_status_t ret;
	u16 *load_options;

	file_path = efi_dp_concat(dp_dev, dp_img, 0);
	msg_path = dp_img;

	log_info("Booting %pD\n", msg_path);

	ret = EFI_CALL(efi_load_image(false, efi_root, file_path, source_buffer,
				      source_size, &handle));
	if (ret != EFI_SUCCESS) {
		log_err("Loading image failed\n");
		goto out;
	}

	/* Transfer environment variable as load options */
	ret = efi_env_set_load_options(handle, "bootargs", &load_options);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = do_bootefi_exec(handle, load_options);

out:

	return ret;
}

/**
 * efi_binary_run_dp() - run loaded UEFI image
 *
 * @image:	memory address of the UEFI image
 * @size:	size of the UEFI image
 * @fdt:	device-tree
 * @initrd:	initrd
 * @initrd_sz:	initrd size
 * @dp_dev:	EFI device-path
 * @dp_img:	EFI image-path
 *
 * Execute an EFI binary image loaded at @image.
 * @size may be zero if the binary is loaded with U-Boot load command.
 *
 * Return:	status code
 */
static efi_status_t efi_binary_run_dp(void *image, size_t size, void *fdt,
				      void *initrd, size_t initrd_sz,
				      struct efi_device_path *dp_dev,
				      struct efi_device_path *dp_img)
{
	efi_status_t ret;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
			ret & ~EFI_ERROR_MASK);
		return -1;
	}

	ret = efi_install_fdt(fdt);
	if (ret != EFI_SUCCESS)
		return ret;

	ret = efi_install_initrd(initrd, initrd_sz);
	if (ret != EFI_SUCCESS)
		return ret;

	return efi_run_image(image, size, dp_dev, dp_img);
}

/**
 * efi_binary_run() - run loaded UEFI image
 *
 * @image:	memory address of the UEFI image
 * @size:	size of the UEFI image
 * @fdt:	device-tree
 * @initrd:	initrd
 * @initrd_sz:	initrd size
 *
 * Execute an EFI binary image loaded at @image.
 * @size may be zero if the binary is loaded with U-Boot load command.
 *
 * Return:	status code
 */
efi_status_t efi_binary_run(void *image, size_t size, void *fdt, void *initrd, size_t initrd_sz)
{
	efi_handle_t mem_handle = NULL;
	struct efi_device_path *file_path = NULL;
	efi_status_t ret;

	if (!bootefi_device_path || !bootefi_image_path) {
		log_debug("Not loaded from disk\n");
		/*
		 * Special case for efi payload not loaded from disk,
		 * such as 'bootefi hello' or for example payload
		 * loaded directly into memory via JTAG, etc:
		 */
		file_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
					    (uintptr_t)image, size);
		/*
		 * Make sure that device for device_path exist
		 * in load_image(). Otherwise, shell and grub will fail.
		 */
		ret = efi_install_multiple_protocol_interfaces(&mem_handle,
							       &efi_guid_device_path,
							       file_path, NULL);
		if (ret != EFI_SUCCESS)
			goto out;

		bootefi_device_path = file_path;
		bootefi_image_path = NULL;
	} else {
		log_debug("Loaded from disk\n");
	}

	ret = efi_binary_run_dp(image, size, fdt, initrd, initrd_sz, bootefi_device_path,
				bootefi_image_path);
out:
	if (mem_handle) {
		efi_status_t r;

		r = efi_uninstall_multiple_protocol_interfaces(mem_handle,
					&efi_guid_device_path, file_path, NULL);
		if (r != EFI_SUCCESS)
			log_err("Uninstalling protocol interfaces failed\n");
	}
	efi_free_pool(file_path);

	return ret;
}

/**
 * calc_dev_name() - Calculate the device name to give to EFI
 *
 * If not supported, this shows an error.
 *
 * Return name, or NULL if not supported
 */
static const char *calc_dev_name(struct bootflow *bflow)
{
	const struct udevice *media_dev;

	media_dev = dev_get_parent(bflow->dev);

	if (!bflow->blk) {
		if (device_get_uclass_id(media_dev) == UCLASS_ETH)
			return "Net";

		log_err("Cannot boot EFI app on media '%s'\n",
			dev_get_uclass_name(media_dev));

		return NULL;
	}

	if (device_get_uclass_id(media_dev) == UCLASS_MASS_STORAGE)
		return "usb";

	return blk_get_uclass_name(device_get_uclass_id(media_dev));
}

efi_status_t efi_bootflow_run(struct bootflow *bflow)
{
	struct efi_device_path *device, *image;
	const struct udevice *media_dev;
	struct blk_desc *desc = NULL;
	const char *dev_name;
	char devnum_str[9];
	efi_status_t ret;
	void *fdt;

	media_dev = dev_get_parent(bflow->dev);
	if (bflow->blk) {
		desc = dev_get_uclass_plat(bflow->blk);

		snprintf(devnum_str, sizeof(devnum_str), "%x:%x",
			 desc ? desc->devnum : dev_seq(media_dev), bflow->part);
	} else {
		*devnum_str = '\0';
	}

	dev_name = calc_dev_name(bflow);
	log_debug("dev_name '%s' devnum_str '%s' fname '%s' media_dev '%s'\n",
		  dev_name, devnum_str, bflow->fname, media_dev->name);
	if (!dev_name)
		return EFI_UNSUPPORTED;
	ret = calculate_paths(dev_name, devnum_str, bflow->fname, &device,
			      &image);
	if (ret)
		return EFI_UNSUPPORTED;

	if (bflow->flags & BOOTFLOWF_USE_BUILTIN_FDT) {
		log_debug("Booting with built-in fdt\n");
		fdt = EFI_FDT_USE_INTERNAL;
	} else {
		log_debug("Booting with external fdt\n");
		fdt = map_sysmem(bflow->fdt_addr, 0);
	}
	ret = efi_binary_run_dp(bflow->buf, bflow->size, fdt, NULL, 0, device, image);

	return ret;
}
