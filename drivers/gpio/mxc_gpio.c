/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
 *
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, <sbabic@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <malloc.h>
#include <asm/arch/imx-regs.h>
#include <asm/gpio.h>
#include <asm/io.h>

enum mxc_gpio_direction {
	MXC_GPIO_DIRECTION_IN,
	MXC_GPIO_DIRECTION_OUT,
};

#define GPIO_NAME_SIZE			20
#define GPIO_PER_BANK			32

struct mxc_gpio_plat {
	struct gpio_regs *regs;
};

struct mxc_bank_info {
	char label[GPIO_PER_BANK][GPIO_NAME_SIZE];
	struct gpio_regs *regs;
};

#ifndef CONFIG_DM_GPIO
#define GPIO_TO_PORT(n)		(n / 32)

/* GPIO port description */
static unsigned long gpio_ports[] = {
	[0] = GPIO1_BASE_ADDR,
	[1] = GPIO2_BASE_ADDR,
	[2] = GPIO3_BASE_ADDR,
#if defined(CONFIG_MX25) || defined(CONFIG_MX27) || defined(CONFIG_MX51) || \
		defined(CONFIG_MX53) || defined(CONFIG_MX6)
	[3] = GPIO4_BASE_ADDR,
#endif
#if defined(CONFIG_MX27) || defined(CONFIG_MX53) || defined(CONFIG_MX6)
	[4] = GPIO5_BASE_ADDR,
	[5] = GPIO6_BASE_ADDR,
#endif
#if defined(CONFIG_MX53) || defined(CONFIG_MX6)
	[6] = GPIO7_BASE_ADDR,
#endif
};

static int mxc_gpio_direction(unsigned int gpio,
	enum mxc_gpio_direction direction)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	struct gpio_regs *regs;
	u32 l;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	gpio &= 0x1f;

	regs = (struct gpio_regs *)gpio_ports[port];

	l = readl(&regs->gpio_dir);

	switch (direction) {
	case MXC_GPIO_DIRECTION_OUT:
		l |= 1 << gpio;
		break;
	case MXC_GPIO_DIRECTION_IN:
		l &= ~(1 << gpio);
	}
	writel(l, &regs->gpio_dir);

	return 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	struct gpio_regs *regs;
	u32 l;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	gpio &= 0x1f;

	regs = (struct gpio_regs *)gpio_ports[port];

	l = readl(&regs->gpio_dr);
	if (value)
		l |= 1 << gpio;
	else
		l &= ~(1 << gpio);
	writel(l, &regs->gpio_dr);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	struct gpio_regs *regs;
	u32 val;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	gpio &= 0x1f;

	regs = (struct gpio_regs *)gpio_ports[port];

	val = (readl(&regs->gpio_psr) >> gpio) & 0x01;

	return val;
}

int gpio_request(unsigned gpio, const char *label)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	return mxc_gpio_direction(gpio, MXC_GPIO_DIRECTION_IN);
}

int gpio_direction_output(unsigned gpio, int value)
{
	int ret = gpio_set_value(gpio, value);

	if (ret < 0)
		return ret;

	return mxc_gpio_direction(gpio, MXC_GPIO_DIRECTION_OUT);
}
#endif

#ifdef CONFIG_DM_GPIO
/**
 * gpio_is_requested() - check if a GPIO has been requested
 *
 * @bank:	Bank to check
 * @offset:	GPIO offset within bank to check
 * @return true if marked as requested, false if not
 */
static inline bool gpio_is_requested(struct mxc_bank_info *bank, int offset)
{
	return *bank->label[offset] != '\0';
}

static int mxc_gpio_is_output(struct gpio_regs *regs, int offset)
{
	u32 val;

	val = readl(&regs->gpio_dir);

	return val & (1 << offset) ? 1 : 0;
}

static void mxc_gpio_bank_direction(struct gpio_regs *regs, int offset,
				    enum mxc_gpio_direction direction)
{
	u32 l;

	l = readl(&regs->gpio_dir);

	switch (direction) {
	case MXC_GPIO_DIRECTION_OUT:
		l |= 1 << offset;
		break;
	case MXC_GPIO_DIRECTION_IN:
		l &= ~(1 << offset);
	}
	writel(l, &regs->gpio_dir);
}

static void mxc_gpio_bank_set_value(struct gpio_regs *regs, int offset,
				    int value)
{
	u32 l;

	l = readl(&regs->gpio_dr);
	if (value)
		l |= 1 << offset;
	else
		l &= ~(1 << offset);
	writel(l, &regs->gpio_dr);
}

static int mxc_gpio_bank_get_value(struct gpio_regs *regs, int offset)
{
	return (readl(&regs->gpio_psr) >> offset) & 0x01;
}

static int mxc_gpio_bank_get_output_value(struct gpio_regs *regs, int offset)
{
	return (readl(&regs->gpio_dr) >> offset) & 0x01;
}

static int check_requested(struct udevice *dev, unsigned offset,
			   const char *func)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

	if (!gpio_is_requested(bank, offset)) {
		printf("mxc_gpio: %s: error: gpio %s%d not requested\n",
		       func, uc_priv->bank_name, offset);
		return -EPERM;
	}

	return 0;
}

/* set GPIO pin 'gpio' as an input */
static int mxc_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	/* Configure GPIO direction as input. */
	mxc_gpio_bank_direction(bank->regs, offset, MXC_GPIO_DIRECTION_IN);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int mxc_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	/* Configure GPIO output value. */
	mxc_gpio_bank_set_value(bank->regs, offset, value);

	/* Configure GPIO direction as output. */
	mxc_gpio_bank_direction(bank->regs, offset, MXC_GPIO_DIRECTION_OUT);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int mxc_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	return mxc_gpio_bank_get_value(bank->regs, offset);
}

/* write GPIO OUT value to pin 'gpio' */
static int mxc_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	mxc_gpio_bank_set_value(bank->regs, offset, value);

	return 0;
}

static int mxc_gpio_get_state(struct udevice *dev, unsigned int offset,
			      char *buf, int bufsize)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct mxc_bank_info *bank = dev_get_priv(dev);
	const char *label;
	bool requested;
	bool is_output;
	int size;

	label = bank->label[offset];
	is_output = mxc_gpio_is_output(bank->regs, offset);
	size = snprintf(buf, bufsize, "%s%d: ",
			uc_priv->bank_name ? uc_priv->bank_name : "", offset);
	buf += size;
	bufsize -= size;
	requested = gpio_is_requested(bank, offset);
	snprintf(buf, bufsize, "%s: %d [%c]%s%s",
		 is_output ? "out" : " in",
		 is_output ?
			mxc_gpio_bank_get_output_value(bank->regs, offset) :
			mxc_gpio_bank_get_value(bank->regs, offset),
		 requested ? 'x' : ' ',
		 requested ? " " : "",
		 label);

	return 0;
}

static int mxc_gpio_request(struct udevice *dev, unsigned offset,
			      const char *label)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);

	if (gpio_is_requested(bank, offset))
		return -EBUSY;

	strncpy(bank->label[offset], label, GPIO_NAME_SIZE);
	bank->label[offset][GPIO_NAME_SIZE - 1] = '\0';

	return 0;
}

static int mxc_gpio_free(struct udevice *dev, unsigned offset)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;
	bank->label[offset][0] = '\0';

	return 0;
}

static int mxc_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);

	if (!gpio_is_requested(bank, offset))
		return GPIOF_UNUSED;

	/* GPIOF_FUNC is not implemented yet */
	if (mxc_gpio_is_output(bank->regs, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_mxc_ops = {
	.request		= mxc_gpio_request,
	.free			= mxc_gpio_free,
	.direction_input	= mxc_gpio_direction_input,
	.direction_output	= mxc_gpio_direction_output,
	.get_value		= mxc_gpio_get_value,
	.set_value		= mxc_gpio_set_value,
	.get_function		= mxc_gpio_get_function,
	.get_state		= mxc_gpio_get_state,
};

static const struct mxc_gpio_plat mxc_plat[] = {
	{ (struct gpio_regs *)GPIO1_BASE_ADDR },
	{ (struct gpio_regs *)GPIO2_BASE_ADDR },
	{ (struct gpio_regs *)GPIO3_BASE_ADDR },
#if defined(CONFIG_MX25) || defined(CONFIG_MX27) || defined(CONFIG_MX51) || \
		defined(CONFIG_MX53) || defined(CONFIG_MX6)
	{ (struct gpio_regs *)GPIO4_BASE_ADDR },
#endif
#if defined(CONFIG_MX27) || defined(CONFIG_MX53) || defined(CONFIG_MX6)
	{ (struct gpio_regs *)GPIO5_BASE_ADDR },
	{ (struct gpio_regs *)GPIO6_BASE_ADDR },
#endif
#if defined(CONFIG_MX53) || defined(CONFIG_MX6)
	{ (struct gpio_regs *)GPIO7_BASE_ADDR },
#endif
};

static int mxc_gpio_probe(struct udevice *dev)
{
	struct mxc_bank_info *bank = dev_get_priv(dev);
	struct mxc_gpio_plat *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	int banknum;
	char name[18], *str;

	banknum = plat - mxc_plat;
	sprintf(name, "GPIO%d_", banknum + 1);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;
	uc_priv->gpio_count = GPIO_PER_BANK;
	bank->regs = plat->regs;

	return 0;
}

U_BOOT_DRIVER(gpio_mxc) = {
	.name	= "gpio_mxc",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mxc_ops,
	.probe	= mxc_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct mxc_bank_info),
};

U_BOOT_DEVICES(mxc_gpios) = {
	{ "gpio_mxc", &mxc_plat[0] },
	{ "gpio_mxc", &mxc_plat[1] },
	{ "gpio_mxc", &mxc_plat[2] },
#if defined(CONFIG_MX25) || defined(CONFIG_MX27) || defined(CONFIG_MX51) || \
		defined(CONFIG_MX53) || defined(CONFIG_MX6)
	{ "gpio_mxc", &mxc_plat[3] },
#endif
#if defined(CONFIG_MX27) || defined(CONFIG_MX53) || defined(CONFIG_MX6)
	{ "gpio_mxc", &mxc_plat[4] },
	{ "gpio_mxc", &mxc_plat[5] },
#endif
#if defined(CONFIG_MX53) || defined(CONFIG_MX6)
	{ "gpio_mxc", &mxc_plat[6] },
#endif
};
#endif
