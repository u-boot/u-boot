// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 * Author(s): Gabriel Fernandez, <gabriel.fernandez@foss.st.com> for STMicroelectronics.
 */

#include <dm.h>
#include <reset-uclass.h>
#include "stm32-reset-core.h"
#include <stm32_rcc.h>
#include <dm/device_compat.h>
#include <linux/iopoll.h>

static int stm32_reset_update(struct reset_ctl *reset_ctl, bool status)
{
	struct stm32_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	const struct stm32_reset_data *data = priv->data;
	const struct stm32_reset_cfg *ptr_line;
	fdt_addr_t addr;

	assert(priv->data->get_reset_line);

	ptr_line = priv->data->get_reset_line(reset_ctl);
	if (!ptr_line)
		return -EPERM;

	addr = priv->base + ptr_line->offset;

	dev_dbg(reset_ctl->dev, "reset id=%ld offset=0x%x bit=%d status=%d\n",
		reset_ctl->id, ptr_line->offset, ptr_line->bit_idx, status);

	status = ptr_line->inverted ^ status;

	if (ptr_line->set_clr) {
		if (!status)
			addr += data->clear_offset;

		writel(BIT(ptr_line->bit_idx), addr);

	} else {
		if (status)
			setbits_le32(addr, BIT(ptr_line->bit_idx));
		else
			clrbits_le32(addr, BIT(ptr_line->bit_idx));
	}

	/* Check deassert */
	if (!status) {
		u32 reg;

		return readl_poll_timeout(addr, reg,
					  !(reg & BIT(ptr_line->bit_idx)),
					  data->reset_us);
	}

	return 0;
}

static int stm32_reset_assert(struct reset_ctl *reset_ctl)
{
	return stm32_reset_update(reset_ctl, true);
}

static int stm32_reset_deassert(struct reset_ctl *reset_ctl)
{
	return stm32_reset_update(reset_ctl, false);
}

const struct reset_ops stm32_reset_ops = {
	.rst_assert	= stm32_reset_assert,
	.rst_deassert	= stm32_reset_deassert,
};

int stm32_reset_core_probe(struct udevice *dev,
			   const struct stm32_reset_data *data)
{
	struct stm32_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE) {
		/* for MFD, get address of parent */
		priv->base = dev_read_addr(dev->parent);
		if (priv->base == FDT_ADDR_T_NONE)
			return -EINVAL;
	}

	priv->data = data;

	assert(priv->data);

	return 0;
}
