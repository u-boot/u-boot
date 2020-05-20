// SPDX-License-Identifier: GPL-2.0 or later
/*
 * Copyright (C) 2020 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <reset.h>

#include "init.h"

/*
 * Assert the Denali NAND controller reset if found.
 *
 * On LD4, the bootstrap process starts running after power-on reset regardless
 * of the boot mode, here the pin-mux is not necessarily set up for NAND, then
 * the controller is stuck. Assert the controller reset here, and should be
 * deasserted in the driver after the pin-mux is correctly handled. For other
 * SoCs, the bootstrap runs only when the boot mode selects ONFi, but it is yet
 * effective when the boot swap is on. So, the reset should be asserted anyway.
 */
void uniphier_nand_reset_assert(void)
{
	struct udevice *dev;
	struct reset_ctl_bulk resets;
	int ret;

	ret = uclass_find_first_device(UCLASS_MTD, &dev);
	if (ret || !dev)
		return;

	/* make sure this is the Denali NAND controller */
	if (strcmp(dev->driver->name, "denali-nand-dt"))
		return;

	ret = reset_get_bulk(dev, &resets);
	if (ret)
		return;

	reset_assert_bulk(&resets);
}
