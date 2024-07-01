// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <cyclic.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <log.h>
#include <malloc.h>
#include <asm/gpio.h>

struct led_gpio_priv {
	struct gpio_desc gpio;
#ifdef CONFIG_LED_GPIO_SW_BLINK
	bool sw_blink;
	struct cyclic_info cyclic;
#endif
};

#ifdef CONFIG_LED_GPIO_SW_BLINK
static void gpio_led_toggle(struct cyclic_info *ctx)
{
	struct led_gpio_priv *priv = container_of(ctx, struct led_gpio_priv, cyclic);
	struct gpio_desc *gpio = &priv->gpio;
	int state;

	state = dm_gpio_get_value(gpio);
	if (state < 0) {
		printf("Error getting value for GPIO %d\n",
		       gpio->offset);
		return;
	}

	dm_gpio_set_value(gpio, !state);
}

static int gpio_led_set_period(struct udevice *dev, int period_ms)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);
	char cyclic_name[16];

	if (priv->sw_blink)
		cyclic_unregister(&priv->cyclic);

	snprintf(cyclic_name, sizeof(cyclic_name),
		 "gpio_cyclic%u", priv->gpio.offset);
	cyclic_register(&priv->cyclic, gpio_led_toggle,
			period_ms * 500, cyclic_name);

	/* Init the LED on */
	dm_gpio_set_value(&priv->gpio, LEDST_ON);

	priv->sw_blink = true;
	return 0;
}
#endif

static int gpio_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);
	int ret;

#ifdef CONFIG_LED_GPIO_SW_BLINK
	if (priv->sw_blink)
		cyclic_unregister(&priv->cyclic);
#endif

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	switch (state) {
	case LEDST_OFF:
	case LEDST_ON:
		break;
	case LEDST_TOGGLE:
		ret = dm_gpio_get_value(&priv->gpio);
		if (ret < 0)
			return ret;
		state = !ret;
		break;
	default:
		return -ENOSYS;
	}

	return dm_gpio_set_value(&priv->gpio, state);
}

static enum led_state_t gpio_led_get_state(struct udevice *dev)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	ret = dm_gpio_get_value(&priv->gpio);
	if (ret < 0)
		return ret;

#ifdef CONFIG_LED_GPIO_SW_BLINK
	if (priv->sw_blink)
		return LEDST_BLINK;
#endif

	return ret ? LEDST_ON : LEDST_OFF;
}

static int led_gpio_probe(struct udevice *dev)
{
	struct led_gpio_priv *priv = dev_get_priv(dev);

	return gpio_request_by_name(dev, "gpios", 0, &priv->gpio, GPIOD_IS_OUT);
}

static int led_gpio_remove(struct udevice *dev)
{
	/*
	 * The GPIO driver may have already been removed. We will need to
	 * address this more generally.
	 */
#ifndef CONFIG_SANDBOX
	struct led_gpio_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->gpio))
		dm_gpio_free(dev, &priv->gpio);
#endif

	return 0;
}

static int led_gpio_bind(struct udevice *parent)
{
	return led_bind_generic(parent, "gpio_led");
}

static const struct led_ops gpio_led_ops = {
	.set_state	= gpio_led_set_state,
	.get_state	= gpio_led_get_state,
#ifdef CONFIG_LED_GPIO_SW_BLINK
	.set_period	= gpio_led_set_period,
#endif
};

U_BOOT_DRIVER(led_gpio) = {
	.name	= "gpio_led",
	.id	= UCLASS_LED,
	.ops	= &gpio_led_ops,
	.priv_auto	= sizeof(struct led_gpio_priv),
	.probe	= led_gpio_probe,
	.remove	= led_gpio_remove,
};

static const struct udevice_id led_gpio_ids[] = {
	{ .compatible = "gpio-leds" },
	{ }
};

U_BOOT_DRIVER(led_gpio_wrap) = {
	.name	= "gpio_led_wrap",
	.id	= UCLASS_NOP,
	.of_match = led_gpio_ids,
	.bind	= led_gpio_bind,
};
