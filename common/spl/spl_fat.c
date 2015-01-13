/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <fat.h>
#include <image.h>

static int fat_registered;

#ifdef CONFIG_SPL_FAT_SUPPORT
static int spl_register_fat_device(block_dev_desc_t *block_dev, int partition)
{
	int err = 0;

	if (fat_registered)
		return err;

	err = fat_register_device(block_dev, partition);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: fat register err - %d\n", __func__, err);
#endif
		return err;
	}

	fat_registered = 1;

	return err;
}

int spl_load_image_fat(block_dev_desc_t *block_dev,
						int partition,
						const char *filename)
{
	int err;
	struct image_header *header;

	err = spl_register_fat_device(block_dev, partition);
	if (err)
		goto end;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	err = file_fat_read(filename, header, sizeof(struct image_header));
	if (err <= 0)
		goto end;

	spl_parse_image_header(header);

	err = file_fat_read(filename, (u8 *)spl_image.load_addr, 0);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err <= 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
#endif

	return (err <= 0);
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_load_image_fat_os(block_dev_desc_t *block_dev, int partition)
{
	int err;
	__maybe_unused char *file;

	err = spl_register_fat_device(block_dev, partition);
	if (err)
		return err;

#if defined(CONFIG_SPL_ENV_SUPPORT) && defined(CONFIG_SPL_OS_BOOT)
	file = getenv("falcon_args_file");
	if (file) {
		err = file_fat_read(file, (void *)CONFIG_SYS_SPL_ARGS_ADDR, 0);
		if (err <= 0) {
			printf("spl: error reading image %s, err - %d, falling back to default\n",
			       file, err);
			goto defaults;
		}
		file = getenv("falcon_image_file");
		if (file) {
			err = spl_load_image_fat(block_dev, partition, file);
			if (err != 0) {
				puts("spl: falling back to default\n");
				goto defaults;
			}

			return 0;
		} else
			puts("spl: falcon_image_file not set in environment, falling back to default\n");
	} else
		puts("spl: falcon_args_file not set in environment, falling back to default\n");

defaults:
#endif

	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SYS_SPL_ARGS_ADDR, 0);
	if (err <= 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
#endif
		return -1;
	}

	return spl_load_image_fat(block_dev, partition,
			CONFIG_SPL_FS_LOAD_KERNEL_NAME);
}
#endif
#endif
