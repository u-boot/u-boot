/*
 * Copyright (C) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <environment.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
void fix_envdriver(void)
{
	struct env_driver *drv;
	const int n_ents = ll_entry_count(struct env_driver, env_driver);
	struct env_driver *entry;

	drv = ll_entry_start(struct env_driver, env_driver);
	for (entry = drv; entry != drv + n_ents; entry++) {
		if (entry->name)
			entry->name += gd->reloc_off;
		if (entry->get_char)
			entry->get_char += gd->reloc_off;
		if (entry->load)
			entry->load += gd->reloc_off;
		if (entry->save)
			entry->save += gd->reloc_off;
		if (entry->init)
			entry->init += gd->reloc_off;
	}
}
#endif

static struct env_driver *env_driver_lookup(enum env_location loc)
{
	struct env_driver *drv;
	const int n_ents = ll_entry_count(struct env_driver, env_driver);
	struct env_driver *entry;

	drv = ll_entry_start(struct env_driver, env_driver);
	for (entry = drv; entry != drv + n_ents; entry++) {
		if (loc == entry->location)
			return entry;
	}

	/* Not found */
	return NULL;
}

static enum env_location env_get_default_location(void)
{
	if IS_ENABLED(CONFIG_ENV_IS_IN_EEPROM)
		return ENVL_EEPROM;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_FAT)
		return ENVL_FAT;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_FLASH)
		return ENVL_FLASH;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
		return ENVL_MMC;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_NAND)
		return ENVL_NAND;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_NVRAM)
		return ENVL_NVRAM;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_REMOTE)
		return ENVL_REMOTE;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH)
		return ENVL_SPI_FLASH;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_UBI)
		return ENVL_UBI;
	else if IS_ENABLED(CONFIG_ENV_IS_NOWHERE)
		return ENVL_NOWHERE;
	else
		return ENVL_UNKNOWN;
}

struct env_driver *env_driver_lookup_default(void)
{
	enum env_location loc = env_get_default_location();
	struct env_driver *drv;

	drv = env_driver_lookup(loc);
	if (!drv) {
		debug("%s: No environment driver for location %d\n", __func__,
		      loc);
		return NULL;
	}

	return drv;
}

int env_get_char(int index)
{
	struct env_driver *drv = env_driver_lookup_default();
	int ret;

	if (gd->env_valid == ENV_INVALID)
		return default_environment[index];
	if (!drv)
		return -ENODEV;
	if (!drv->get_char)
		return *(uchar *)(gd->env_addr + index);
	ret = drv->get_char(index);
	if (ret < 0) {
		debug("%s: Environment failed to load (err=%d)\n",
		      __func__, ret);
	}

	return ret;
}

int env_load(void)
{
	struct env_driver *drv = env_driver_lookup_default();
	int ret = 0;

	if (!drv)
		return -ENODEV;
	if (!drv->load)
		return 0;
	ret = drv->load();
	if (ret) {
		debug("%s: Environment failed to load (err=%d)\n", __func__,
		      ret);
		return ret;
	}

	return 0;
}

int env_save(void)
{
	struct env_driver *drv = env_driver_lookup_default();
	int ret;

	if (!drv)
		return -ENODEV;
	if (!drv->save)
		return -ENOSYS;
	ret = drv->save();
	if (ret) {
		debug("%s: Environment failed to save (err=%d)\n", __func__,
		      ret);
		return ret;
	}

	return 0;
}

int env_init(void)
{
	struct env_driver *drv = env_driver_lookup_default();
	int ret = -ENOENT;

	if (!drv)
		return -ENODEV;
	if (drv->init)
		ret = drv->init();
	if (ret == -ENOENT) {
		gd->env_addr = (ulong)&default_environment[0];
		gd->env_valid = ENV_VALID;

		return 0;
	} else if (ret) {
		debug("%s: Environment failed to init (err=%d)\n", __func__,
		      ret);
		return ret;
	}

	return 0;
}
