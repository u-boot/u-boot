/*
 * Copyright (C) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <environment.h>

DECLARE_GLOBAL_DATA_PTR;

static struct env_driver *_env_driver_lookup(enum env_location loc)
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

/**
 * env_get_location() - Returns the best env location for a board
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will return the preferred environment for the given priority.
 *
 * All implementations are free to use the operation, the priority and
 * any other data relevant to their choice, but must take into account
 * the fact that the lowest prority (0) is the most important location
 * in the system. The following locations should be returned by order
 * of descending priorities, from the highest to the lowest priority.
 *
 * Returns:
 * an enum env_location value on success, a negative error code otherwise
 */
static enum env_location env_get_location(enum env_operation op, int prio)
{
	/*
	 * We support a single environment, so any environment asked
	 * with a priority that is not zero is out of our supported
	 * bounds.
	 */
	if (prio >= 1)
		return ENVL_UNKNOWN;

	if IS_ENABLED(CONFIG_ENV_IS_IN_EEPROM)
		return ENVL_EEPROM;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_FAT)
		return ENVL_FAT;
	else if IS_ENABLED(CONFIG_ENV_IS_IN_EXT4)
		return ENVL_EXT4;
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


/**
 * env_driver_lookup() - Finds the most suited environment location
 * @op: operations performed on the environment
 * @prio: priority between the multiple environments, 0 being the
 *        highest priority
 *
 * This will try to find the available environment with the highest
 * priority in the system.
 *
 * Returns:
 * NULL on error, a pointer to a struct env_driver otherwise
 */
static struct env_driver *env_driver_lookup(enum env_operation op, int prio)
{
	enum env_location loc = env_get_location(op, prio);
	struct env_driver *drv;

	if (loc == ENVL_UNKNOWN)
		return NULL;

	drv = _env_driver_lookup(loc);
	if (!drv) {
		debug("%s: No environment driver for location %d\n", __func__,
		      loc);
		return NULL;
	}

	return drv;
}

int env_get_char(int index)
{
	struct env_driver *drv;
	int prio;

	if (gd->env_valid == ENV_INVALID)
		return default_environment[index];

	for (prio = 0; (drv = env_driver_lookup(ENVOP_GET_CHAR, prio)); prio++) {
		int ret;

		if (!drv->get_char)
			continue;

		ret = drv->get_char(index);
		if (!ret)
			return 0;

		debug("%s: Environment %s failed to load (err=%d)\n", __func__,
		      drv->name, ret);
	}

	return -ENODEV;
}

int env_load(void)
{
	struct env_driver *drv;
	int prio;

	for (prio = 0; (drv = env_driver_lookup(ENVOP_LOAD, prio)); prio++) {
		int ret;

		if (!drv->load)
			continue;

		ret = drv->load();
		if (!ret)
			return 0;

		debug("%s: Environment %s failed to load (err=%d)\n", __func__,
		      drv->name, ret);
	}

	return -ENODEV;
}

int env_save(void)
{
	struct env_driver *drv;
	int prio;

	for (prio = 0; (drv = env_driver_lookup(ENVOP_SAVE, prio)); prio++) {
		int ret;

		if (!drv->save)
			continue;

		printf("Saving Environment to %s... ", drv->name);
		ret = drv->save();
		printf("%s\n", ret ? "Failed" : "OK");
		if (!ret)
			return 0;

		debug("%s: Environment %s failed to save (err=%d)\n", __func__,
		      drv->name, ret);
	}

	return -ENODEV;
}

int env_init(void)
{
	struct env_driver *drv;
	int ret = -ENOENT;
	int prio;

	for (prio = 0; (drv = env_driver_lookup(ENVOP_INIT, prio)); prio++) {
		if (!drv->init)
			continue;

		ret = drv->init();
		if (!ret)
			return 0;

		debug("%s: Environment %s failed to init (err=%d)\n", __func__,
		      drv->name, ret);
	}

	if (!prio)
		return -ENODEV;

	if (ret == -ENOENT) {
		gd->env_addr = (ulong)&default_environment[0];
		gd->env_valid = ENV_VALID;

		return 0;
	}

	return ret;
}
