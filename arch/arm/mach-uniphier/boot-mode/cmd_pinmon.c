/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include "../sbc/sbc-regs.h"
#include "../soc-info.h"
#include "boot-device.h"

static int do_pinmon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("Boot Swap: %s\n\n", boot_is_swapped() ? "ON" : "OFF");

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case SOC_UNIPHIER_SLD3:
		uniphier_sld3_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || defined(CONFIG_ARCH_UNIPHIER_PRO4) || \
	defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case SOC_UNIPHIER_LD4:
	case SOC_UNIPHIER_PRO4:
	case SOC_UNIPHIER_SLD8:
		uniphier_ld4_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case SOC_UNIPHIER_PRO5:
		uniphier_pro5_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case SOC_UNIPHIER_PXS2:
	case SOC_UNIPHIER_LD6B:
		uniphier_pxs2_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	case SOC_UNIPHIER_LD20:
		uniphier_ld20_boot_mode_show();
		break;
#endif
	default:
		break;
	}

	return 0;
}

U_BOOT_CMD(
	pinmon,	1,	1,	do_pinmon,
	"pin monitor",
	""
);
