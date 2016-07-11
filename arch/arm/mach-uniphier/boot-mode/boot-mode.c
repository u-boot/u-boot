/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <spl.h>
#include <linux/err.h>

#include "../sbc/sbc-regs.h"
#include "../soc-info.h"
#include "boot-device.h"

u32 spl_boot_device_raw(void)
{
	if (boot_is_swapped())
		return BOOT_DEVICE_NOR;

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case SOC_UNIPHIER_SLD3:
		return uniphier_sld3_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || defined(CONFIG_ARCH_UNIPHIER_PRO4) || \
	defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case SOC_UNIPHIER_LD4:
	case SOC_UNIPHIER_PRO4:
	case SOC_UNIPHIER_SLD8:
		return uniphier_ld4_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case SOC_UNIPHIER_PRO5:
		return uniphier_pro5_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case SOC_UNIPHIER_PXS2:
	case SOC_UNIPHIER_LD6B:
		return uniphier_pxs2_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	case SOC_UNIPHIER_LD20:
		return uniphier_ld20_boot_device();
#endif
	default:
		return BOOT_DEVICE_NONE;
	}
}

u32 spl_boot_device(void)
{
	u32 ret;

	ret = spl_boot_device_raw();

	return ret == BOOT_DEVICE_USB ? BOOT_DEVICE_NOR : ret;
}

u32 spl_boot_mode(void)
{
	struct mmc *mmc;

	/*
	 * work around a bug in the Boot ROM of PH1-sLD3, LD4, Pro4, and sLD8:
	 *
	 * The boot ROM in these SoCs breaks the PARTITION_CONFIG [179] of
	 * Extended CSD register; when switching to the Boot Partition 1, the
	 * Boot ROM should issue the SWITCH command (CMD6) with Set Bits for
	 * the Access Bits, but in fact it uses Write Byte for the Access Bits.
	 * As a result, the BOOT_PARTITION_ENABLE field of the PARTITION_CONFIG
	 * is lost.  This bug was fixed for PH1-Pro5 and later SoCs.
	 *
	 * Fixup mmc->part_config here because it is used to determine the
	 * partition which the U-Boot image is read from.
	 */
	mmc = find_mmc_device(0);
	mmc->part_config &= ~EXT_CSD_BOOT_PART_NUM(PART_ACCESS_MASK);
	mmc->part_config |= EXT_CSD_BOOT_PARTITION_ENABLE;

	return MMCSD_MODE_EMMCBOOT;
}

#if defined(CONFIG_DM_MMC) && !defined(CONFIG_SPL_BUILD)
static int find_first_mmc_device(void)
{
	struct mmc *mmc;
	int i;

	for (i = 0; (mmc = find_mmc_device(i)); i++) {
		if (!mmc_init(mmc) && IS_MMC(mmc))
			return i;
	}

	return -ENODEV;
}

int mmc_get_env_dev(void)
{
	return find_first_mmc_device();
}

static int do_mmcsetn(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev;

	dev = find_first_mmc_device();
	if (dev < 0)
		return CMD_RET_FAILURE;

	setenv_ulong("mmc_first_dev", dev);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	   mmcsetn,	1,	1,	do_mmcsetn,
	"Set the first MMC (not SD) dev number to \"mmc_first_dev\" environment",
	""
);
#endif
