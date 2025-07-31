// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#include <dm.h>
#include "stm32-reset-core.h"

/* Timeout for deassert */
#define STM32_DEASSERT_TIMEOUT_US	10000

static const struct stm32_reset_cfg *stm32_get_reset_line(struct reset_ctl *reset_ctl)
{
	struct stm32_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	struct stm32_reset_cfg *ptr_line = &priv->reset_line;
	int bank = (reset_ctl->id / (sizeof(u32) * BITS_PER_BYTE)) * 4;
	int offset = reset_ctl->id % (sizeof(u32) * BITS_PER_BYTE);

	ptr_line->offset = bank;
	ptr_line->bit_idx = offset;
	ptr_line->set_clr = false;

	return ptr_line;
}

static const struct stm32_reset_data stm32_reset_data = {
	.get_reset_line	= stm32_get_reset_line,
	.reset_us	= STM32_DEASSERT_TIMEOUT_US,
};

static int stm32_reset_probe(struct udevice *dev)
{
	return stm32_reset_core_probe(dev, &stm32_reset_data);
}

U_BOOT_DRIVER(stm32_rcc_reset) = {
	.name		= "stm32_rcc_reset",
	.id		= UCLASS_RESET,
	.probe		= stm32_reset_probe,
	.priv_auto	= sizeof(struct stm32_reset_priv),
	.ops		= &stm32_reset_ops,
};
