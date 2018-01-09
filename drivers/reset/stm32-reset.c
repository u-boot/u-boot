/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset-uclass.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct stm32_reset_priv {
	fdt_addr_t base;
};

static int stm32_reset_request(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int stm32_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int stm32_reset_assert(struct reset_ctl *reset_ctl)
{
	struct stm32_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int bank = (reset_ctl->id / BITS_PER_LONG) * 4;
	int offset = reset_ctl->id % BITS_PER_LONG;
	debug("%s: reset id = %ld bank = %d offset = %d)\n", __func__,
	      reset_ctl->id, bank, offset);

	setbits_le32(priv->base + bank, BIT(offset));

	return 0;
}

static int stm32_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct stm32_reset_priv *priv = dev_get_priv(reset_ctl->dev);
	int bank = (reset_ctl->id / BITS_PER_LONG) * 4;
	int offset = reset_ctl->id % BITS_PER_LONG;
	debug("%s: reset id = %ld bank = %d offset = %d)\n", __func__,
	      reset_ctl->id, bank, offset);

	clrbits_le32(priv->base + bank, BIT(offset));

	return 0;
}

static const struct reset_ops stm32_reset_ops = {
	.request	= stm32_reset_request,
	.free		= stm32_reset_free,
	.rst_assert	= stm32_reset_assert,
	.rst_deassert	= stm32_reset_deassert,
};

static int stm32_reset_probe(struct udevice *dev)
{
	struct stm32_reset_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_get_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(stm32_rcc_reset) = {
	.name			= "stm32_rcc_reset",
	.id			= UCLASS_RESET,
	.probe			= stm32_reset_probe,
	.priv_auto_alloc_size	= sizeof(struct stm32_reset_priv),
	.ops			= &stm32_reset_ops,
};
