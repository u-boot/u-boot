/*
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

static const unsigned long io_base[] = {
	STM32_GPIOA_BASE, STM32_GPIOB_BASE, STM32_GPIOC_BASE,
	STM32_GPIOD_BASE, STM32_GPIOE_BASE, STM32_GPIOF_BASE,
	STM32_GPIOG_BASE, STM32_GPIOH_BASE, STM32_GPIOI_BASE
};

struct stm32_gpio_regs {
	u32 moder;	/* GPIO port mode */
	u32 otyper;	/* GPIO port output type */
	u32 ospeedr;	/* GPIO port output speed */
	u32 pupdr;	/* GPIO port pull-up/pull-down */
	u32 idr;	/* GPIO port input data */
	u32 odr;	/* GPIO port output data */
	u32 bsrr;	/* GPIO port bit set/reset */
	u32 lckr;	/* GPIO port configuration lock */
	u32 afr[2];	/* GPIO alternate function */
};

#define CHECK_DSC(x)	(!x || x->port > 8 || x->pin > 15)
#define CHECK_CTL(x)	(!x || x->af > 15 || x->mode > 3 || x->otype > 1 || \
			x->pupd > 2 || x->speed > 3)

int stm32_gpio_config(const struct stm32_gpio_dsc *dsc,
		const struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_regs *gpio_regs;
	u32 i;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}
	if (CHECK_CTL(ctl)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	i = (dsc->pin & 0x07) * 4;
	clrsetbits_le32(&gpio_regs->afr[dsc->pin >> 3], 0xF << i, ctl->af << i);

	i = dsc->pin * 2;

	clrsetbits_le32(&gpio_regs->moder, 0x3 << i, ctl->mode << i);
	clrsetbits_le32(&gpio_regs->otyper, 0x3 << i, ctl->otype << i);
	clrsetbits_le32(&gpio_regs->ospeedr, 0x3 << i, ctl->speed << i);
	clrsetbits_le32(&gpio_regs->pupdr, 0x3 << i, ctl->pupd << i);

	rv = 0;
out:
	return rv;
}

int stm32_gpout_set(const struct stm32_gpio_dsc *dsc, int state)
{
	struct stm32_gpio_regs	*gpio_regs;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	if (state)
		writel(1 << dsc->pin, &gpio_regs->bsrr);
	else
		writel(1 << (dsc->pin + 16), &gpio_regs->bsrr);

	rv = 0;
out:
	return rv;
}

int stm32_gpin_get(const struct stm32_gpio_dsc *dsc)
{
	struct stm32_gpio_regs	*gpio_regs;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];
	rv = readl(&gpio_regs->idr) & (1 << dsc->pin);
out:
	return rv;
}

/* Common GPIO API */

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	struct stm32_gpio_dsc dsc;
	struct stm32_gpio_ctl ctl;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);
	ctl.af = STM32_GPIO_AF0;
	ctl.mode = STM32_GPIO_MODE_IN;
	ctl.otype = STM32_GPIO_OTYPE_PP;
	ctl.pupd = STM32_GPIO_PUPD_NO;
	ctl.speed = STM32_GPIO_SPEED_50M;

	return stm32_gpio_config(&dsc, &ctl);
}

int gpio_direction_output(unsigned gpio, int value)
{
	struct stm32_gpio_dsc dsc;
	struct stm32_gpio_ctl ctl;
	int res;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);
	ctl.af = STM32_GPIO_AF0;
	ctl.mode = STM32_GPIO_MODE_OUT;
	ctl.pupd = STM32_GPIO_PUPD_NO;
	ctl.speed = STM32_GPIO_SPEED_50M;

	res = stm32_gpio_config(&dsc, &ctl);
	if (res < 0)
		goto out;
	res = stm32_gpout_set(&dsc, value);
out:
	return res;
}

int gpio_get_value(unsigned gpio)
{
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	return stm32_gpin_get(&dsc);
}

int gpio_set_value(unsigned gpio, int value)
{
	struct stm32_gpio_dsc dsc;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);

	return stm32_gpout_set(&dsc, value);
}
