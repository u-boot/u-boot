/*
 * Copyright (C) 2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/micro-support-card.h>

int misc_init_f(void)
{
	return check_support_card();
}
