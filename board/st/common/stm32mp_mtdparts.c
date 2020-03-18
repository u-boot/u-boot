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
#include <tee.h>

#define MTDPARTS_LEN		256
#define MTDIDS_LEN		128

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

/**
 * update the variables "mtdids" and "mtdparts" with boot, tee and user strings
 */
static void board_get_mtdparts(const char *dev,
			       char *mtdids,
			       char *mtdparts,
			       const char *boot,
			       const char *tee,
			       const char *user)
{
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

	if (boot) {
		strncat(mtdparts, boot, MTDPARTS_LEN);
		strncat(mtdparts, ",", MTDPARTS_LEN);
	}

	if (tee) {
		strncat(mtdparts, tee, MTDPARTS_LEN);
		strncat(mtdparts, ",", MTDPARTS_LEN);
	}

	strncat(mtdparts, user, MTDPARTS_LEN);
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct mtd_info *mtd;
	struct udevice *dev;
	static char parts[3 * MTDPARTS_LEN + 1];
	static char ids[MTDIDS_LEN + 1];
	static bool mtd_initialized;
	bool tee = false;

	if (mtd_initialized) {
		*mtdids = ids;
		*mtdparts = parts;
		return;
	}

	if (CONFIG_IS_ENABLED(OPTEE) &&
	    tee_find_device(NULL, NULL, NULL, NULL))
		tee = true;

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
		board_get_mtdparts("nand0", ids, parts,
				   CONFIG_MTDPARTS_NAND0_BOOT,
				   tee ? CONFIG_MTDPARTS_NAND0_TEE : NULL,
				   "-(UBI)");
		put_mtd_device(mtd);
	}

	mtd = get_mtd_device_nm("spi-nand0");
	if (!IS_ERR_OR_NULL(mtd)) {
		board_get_mtdparts("spi-nand0", ids, parts,
				   CONFIG_MTDPARTS_SPINAND0_BOOT,
				   tee ? CONFIG_MTDPARTS_SPINAND0_TEE : NULL,
				   "-(UBI)");
		put_mtd_device(mtd);
	}

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		board_get_mtdparts("nor0", ids, parts,
				   CONFIG_MTDPARTS_NOR0_BOOT,
				   tee ? CONFIG_MTDPARTS_NOR0_TEE : NULL,
				   "-(nor_user)");

	mtd_initialized = true;
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
