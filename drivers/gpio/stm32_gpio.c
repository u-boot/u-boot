/*
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_STM32F4)
#define STM32_GPIOA_BASE	(STM32_AHB1PERIPH_BASE + 0x0000)
#define STM32_GPIOB_BASE	(STM32_AHB1PERIPH_BASE + 0x0400)
#define STM32_GPIOC_BASE	(STM32_AHB1PERIPH_BASE + 0x0800)
#define STM32_GPIOD_BASE	(STM32_AHB1PERIPH_BASE + 0x0C00)
#define STM32_GPIOE_BASE	(STM32_AHB1PERIPH_BASE + 0x1000)
#define STM32_GPIOF_BASE	(STM32_AHB1PERIPH_BASE + 0x1400)
#define STM32_GPIOG_BASE	(STM32_AHB1PERIPH_BASE + 0x1800)
#define STM32_GPIOH_BASE	(STM32_AHB1PERIPH_BASE + 0x1C00)
#define STM32_GPIOI_BASE	(STM32_AHB1PERIPH_BASE + 0x2000)

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

	setbits_le32(&STM32_RCC->ahb1enr, 1 << dsc->port);

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
#elif defined(CONFIG_STM32F1)
#define STM32_GPIOA_BASE	(STM32_APB2PERIPH_BASE + 0x0800)
#define STM32_GPIOB_BASE	(STM32_APB2PERIPH_BASE + 0x0C00)
#define STM32_GPIOC_BASE	(STM32_APB2PERIPH_BASE + 0x1000)
#define STM32_GPIOD_BASE	(STM32_APB2PERIPH_BASE + 0x1400)
#define STM32_GPIOE_BASE	(STM32_APB2PERIPH_BASE + 0x1800)
#define STM32_GPIOF_BASE	(STM32_APB2PERIPH_BASE + 0x1C00)
#define STM32_GPIOG_BASE	(STM32_APB2PERIPH_BASE + 0x2000)

static const unsigned long io_base[] = {
	STM32_GPIOA_BASE, STM32_GPIOB_BASE, STM32_GPIOC_BASE,
	STM32_GPIOD_BASE, STM32_GPIOE_BASE, STM32_GPIOF_BASE,
	STM32_GPIOG_BASE
};

#define STM32_GPIO_CR_MODE_MASK		0x3
#define STM32_GPIO_CR_MODE_SHIFT(p)	(p * 4)
#define STM32_GPIO_CR_CNF_MASK		0x3
#define STM32_GPIO_CR_CNF_SHIFT(p)	(p * 4 + 2)

struct stm32_gpio_regs {
	u32 crl;	/* GPIO port configuration low */
	u32 crh;	/* GPIO port configuration high */
	u32 idr;	/* GPIO port input data */
	u32 odr;	/* GPIO port output data */
	u32 bsrr;	/* GPIO port bit set/reset */
	u32 brr;	/* GPIO port bit reset */
	u32 lckr;	/* GPIO port configuration lock */
};

#define CHECK_DSC(x)	(!x || x->port > 6 || x->pin > 15)
#define CHECK_CTL(x)	(!x || x->mode > 3 || x->icnf > 3 || x->ocnf > 3 || \
			 x->pupd > 1)

int stm32_gpio_config(const struct stm32_gpio_dsc *dsc,
		const struct stm32_gpio_ctl *ctl)
{
	struct stm32_gpio_regs *gpio_regs;
	u32 *cr;
	int p, crp;
	int rv;

	if (CHECK_DSC(dsc)) {
		rv = -EINVAL;
		goto out;
	}
	if (CHECK_CTL(ctl)) {
		rv = -EINVAL;
		goto out;
	}

	p = dsc->pin;

	gpio_regs = (struct stm32_gpio_regs *)io_base[dsc->port];

	/* Enable clock for GPIO port */
	setbits_le32(&STM32_RCC->apb2enr, 0x04 << dsc->port);

	if (p < 8) {
		cr = &gpio_regs->crl;
		crp = p;
	} else {
		cr = &gpio_regs->crh;
		crp = p - 8;
	}

	clrbits_le32(cr, 0x3 << STM32_GPIO_CR_MODE_SHIFT(crp));
	setbits_le32(cr, ctl->mode << STM32_GPIO_CR_MODE_SHIFT(crp));

	clrbits_le32(cr, 0x3 << STM32_GPIO_CR_CNF_SHIFT(crp));
	/* Inputs set the optional pull up / pull down */
	if (ctl->mode == STM32_GPIO_MODE_IN) {
		setbits_le32(cr, ctl->icnf << STM32_GPIO_CR_CNF_SHIFT(crp));
		clrbits_le32(&gpio_regs->odr, 0x1 << p);
		setbits_le32(&gpio_regs->odr, ctl->pupd << p);
	} else {
		setbits_le32(cr, ctl->ocnf << STM32_GPIO_CR_CNF_SHIFT(crp));
	}

	rv = 0;
out:
	return rv;
}
#else
#error STM32 family not supported
#endif

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
#if defined(CONFIG_STM32F4)
	ctl.af = STM32_GPIO_AF0;
	ctl.mode = STM32_GPIO_MODE_IN;
	ctl.otype = STM32_GPIO_OTYPE_PP;
	ctl.pupd = STM32_GPIO_PUPD_NO;
	ctl.speed = STM32_GPIO_SPEED_50M;
#elif defined(CONFIG_STM32F1)
	ctl.mode = STM32_GPIO_MODE_IN;
	ctl.icnf = STM32_GPIO_ICNF_IN_FLT;
	ctl.ocnf = STM32_GPIO_OCNF_GP_PP;	/* ignored for input */
	ctl.pupd = STM32_GPIO_PUPD_UP;		/* ignored for floating */
#else
#error STM32 family not supported
#endif

	return stm32_gpio_config(&dsc, &ctl);
}

int gpio_direction_output(unsigned gpio, int value)
{
	struct stm32_gpio_dsc dsc;
	struct stm32_gpio_ctl ctl;
	int res;

	dsc.port = stm32_gpio_to_port(gpio);
	dsc.pin = stm32_gpio_to_pin(gpio);
#if defined(CONFIG_STM32F4)
	ctl.af = STM32_GPIO_AF0;
	ctl.mode = STM32_GPIO_MODE_OUT;
	ctl.pupd = STM32_GPIO_PUPD_NO;
	ctl.speed = STM32_GPIO_SPEED_50M;
#elif defined(CONFIG_STM32F1)
	ctl.mode = STM32_GPIO_MODE_OUT_50M;
	ctl.ocnf = STM32_GPIO_OCNF_GP_PP;
	ctl.icnf = STM32_GPIO_ICNF_IN_FLT;	/* ignored for output */
	ctl.pupd = STM32_GPIO_PUPD_UP;		/* ignored for output */
#else
#error STM32 family not supported
#endif

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
