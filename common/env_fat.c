/*
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <errno.h>
#include <fat.h>
#include <mmc.h>

char *env_name_spec = "FAT";

env_t *env_ptr;

DECLARE_GLOBAL_DATA_PTR;

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
int saveenv(void)
{
	env_t	env_new;
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;
	loff_t size;

	err = env_export(&env_new);
	if (err)
		return err;

	part = get_device_and_partition(FAT_ENV_INTERFACE,
					FAT_ENV_DEVICE_AND_PART,
					&dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->dev;
	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		printf("\n** Unable to use %s %d:%d for saveenv **\n",
		       FAT_ENV_INTERFACE, dev, part);
		return 1;
	}

	err = file_fat_write(FAT_ENV_FILE, (void *)&env_new, 0, sizeof(env_t),
			     &size);
	if (err == -1) {
		printf("\n** Unable to write \"%s\" from %s%d:%d **\n",
			FAT_ENV_FILE, FAT_ENV_INTERFACE, dev, part);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

void env_relocate_spec(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;

	part = get_device_and_partition(FAT_ENV_INTERFACE,
					FAT_ENV_DEVICE_AND_PART,
					&dev_desc, &info, 1);
	if (part < 0)
		goto err_env_relocate;

	dev = dev_desc->dev;
	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		printf("\n** Unable to use %s %d:%d for loading the env **\n",
		       FAT_ENV_INTERFACE, dev, part);
		goto err_env_relocate;
	}

	err = file_fat_read(FAT_ENV_FILE, buf, CONFIG_ENV_SIZE);
	if (err == -1) {
		printf("\n** Unable to read \"%s\" from %s%d:%d **\n",
			FAT_ENV_FILE, FAT_ENV_INTERFACE, dev, part);
		goto err_env_relocate;
	}

	env_import(buf, 1);
	return;

err_env_relocate:
	set_default_env(NULL);
}
