// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <mtd.h>
#include <mtd_node.h>

#define MTDPARTS_LEN		256
#define MTDIDS_LEN		128

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

/**
 * The mtdparts_nand0 and mtdparts_nor0 variable tends to be long.
 * If we need to access it before the env is relocated, then we need
 * to use our own stack buffer. gd->env_buf will be too small.
 *
 * @param buf temporary buffer pointer MTDPARTS_LEN long
 * @return mtdparts variable string, NULL if not found
 */
static const char *env_get_mtdparts(const char *str, char *buf)
{
	if (gd->flags & GD_FLG_ENV_READY)
		return env_get(str);
	if (env_get_f(str, buf, MTDPARTS_LEN) != -1)
		return buf;

	return NULL;
}

/**
 * update the variables "mtdids" and "mtdparts" with content of mtdparts_<dev>
 */
static void board_get_mtdparts(const char *dev,
			       char *mtdids,
			       char *mtdparts)
{
	char env_name[32] = "mtdparts_";
	char tmp_mtdparts[MTDPARTS_LEN];
	const char *tmp;

	/* name of env variable to read = mtdparts_<dev> */
	strcat(env_name, dev);
	tmp = env_get_mtdparts(env_name, tmp_mtdparts);
	if (tmp) {
		/* mtdids: "<dev>=<dev>, ...." */
		if (mtdids[0] != '\0')
			strcat(mtdids, ",");
		strcat(mtdids, dev);
		strcat(mtdids, "=");
		strcat(mtdids, dev);

		/* mtdparts: "mtdparts=<dev>:<mtdparts_<dev>>;..." */
		if (mtdparts[0] != '\0')
			strncat(mtdparts, ";", MTDPARTS_LEN);
		else
			strcat(mtdparts, "mtdparts=");
		strncat(mtdparts, dev, MTDPARTS_LEN);
		strncat(mtdparts, ":", MTDPARTS_LEN);
		strncat(mtdparts, tmp, MTDPARTS_LEN);
	}
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct mtd_info *mtd;
	struct udevice *dev;
	static char parts[3 * MTDPARTS_LEN + 1];
	static char ids[MTDIDS_LEN + 1];
	static bool mtd_initialized;

	if (mtd_initialized) {
		*mtdids = ids;
		*mtdparts = parts;
		return;
	}

	memset(parts, 0, sizeof(parts));
	memset(ids, 0, sizeof(ids));

	/* probe all MTD devices */
	for (uclass_first_device(UCLASS_MTD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		pr_debug("mtd device = %s\n", dev->name);
	}

	mtd = get_mtd_device_nm("nand0");
	if (!IS_ERR_OR_NULL(mtd)) {
		board_get_mtdparts("nand0", ids, parts);
		put_mtd_device(mtd);
	}

	mtd = get_mtd_device_nm("spi-nand0");
	if (!IS_ERR_OR_NULL(mtd)) {
		board_get_mtdparts("spi-nand0", ids, parts);
		put_mtd_device(mtd);
	}

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		board_get_mtdparts("nor0", ids, parts);

	mtd_initialized = true;
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
