// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#include <bmp_layout.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <fs.h>
#include <fdt_support.h>
#include <image.h>
#include <log.h>
#include <nand.h>
#include <sata.h>
#include <spi.h>
#include <spi_flash.h>
#include <splash.h>
#include <usb.h>
#include <virtio.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
static struct spi_flash *sf;
static int splash_sf_read_raw(u32 bmp_load_addr, int offset, size_t read_size)
{
	if (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		if (!sf)
			return -ENODEV;
	}

	return spi_flash_read(sf, offset, read_size, (void *)(uintptr_t)bmp_load_addr);
}
#else
static int splash_sf_read_raw(u32 bmp_load_addr, int offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	return -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
static int splash_nand_read_raw(u32 bmp_load_addr, int offset, size_t read_size)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	return nand_read_skip_bad(mtd, offset,
				  &read_size, NULL,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#else
static int splash_nand_read_raw(u32 bmp_load_addr, int offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	return -ENOSYS;
}
#endif

static int splash_mmc_read_raw(u32 bmp_load_addr, struct splash_location *location,
			       size_t read_size)
{
	struct disk_partition partition;
	struct blk_desc *desc;
	lbaint_t blkcnt;
	int ret, n;

	if (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		return -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_num("mmc", location->devpart, &desc,
						   &partition, 1);
	if (ret < 0)
		return ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (void *)(uintptr_t)bmp_load_addr);

	return (n == blkcnt) ? 0 : -EIO;
}

static int splash_storage_read_raw(struct splash_location *location,
			       u32 bmp_load_addr, size_t read_size)
{
	u32 offset;

	if (!location)
		return -EINVAL;

	offset = location->offset;
	switch (location->storage) {
	case SPLASH_STORAGE_MMC:
		return splash_mmc_read_raw(bmp_load_addr, location, read_size);
	case SPLASH_STORAGE_NAND:
		return splash_nand_read_raw(bmp_load_addr, offset, read_size);
	case SPLASH_STORAGE_SF:
		return splash_sf_read_raw(bmp_load_addr, offset, read_size);
	default:
		printf("Unknown splash location\n");
	}

	return -EINVAL;
}

static int splash_load_raw(struct splash_location *location, u32 bmp_load_addr)
{
	struct bmp_header *bmp_hdr;
	int res;
	size_t bmp_size, bmp_header_size = sizeof(struct bmp_header);

	if (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	if (res < 0)
		return res;

	bmp_hdr = (struct bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	if (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	return splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	return -EFAULT;
}

static int splash_select_fs_dev(struct splash_location *location)
{
	int res;

	switch (location->storage) {
	case SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_NAND:
		if (location->ubivol != NULL)
			res = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
		else
			res = -ENODEV;
		break;
	default:
		printf("Error: unsupported location storage.\n");
		return -ENODEV;
	}

	if (res)
		printf("Error: could not access storage.\n");

	return res;
}

#ifdef CONFIG_USB_STORAGE
static int splash_init_usb(void)
{
	int err;

	err = usb_init();
	if (err)
		return err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	return err;
}
#else
static inline int splash_init_usb(void)
{
	printf("Cannot load splash image: no USB support\n");
	return -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
static int splash_init_sata(void)
{
	return sata_probe(0);
}
#else
static inline int splash_init_sata(void)
{
	printf("Cannot load splash image: no SATA support\n");
	return -ENOSYS;
}
#endif

static int splash_init_virtio(void)
{
	if (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		return -ENOSYS;
	} else {
		return virtio_init();
	}
}

#if defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
static int splash_mount_ubifs(struct splash_location *location)
{
	int res;
	char cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	if (res)
		return res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	return res;
}

static inline int splash_umount_ubifs(void)
{
	return run_command("ubifsumount", 0);
}
#else
static inline int splash_mount_ubifs(struct splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	return -ENOSYS;
}

static inline int splash_umount_ubifs(void)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	return -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

static int splash_load_fs(struct splash_location *location, u32 bmp_load_addr)
{
	int res = 0;
	loff_t bmp_size;
	loff_t actread;
	char *splash_file;

	splash_file = env_get("splashfile");
	if (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	if (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	if (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	if (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	if (location->ubivol != NULL)
		res = splash_mount_ubifs(location);

	if (res)
		return res;

	res = splash_select_fs_dev(location);
	if (res)
		goto out;

	res = fs_size(splash_file, &bmp_size);
	if (res) {
		printf("Error (%d): cannot determine file size\n", res);
		goto out;
	}

	if (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		goto out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	if (location->ubivol != NULL)
		splash_umount_ubifs();

	return res;
}

/**
 * select_splash_location - return the splash location based on board support
 *			    and env variable "splashsource".
 *
 * @locations:		An array of supported splash locations.
 * @size:		Size of splash_locations array.
 *
 * @return: If a null set of splash locations is given, or
 *	    splashsource env variable is set to unsupported value
 *			return NULL.
 *	    If splashsource env variable is not defined
 *			return the first entry in splash_locations as default.
 *	    If splashsource env variable contains a supported value
 *			return the location selected by splashsource.
 */
static struct splash_location *select_splash_location(
			    struct splash_location *locations, uint size)
{
	int i;
	char *env_splashsource;

	if (!locations || size == 0)
		return NULL;

	env_splashsource = env_get("splashsource");
	if (env_splashsource == NULL)
		return &locations[0];

	for (i = 0; i < size; i++) {
		if (!strcmp(locations[i].name, env_splashsource))
			return &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	return NULL;
}

#ifdef CONFIG_FIT
static int splash_load_fit(struct splash_location *location, u32 bmp_load_addr)
{
	int res;
	int node_offset;
	const char *splash_file;
	const void *internal_splash_data;
	size_t internal_splash_size;
	int external_splash_addr;
	int external_splash_size;
	bool is_splash_external = false;
	struct legacy_img_hdr *img_header;
	const u32 *fit_header;
	u32 fit_size;
	const size_t header_size = sizeof(struct legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	if (res < 0)
		return res;

	img_header = (struct legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	if (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		return -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (const u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	if (res < 0)
		return res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	if (res) {
		debug("Could not find valid FIT image\n");
		return res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	if (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	node_offset = fit_image_get_node(fit_header, splash_file);
	if (node_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		return -ENOENT;
	}

	/* Extract the splash data from FIT */
	if (!fit_image_get_data(fit_header, node_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((void *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} else {
		printf("Failed to get splash image from FIT\n");
		return -ENODATA;
	}

	if (is_splash_external) {
		res = fit_image_get_data_size(fit_header, node_offset, &external_splash_size);
		if (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			return res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		if (res < 0)
			return res;
	}

	return 0;
}
#endif /* CONFIG_FIT */

/**
 * splash_source_load - load splash image from a supported location.
 *
 * Select a splash image location based on the value of splashsource environment
 * variable and the board supported splash source locations, and load a
 * splashimage to the address pointed to by splashimage environment variable.
 *
 * @locations:		An array of supported splash locations.
 * @size:		Size of splash_locations array.
 *
 * @return: 0 on success, negative value on failure.
 */
int splash_source_load(struct splash_location *locations, uint size)
{
	struct splash_location *splash_location;
	char *env_splashimage_value;
	char *devpart;
	u32 bmp_load_addr;

	env_splashimage_value = env_get("splashimage");
	if (env_splashimage_value == NULL)
		return -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_value, 0);
	if (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		return -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	if (!splash_location)
		return -EINVAL;

	devpart = env_get("splashdevpart");
	if (devpart)
		splash_location->devpart = devpart;

	if (splash_location->flags == SPLASH_STORAGE_RAW)
		return splash_load_raw(splash_location, bmp_load_addr);
	else if (splash_location->flags == SPLASH_STORAGE_FS)
		return splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	else if (splash_location->flags == SPLASH_STORAGE_FIT)
		return splash_load_fit(splash_location, bmp_load_addr);
#endif
	return -EINVAL;
}
