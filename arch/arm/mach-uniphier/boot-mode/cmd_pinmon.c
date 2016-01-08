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
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD3)
	case SOC_UNIPHIER_PH1_SLD3:
		ph1_sld3_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_PRO4) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
	case SOC_UNIPHIER_PH1_LD4:
	case SOC_UNIPHIER_PH1_PRO4:
	case SOC_UNIPHIER_PH1_SLD8:
		ph1_ld4_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO5)
	case SOC_UNIPHIER_PH1_PRO5:
		ph1_pro5_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PROXSTREAM2) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_LD6B)
	case SOC_UNIPHIER_PROXSTREAM2:
	case SOC_UNIPHIER_PH1_LD6B:
		proxstream2_boot_mode_show();
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
