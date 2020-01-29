// SPDX-License-Identifier: GPL-2.0+
//
// Copyright (C) 2011-2014 Panasonic Corporation
// Copyright (C) 2015-2019 Socionext Inc.

#include <linux/io.h>

#include "sbc-regs.h"

int uniphier_sbc_boot_is_swapped(void)
{
	return !(readl(SBBASE0) & SBBASE_BANK_ENABLE);
}
