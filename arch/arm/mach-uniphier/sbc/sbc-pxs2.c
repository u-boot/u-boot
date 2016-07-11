/*
 * Copyright (C) 2015-2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "sbc-regs.h"

int uniphier_pxs2_sbc_init(const struct uniphier_board_data *bd)
{
	/* necessary for ROM boot ?? */
	/* system bus output enable */
	writel(0x17, PC0CTRL);

	return 0;
}
