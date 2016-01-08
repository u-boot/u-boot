/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "micro-support-card.h"

int misc_init_f(void)
{
	return check_support_card();
}
