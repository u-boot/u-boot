/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include "micro-support-card.h"

void uniphier_smp_kick_all_cpus(void);

int board_init(void)
{
	led_puts("Uboo");
#ifdef CONFIG_ARM64
	uniphier_smp_kick_all_cpus();
#endif
	return 0;
}
