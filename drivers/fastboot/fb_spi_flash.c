// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Collabora Ltd.
 */

#include <blk.h>
#include <config.h>
#include <env.h>
#include <fastboot.h>
#include <image-sparse.h>
#include <spi.h>
#include <spi_flash.h>
#include <dm.h>
#include <dm/device-internal.h>

static struct spi_flash *flash;

__weak int board_fastboot_spi_flash_write_setup(void)
{
	return 0;
}

__weak int board_fastboot_spi_flash_erase_setup(void)
{
	return 0;
}

static int raw_part_get_info_by_name(const char *name,
				     struct disk_partition *part_info)
{
	/* strlen("fastboot_raw_partition_") + PART_NAME_LEN + 1 */
	char env_desc_name[23 + PART_NAME_LEN + 1];
	char *raw_part_desc;
	const char *argv[2];
	const char **parg = argv;

	/* check for raw partition descriptor */
	strcpy(env_desc_name, "fastboot_raw_partition_");
	strlcat(env_desc_name, name, sizeof(env_desc_name));
	raw_part_desc = strdup(env_get(env_desc_name));
	if (!raw_part_desc)
		return -ENODEV;

	/* parse partition descriptor: <start> <size> */
	for (; parg < argv + sizeof(argv) / sizeof(*argv); ++parg) {
		*parg = strsep(&raw_part_desc, " ");
		if (!*parg) {
			pr_err("Invalid number of arguments.\n");
			return -ENODEV;
		}
	}

	part_info->start = simple_strtoul(argv[0], NULL, 0);
	part_info->size = simple_strtoul(argv[1], NULL, 0);
	strlcpy((char *)part_info->name, name, PART_NAME_LEN);

	return 0;
}

static int fastboot_spi_flash_probe(void)
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	struct udevice *new, *bus_dev;
	int ret;

	/* Remove the old device, otherwise probe will just be a nop */
	ret = spi_find_bus_and_cs(bus, cs, &bus_dev, &new);
	if (!ret)
		device_remove(new, DM_REMOVE_NORMAL);

	spi_flash_probe_bus_cs(bus, cs, &new);
	flash = dev_get_uclass_priv(new);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u (error %d)\n",
		       bus, cs, ret);
		return 1;
	}

	return 0;
}

static int fastboot_spi_flash_unlock(struct spi_flash *flash,
				     struct disk_partition *part_info)
{
	int ret = spi_flash_protect(flash, part_info->start, part_info->size,
				    false);

	if (ret && ret != -EOPNOTSUPP) {
		printf("Failed to unlock SPI flash (%d)\n", ret);
		return ret;
	}

	return 0;
}

static lbaint_t fb_spi_flash_sparse_write(struct sparse_storage *info,
					  lbaint_t blk, lbaint_t blkcnt,
					  const void *buffer)
{
	size_t len = blkcnt * info->blksz;
	u32 offset = blk * info->blksz;
	int ret;

	ret = spi_flash_erase(flash, offset, ROUND(len, flash->erase_size));
	if (ret < 0) {
		printf("Failed to erase sparse chunk (%d)\n", ret);
		return ret;
	}

	ret = spi_flash_write(flash, offset, len, buffer);
	if (ret < 0) {
		printf("Failed to write sparse chunk (%d)\n", ret);
		return ret;
	}

	return blkcnt;
}

static lbaint_t fb_spi_flash_sparse_reserve(struct sparse_storage *info,
					    lbaint_t blk, lbaint_t blkcnt)
{
	return blkcnt;
}

/**
 * fastboot_spi_flash_get_part_info() - Lookup SPI partition by name
 *
 * @part_name: Named device to lookup
 * @part_info: Pointer to returned struct disk_partition
 * @response: Pointer to fastboot response buffer
 * Return: 0 if OK, -ENOENT if no partition name was given, -ENODEV on invalid
 * raw partition descriptor
 */
int fastboot_spi_flash_get_part_info(const char *part_name,
				     struct disk_partition *part_info,
				     char *response)
{
	int ret;

	if (!part_name || !strcmp(part_name, "")) {
		fastboot_fail("partition not given", response);
		return -ENOENT;
	}

	/* TODO: Support partitions on the device */
	ret = raw_part_get_info_by_name(part_name, part_info);
	if (ret < 0)
		fastboot_fail("invalid partition or device", response);

	return ret;
}

/**
 * fastboot_spi_flash_write() - Write image to SPI for fastboot
 *
 * @cmd: Named device to write image to
 * @download_buffer: Pointer to image data
 * @download_bytes: Size of image data
 * @response: Pointer to fastboot response buffer
 */
void fastboot_spi_flash_write(const char *cmd, void *download_buffer,
			      u32 download_bytes, char *response)
{
	struct disk_partition part_info;
	int ret;

	if (fastboot_spi_flash_get_part_info(cmd, &part_info, response))
		return;

	if (fastboot_spi_flash_probe())
		return;

	if (board_fastboot_spi_flash_write_setup())
		return;

	if (fastboot_spi_flash_unlock(flash, &part_info))
		return;

	if (is_sparse_image(download_buffer)) {
		struct sparse_storage sparse;

		sparse.blksz = flash->sector_size;
		sparse.start = part_info.start / sparse.blksz;
		sparse.size = part_info.size / sparse.blksz;
		sparse.write = fb_spi_flash_sparse_write;
		sparse.reserve = fb_spi_flash_sparse_reserve;
		sparse.mssg = fastboot_fail;

		printf("Flashing sparse image at offset " LBAFU "\n",
		       sparse.start);

		ret = write_sparse_image(&sparse, cmd, download_buffer,
					 response);
	} else {
		printf("Flashing raw image at offset " LBAFU "\n",
		       part_info.start);

		ret = spi_flash_erase(flash, part_info.start,
				      ROUND(download_bytes, flash->erase_size));
		if (ret < 0) {
			printf("Failed to erase raw image (%d)\n", ret);
			return;
		}
		ret = spi_flash_write(flash, part_info.start, download_bytes,
				      download_buffer);
		if (ret < 0) {
			printf("Failed to write raw image (%d)\n", ret);
			return;
		}
		printf("........ wrote %u bytes\n", download_bytes);
	}

	if (ret)
		fastboot_fail("error writing the image", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * fastboot_spi_flash_erase() - Erase SPI for fastboot
 *
 * @cmd: Named device to erase
 * @response: Pointer to fastboot response buffer
 */
void fastboot_spi_flash_erase(const char *cmd, char *response)
{
	struct disk_partition part_info;
	int ret;

	if (fastboot_spi_flash_get_part_info(cmd, &part_info, response))
		return;

	if (fastboot_spi_flash_probe())
		return;

	if (board_fastboot_spi_flash_erase_setup())
		return;

	if (fastboot_spi_flash_unlock(flash, &part_info))
		return;

	ret = spi_flash_erase(flash, part_info.start, part_info.size);
	if (ret < 0) {
		pr_err("failed erasing from SPI flash");
		fastboot_fail("failed erasing from SPI flash", response);
		return;
	}

	fastboot_okay(NULL, response);
}
