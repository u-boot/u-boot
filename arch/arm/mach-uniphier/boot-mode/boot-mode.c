/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <spl.h>
#include <linux/errno.h>

#include "../sbc/sbc-regs.h"
#include "../soc-info.h"
#include "boot-device.h"

u32 spl_boot_device_raw(void)
{
	if (boot_is_swapped())
		return BOOT_DEVICE_NOR;

	switch (uniphier_get_soc_id()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case UNIPHIER_SLD3_ID:
		return uniphier_sld3_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || defined(CONFIG_ARCH_UNIPHIER_PRO4) || \
	defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case UNIPHIER_LD4_ID:
	case UNIPHIER_PRO4_ID:
	case UNIPHIER_SLD8_ID:
		return uniphier_ld4_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case UNIPHIER_PRO5_ID:
		return uniphier_pro5_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case UNIPHIER_PXS2_ID:
	case UNIPHIER_LD6B_ID:
		return uniphier_pxs2_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11) || defined(CONFIG_ARCH_UNIPHIER_LD20)
	case UNIPHIER_LD11_ID:
	case UNIPHIER_LD20_ID:
		return uniphier_ld20_boot_device();
#endif
	default:
		return BOOT_DEVICE_NONE;
	}
}

u32 spl_boot_device(void)
{
	u32 mode;

	mode = spl_boot_device_raw();

	switch (uniphier_get_soc_id()) {
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case UNIPHIER_PXS2_ID:
	case UNIPHIER_LD6B_ID:
		if (mode == BOOT_DEVICE_USB)
			mode = BOOT_DEVICE_NOR;
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11) || defined(CONFIG_ARCH_UNIPHIER_LD20)
	case UNIPHIER_LD11_ID:
	case UNIPHIER_LD20_ID:
		if (mode == BOOT_DEVICE_MMC1 || mode == BOOT_DEVICE_USB)
			mode = BOOT_DEVICE_BOARD;
		break;
#endif
	default:
		break;
	}

	return mode;
}
