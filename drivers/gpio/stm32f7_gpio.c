/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <asm/arch/gpio.h>
#include <asm/arch/stm32.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/io.h>

#define MAX_SIZE_BANK_NAME		5
#define STM32_GPIOS_PER_BANK		16
#define MODE_BITS(gpio_pin)		(gpio_pin * 2)
#define MODE_BITS_MASK			3
#define IN_OUT_BIT_INDEX(gpio_pin)	(1UL << (gpio_pin))

DECLARE_GLOBAL_DATA_PTR;

static int stm32_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int bits_index = MODE_BITS(offset);
	int mask = MODE_BITS_MASK << bits_index;

	clrsetbits_le32(&regs->moder, mask, STM32_GPIO_MODE_IN << bits_index);

	return 0;
}

static int stm32_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int bits_index = MODE_BITS(offset);
	int mask = MODE_BITS_MASK << bits_index;

	clrsetbits_le32(&regs->moder, mask, STM32_GPIO_MODE_OUT << bits_index);
	mask = IN_OUT_BIT_INDEX(offset);
	clrsetbits_le32(&regs->odr, mask, value ? mask : 0);

	return 0;
}

static int stm32_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;

	return readl(&regs->idr) & IN_OUT_BIT_INDEX(offset) ? 1 : 0;
}

static int stm32_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int mask = IN_OUT_BIT_INDEX(offset);

	clrsetbits_le32(&regs->odr, mask, value ? mask : 0);

	return 0;
}

static const struct dm_gpio_ops gpio_stm32_ops = {
	.direction_input	= stm32_gpio_direction_input,
	.direction_output	= stm32_gpio_direction_output,
	.get_value		= stm32_gpio_get_value,
	.set_value		= stm32_gpio_set_value,
};

static int gpio_stm32_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	char *name;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = (struct stm32_gpio_regs *)addr;
	name = (char *)fdtdec_locate_byte_array(gd->fdt_blob,
						dev_of_offset(dev),
						"st,bank-name",
						MAX_SIZE_BANK_NAME);
	if (!name)
		return -EINVAL;
	uc_priv->bank_name = name;
	uc_priv->gpio_count = STM32_GPIOS_PER_BANK;
	debug("%s, addr = 0x%p, bank_name = %s\n", __func__, (u32 *)priv->regs,
	      uc_priv->bank_name);

#ifdef CONFIG_CLK
	struct clk clk;
	int ret;
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);

	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
	debug("clock enabled for device %s\n", dev->name);
#endif

	return 0;
}

static const struct udevice_id stm32_gpio_ids[] = {
	{ .compatible = "st,stm32-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_stm32) = {
	.name	= "gpio_stm32",
	.id	= UCLASS_GPIO,
	.of_match = stm32_gpio_ids,
	.probe	= gpio_stm32_probe,
	.ops	= &gpio_stm32_ops,
	.flags	= DM_FLAG_PRE_RELOC | DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto_alloc_size	= sizeof(struct stm32_gpio_priv),
};
