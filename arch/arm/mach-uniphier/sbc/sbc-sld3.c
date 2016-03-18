/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

int uniphier_sld3_sbc_init(const struct uniphier_board_data *bd)
{
	sg_set_pinsel(99, 1, 4, 4);	/* GPIO26 -> EA24 */

	return 0;
}
