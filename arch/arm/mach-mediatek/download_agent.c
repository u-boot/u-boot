// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 */

#include <dm/uclass.h>
#include <fastboot.h>
#include <init.h>
#include <mmc.h>
#include <spl.h>
#include <stdio.h>
#include <linux/errno.h>

static int mtk_mmc_boot_prepare(void)
{
	struct udevice *dev;
	struct mmc *mmc;
	int ret;

	/* No storage loader has run yet, so probe the device here. */
	ret = uclass_get_device_by_seq(UCLASS_MMC,
				       CONFIG_VAL(FASTBOOT_FLASH_MMC_DEV), &dev);
	if (ret) {
		printf("Cannot load MMC device: %d\n", ret);
		return ret;
	}

	mmc = mmc_get_mmc_dev(dev);
	if (!mmc)
		return -ENODEV;

	ret = mmc_init(mmc);
	if (ret) {
		printf("MMC init failed: %d\n", ret);
		return ret;
	}

	/* Enable BOOT1 and boot acknowledgment for normal BootROM startup. */
	ret = mmc_set_part_conf(mmc, 1, 1, 0);
	if (ret)
		printf("Cannot configure the eMMC boot partition: %d\n", ret);

	/* Reset to x1 SDR, backward-compatible timing for each boot. */
	ret = mmc_set_boot_bus_width(mmc, 0, 0, 0);
	if (ret)
		printf("Cannot configure the eMMC boot bus: %d\n", ret);

	/* This permanently enables the reset pin; it cannot be undone. */
	if (!mmc->ext_csd[EXT_CSD_RST_N_FUNCTION]) {
		ret = mmc_set_rst_n_function(mmc, 1);
		if (ret)
			printf("Cannot enable the eMMC reset function: %d\n", ret);
	}

	return 0;
}

#ifdef CONFIG_XPL_BUILD
void spl_board_init(void)
{
	int ret;

	ret = mtk_mmc_boot_prepare();
	if (ret)
		printf("MMC boot preparation failed: %d\n", ret);

	puts("Waiting for fastboot commands...\n");
	ret = fastboot_usb_run(0, NULL, 0);
	if (ret)
		printf("Fastboot failed: %d\n", ret);
}
#else
int board_late_init(void)
{
	int ret;

	ret = mtk_mmc_boot_prepare();
	if (ret)
		printf("MMC boot preparation failed: %d\n", ret);

	/* Keep fastboot available even if storage preparation failed. */
	return 0;
}
#endif
