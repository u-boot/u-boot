// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dfu.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <mtd.h>
#include <mtd_node.h>
#include <tee.h>
#include <asm/arch/stm32prog.h>
#include <asm/arch/sys_proto.h>

#define MTDPARTS_LEN		256
#define MTDIDS_LEN		128

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

/**
 * update the variables "mtdids" and "mtdparts" with boot, tee and user strings
 */
static void board_set_mtdparts(const char *dev,
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
	bool tee, nor, nand, spinand, serial;

	if (mtd_initialized) {
		*mtdids = ids;
		*mtdparts = parts;
		return;
	}

	tee = false;
	nor = false;
	nand = false;
	spinand = false;
	serial = false;

	switch (get_bootmode() & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_SERIAL_UART:
	case BOOT_SERIAL_USB:
		serial = true;
		if (CONFIG_IS_ENABLED(CMD_STM32PROG)) {
			tee = stm32prog_get_tee_partitions();
			nor = stm32prog_get_fsbl_nor();
		}
		nand = true;
		spinand = true;
		break;
	case BOOT_FLASH_NAND:
		nand = true;
		break;
	case BOOT_FLASH_SPINAND:
		spinand = true;
		break;
	case BOOT_FLASH_NOR:
		nor = true;
		break;
	default:
		break;
	}

	if (!serial && CONFIG_IS_ENABLED(OPTEE) &&
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

	if (nor || nand) {
		mtd = get_mtd_device_nm("nand0");
		if (!IS_ERR_OR_NULL(mtd)) {
			const char *mtd_boot = CONFIG_MTDPARTS_NAND0_BOOT;
			const char *mtd_tee = CONFIG_MTDPARTS_NAND0_TEE;

			board_set_mtdparts("nand0", ids, parts,
					   !nor ? mtd_boot : NULL,
					   !nor && tee ? mtd_tee : NULL,
					   "-(UBI)");
			put_mtd_device(mtd);
		}
	}

	if (nor || spinand) {
		mtd = get_mtd_device_nm("spi-nand0");
		if (!IS_ERR_OR_NULL(mtd)) {
			const char *mtd_boot = CONFIG_MTDPARTS_SPINAND0_BOOT;
			const char *mtd_tee = CONFIG_MTDPARTS_SPINAND0_TEE;

			board_set_mtdparts("spi-nand0", ids, parts,
					   !nor ? mtd_boot : NULL,
					   !nor && tee ? mtd_tee : NULL,
					   "-(UBI)");
			put_mtd_device(mtd);
		}
	}

	if (nor) {
		if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev)) {
			const char *mtd_boot = CONFIG_MTDPARTS_NOR0_BOOT;
			const char *mtd_tee = CONFIG_MTDPARTS_NOR0_TEE;

			board_set_mtdparts("nor0", ids, parts,
					   mtd_boot,
					   tee ? mtd_tee : NULL,
					   "-(nor_user)");
		}
	}

	mtd_initialized = true;
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
