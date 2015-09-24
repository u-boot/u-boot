/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>
#include <mach/boot-device.h>
#include <mach/sbc-regs.h>
#include <mach/soc_info.h>

u32 spl_boot_device(void)
{
	if (boot_is_swapped())
		return BOOT_DEVICE_NOR;

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD3)
	case SOC_UNIPHIER_PH1_SLD3:
		return ph1_sld3_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_PRO4) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
	case SOC_UNIPHIER_PH1_LD4:
	case SOC_UNIPHIER_PH1_PRO4:
	case SOC_UNIPHIER_PH1_SLD8:
		return ph1_ld4_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO5)
	case SOC_UNIPHIER_PH1_PRO5:
		return ph1_pro5_boot_device();
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PROXSTREAM2) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_LD6B)
	case SOC_UNIPHIER_PROXSTREAM2:
	case SOC_UNIPHIER_PH1_LD6B:
		return proxstream2_boot_device();
#endif
	default:
		return BOOT_DEVICE_NONE;
	}
}
