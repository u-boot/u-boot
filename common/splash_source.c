/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nand.h>
#include <errno.h>
#include <splash.h>
#include <spi_flash.h>
#include <spi.h>
#include <usb.h>
#include <sata.h>
#include <bmp_layout.h>
#include <fs.h>

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

	return spi_flash_read(sf, offset, read_size, (void *)bmp_load_addr);
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
	return nand_read_skip_bad(&nand_info[nand_curr_device], offset,
				  &read_size, NULL,
				  nand_info[nand_curr_device].size,
				  (u_char *)bmp_load_addr);
}
#else
static int splash_nand_read_raw(u32 bmp_load_addr, int offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	return -ENOSYS;
}
#endif

static int splash_storage_read_raw(struct splash_location *location,
			       u32 bmp_load_addr, size_t read_size)
{
	u32 offset;

	if (!location)
		return -EINVAL;

	offset = location->offset;
	switch (location->storage) {
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

	bmp_hdr = (struct bmp_header *)bmp_load_addr;
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

	return usb_stor_scan(1) < 0 ? -ENODEV : 0;
}
#else
static inline int splash_init_usb(void)
{
	printf("Cannot load splash image: no USB support\n");
	return -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_SATA
static int splash_init_sata(void)
{
	return sata_initialize();
}
#else
static inline int splash_init_sata(void)
{
	printf("Cannot load splash image: no SATA support\n");
	return -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

static int splash_load_fs(struct splash_location *location, u32 bmp_load_addr)
{
	int res = 0;
	loff_t bmp_size;
	char *splash_file;

	splash_file = getenv("splashfile");
	if (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	if (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	if (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	if (res)
		return res;

	res = splash_select_fs_dev(location);
	if (res)
		return res;

	res = fs_size(splash_file, &bmp_size);
	if (res) {
		printf("Error (%d): cannot determine file size\n", res);
		return res;
	}

	if (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		return -EFAULT;
	}

	splash_select_fs_dev(location);
	return fs_read(splash_file, bmp_load_addr, 0, 0, NULL);
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

	env_splashsource = getenv("splashsource");
	if (env_splashsource == NULL)
		return &locations[0];

	for (i = 0; i < size; i++) {
		if (!strcmp(locations[i].name, env_splashsource))
			return &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	return NULL;
}

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
	u32 bmp_load_addr;

	env_splashimage_value = getenv("splashimage");
	if (env_splashimage_value == NULL)
		return -ENOENT;

	bmp_load_addr = simple_strtoul(env_splashimage_value, 0, 16);
	if (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		return -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	if (!splash_location)
		return -EINVAL;

	if (splash_location->flags & SPLASH_STORAGE_RAW)
		return splash_load_raw(splash_location, bmp_load_addr);
	else if (splash_location->flags & SPLASH_STORAGE_FS)
		return splash_load_fs(splash_location, bmp_load_addr);

	return -EINVAL;
}
