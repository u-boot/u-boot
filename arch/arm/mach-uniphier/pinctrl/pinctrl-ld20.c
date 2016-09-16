/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

void uniphier_ld20_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

	sg_set_pinsel(149, 14, 8, 4);	/* XIRQ0    -> XIRQ0 */
	sg_set_iectrl(149);
	sg_set_pinsel(153, 14, 8, 4);	/* XIRQ4    -> XIRQ4 */
	sg_set_iectrl(153);
}
