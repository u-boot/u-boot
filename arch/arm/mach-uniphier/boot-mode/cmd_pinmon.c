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

	switch (uniphier_get_soc_id()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case UNIPHIER_SLD3_ID:
		uniphier_sld3_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || defined(CONFIG_ARCH_UNIPHIER_PRO4) || \
	defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case UNIPHIER_LD4_ID:
	case UNIPHIER_PRO4_ID:
	case UNIPHIER_SLD8_ID:
		uniphier_ld4_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case UNIPHIER_PRO5_ID:
		uniphier_pro5_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case UNIPHIER_PXS2_ID:
	case UNIPHIER_LD6B_ID:
		uniphier_pxs2_boot_mode_show();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11) || defined(CONFIG_ARCH_UNIPHIER_LD20)
	case UNIPHIER_LD11_ID:
	case UNIPHIER_LD20_ID:
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
