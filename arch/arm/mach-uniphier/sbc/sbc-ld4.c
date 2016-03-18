/*
 * Copyright (C) 2011-2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "sbc-regs.h"

int uniphier_ld4_sbc_init(const struct uniphier_board_data *bd)
{
	u32 tmp;

	/* system bus output enable */
	tmp = readl(PC0CTRL);
	tmp &= 0xfffffcff;
	writel(tmp, PC0CTRL);

	return 0;
}
