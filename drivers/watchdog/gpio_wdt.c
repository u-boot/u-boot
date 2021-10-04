// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <dm/device_compat.h>
#include <wdt.h>
#include <asm/gpio.h>

struct gpio_wdt_priv {
	struct gpio_desc gpio;
	bool always_running;
	int state;
};

static int gpio_wdt_reset(struct udevice *dev)
{
	struct gpio_wdt_priv *priv = dev_get_priv(dev);

	priv->state = !priv->state;

	return dm_gpio_set_value(&priv->gpio, priv->state);
}

static int gpio_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct gpio_wdt_priv *priv = dev_get_priv(dev);

	if (priv->always_running)
		return 0;

	return -ENOSYS;
}

static int dm_probe(struct udevice *dev)
{
	struct gpio_wdt_priv *priv = dev_get_priv(dev);
	int ret;

	priv->always_running = dev_read_bool(dev, "always-running");
	ret = gpio_request_by_name(dev, "gpios", 0, &priv->gpio, GPIOD_IS_OUT);
	if (ret < 0) {
		dev_err(dev, "Request for wdt gpio failed: %d\n", ret);
		return ret;
	}

	if (priv->always_running)
		ret = gpio_wdt_reset(dev);

	return ret;
}

static const struct wdt_ops gpio_wdt_ops = {
	.start = gpio_wdt_start,
	.reset = gpio_wdt_reset,
};

static const struct udevice_id gpio_wdt_ids[] = {
	{ .compatible = "linux,wdt-gpio" },
	{}
};

U_BOOT_DRIVER(wdt_gpio) = {
	.name = "wdt_gpio",
	.id = UCLASS_WDT,
	.of_match = gpio_wdt_ids,
	.ops = &gpio_wdt_ops,
	.probe	= dm_probe,
	.priv_auto = sizeof(struct gpio_wdt_priv),
};
