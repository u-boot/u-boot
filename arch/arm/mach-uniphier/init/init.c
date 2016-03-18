/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

#include "../init.h"
#include "../soc-info.h"

void spl_board_init(void)
{
	const struct uniphier_board_data *param;

	param = uniphier_get_board_param();
	if (!param)
		hang();

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case SOC_UNIPHIER_SLD3:
		ph1_sld3_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	case SOC_UNIPHIER_LD4:
		ph1_ld4_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	case SOC_UNIPHIER_PRO4:
		ph1_pro4_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case SOC_UNIPHIER_SLD8:
		ph1_sld8_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case SOC_UNIPHIER_PRO5:
		ph1_pro5_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case SOC_UNIPHIER_PXS2:
	case SOC_UNIPHIER_LD6B:
		proxstream2_init(param);
		break;
#endif
	default:
		break;
	}
}
