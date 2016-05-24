/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include "../init.h"
#include "../soc-info.h"

void spl_board_init(void)
{
	const struct uniphier_board_data *param;

#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	param = uniphier_get_board_param();
	if (!param)
		hang();

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case SOC_UNIPHIER_SLD3:
		uniphier_sld3_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	case SOC_UNIPHIER_LD4:
		uniphier_ld4_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	case SOC_UNIPHIER_PRO4:
		uniphier_pro4_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case SOC_UNIPHIER_SLD8:
		uniphier_sld8_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case SOC_UNIPHIER_PRO5:
		uniphier_pro5_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case SOC_UNIPHIER_PXS2:
	case SOC_UNIPHIER_LD6B:
		uniphier_pxs2_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
	case SOC_UNIPHIER_LD11:
		uniphier_ld11_init(param);
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	case SOC_UNIPHIER_LD20:
		uniphier_ld20_init(param);
		break;
#endif
	default:
		break;
	}
}
