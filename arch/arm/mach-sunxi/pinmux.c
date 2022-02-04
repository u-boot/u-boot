// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

void sunxi_gpio_set_cfgbank(struct sunxi_gpio *pio, int bank_offset, u32 val)
{
	u32 index = GPIO_CFG_INDEX(bank_offset);
	u32 offset = GPIO_CFG_OFFSET(bank_offset);

	clrsetbits_le32(&pio->cfg[index], 0xf << offset, val << offset);
}

void sunxi_gpio_set_cfgpin(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	sunxi_gpio_set_cfgbank(pio, pin, val);
}

int sunxi_gpio_get_cfgbank(struct sunxi_gpio *pio, int bank_offset)
{
	u32 index = GPIO_CFG_INDEX(bank_offset);
	u32 offset = GPIO_CFG_OFFSET(bank_offset);
	u32 cfg;

	cfg = readl(&pio->cfg[index]);
	cfg >>= offset;

	return cfg & 0xf;
}

int sunxi_gpio_get_cfgpin(u32 pin)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	return sunxi_gpio_get_cfgbank(pio, pin);
}

void sunxi_gpio_set_drv(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	sunxi_gpio_set_drv_bank(pio, pin, val);
}

void sunxi_gpio_set_drv_bank(struct sunxi_gpio *pio, u32 bank_offset, u32 val)
{
	u32 index = GPIO_DRV_INDEX(bank_offset);
	u32 offset = GPIO_DRV_OFFSET(bank_offset);

	clrsetbits_le32(&pio->drv[index], 0x3 << offset, val << offset);
}

void sunxi_gpio_set_pull(u32 pin, u32 val)
{
	u32 bank = GPIO_BANK(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	sunxi_gpio_set_pull_bank(pio, pin, val);
}

void sunxi_gpio_set_pull_bank(struct sunxi_gpio *pio, int bank_offset, u32 val)
{
	u32 index = GPIO_PULL_INDEX(bank_offset);
	u32 offset = GPIO_PULL_OFFSET(bank_offset);

	clrsetbits_le32(&pio->pull[index], 0x3 << offset, val << offset);
}
