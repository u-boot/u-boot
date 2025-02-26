// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_LED

#include <dm.h>
#include <errno.h>
#include <led.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>
#include <dt-bindings/leds/common.h>

static const char * const led_colors[LED_COLOR_ID_MAX] = {
	[LED_COLOR_ID_WHITE] = "white",
	[LED_COLOR_ID_RED] = "red",
	[LED_COLOR_ID_GREEN] = "green",
	[LED_COLOR_ID_BLUE] = "blue",
	[LED_COLOR_ID_AMBER] = "amber",
	[LED_COLOR_ID_VIOLET] = "violet",
	[LED_COLOR_ID_YELLOW] = "yellow",
	[LED_COLOR_ID_IR] = "ir",
	[LED_COLOR_ID_MULTI] = "multicolor",
	[LED_COLOR_ID_RGB] = "rgb",
	[LED_COLOR_ID_PURPLE] = "purple",
	[LED_COLOR_ID_ORANGE] = "orange",
	[LED_COLOR_ID_PINK] = "pink",
	[LED_COLOR_ID_CYAN] = "cyan",
	[LED_COLOR_ID_LIME] = "lime",
};

int led_bind_generic(struct udevice *parent, const char *driver_name)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		ret = device_bind_driver_to_node(parent, driver_name,
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
	}

	return 0;
}

int led_get_by_label(const char *label, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_LED, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);

		/* Ignore the top-level LED node */
		if (uc_plat->label && !strcmp(label, uc_plat->label))
			return uclass_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->set_state)
		return -ENOSYS;

	if (IS_ENABLED(CONFIG_LED_SW_BLINK) &&
	    led_sw_on_state_change(dev, state))
		return 0;

	return ops->set_state(dev, state);
}

enum led_state_t led_get_state(struct udevice *dev)
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->get_state)
		return -ENOSYS;

	if (IS_ENABLED(CONFIG_LED_SW_BLINK) &&
	    led_sw_is_blinking(dev))
		return LEDST_BLINK;

	return ops->get_state(dev);
}

int led_set_period(struct udevice *dev, int period_ms)
{
#ifdef CONFIG_LED_BLINK
	struct led_ops *ops = led_get_ops(dev);

	if (ops->set_period)
		return ops->set_period(dev, period_ms);
#endif

	if (IS_ENABLED(CONFIG_LED_SW_BLINK))
		return led_sw_set_period(dev, period_ms);

	return -ENOSYS;
}

#ifdef CONFIG_LED_BOOT
static int led_boot_get(struct udevice **devp, int *period_ms)
{
	struct led_uc_priv *priv;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_LED, &uc);
	if (ret)
		return ret;

	priv = uclass_get_priv(uc);
	if (!priv->boot_led_label)
		return -ENOENT;

	if (period_ms)
		*period_ms = priv->boot_led_period;

	return led_get_by_label(priv->boot_led_label, devp);
}

int led_boot_on(void)
{
	struct udevice *dev;
	int ret;

	ret = led_boot_get(&dev, NULL);
	if (ret)
		return ret;

	return led_set_state(dev, LEDST_ON);
}

int led_boot_off(void)
{
	struct udevice *dev;
	int ret;

	ret = led_boot_get(&dev, NULL);
	if (ret)
		return ret;

	return led_set_state(dev, LEDST_OFF);
}

#if defined(CONFIG_LED_BLINK) || defined(CONFIG_LED_SW_BLINK)
int led_boot_blink(void)
{
	struct udevice *dev;
	int period_ms, ret;

	ret = led_boot_get(&dev, &period_ms);
	if (ret)
		return ret;

	ret = led_set_period(dev, period_ms);
	if (ret) {
		if (ret != -ENOSYS)
			return ret;

		/* fallback to ON with no set_period and no SW_BLINK */
		return led_set_state(dev, LEDST_ON);
	}

	return led_set_state(dev, LEDST_BLINK);
}
#endif
#endif

#ifdef CONFIG_LED_ACTIVITY
static int led_activity_get(struct udevice **devp, int *period_ms)
{
	struct led_uc_priv *priv;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_LED, &uc);
	if (ret)
		return ret;

	priv = uclass_get_priv(uc);
	if (!priv->activity_led_label)
		return -ENOENT;

	if (period_ms)
		*period_ms = priv->activity_led_period;

	return led_get_by_label(priv->activity_led_label, devp);
}

int led_activity_on(void)
{
	struct udevice *dev;
	int ret;

	ret = led_activity_get(&dev, NULL);
	if (ret)
		return ret;

	return led_set_state(dev, LEDST_ON);
}

int led_activity_off(void)
{
	struct udevice *dev;
	int ret;

	ret = led_activity_get(&dev, NULL);
	if (ret)
		return ret;

	return led_set_state(dev, LEDST_OFF);
}

#if defined(CONFIG_LED_BLINK) || defined(CONFIG_LED_SW_BLINK)
int led_activity_blink(void)
{
	struct udevice *dev;
	int period_ms, ret;

	ret = led_activity_get(&dev, &period_ms);
	if (ret)
		return ret;

	ret = led_set_period(dev, period_ms);
	if (ret) {
		if (ret != -ENOSYS)
			return ret;

		/* fallback to ON with no set_period and no SW_BLINK */
		return led_set_state(dev, LEDST_ON);
	}

	return led_set_state(dev, LEDST_BLINK);
}
#endif
#endif

static const char *led_get_function_name(struct udevice *dev)
{
	struct led_uc_plat *uc_plat;
	const char *func;
	u32 color;
	u32 enumerator;
	int ret;
	int cp;

	if (!dev)
		return NULL;

	uc_plat = dev_get_uclass_plat(dev);
	if (!uc_plat)
		return NULL;

	if (uc_plat->label)
		return uc_plat->label;

	/* Now try to detect function label name */
	func = dev_read_string(dev, "function");
	cp = dev_read_u32(dev, "color", &color);
	/*
	 *  prevent coverity scan error CID 541279: (TAINTED_SCALAR)
	 *  only check the upper bound. No need to check the lower bound
	 *  as color is from type u32 and never can be lower than 0.
	 */
	if (color >= LED_COLOR_ID_MAX)
		cp = -EINVAL;

	if (cp == 0 || func) {
		ret = dev_read_u32(dev, "function-enumerator", &enumerator);
		if (!ret) {
			snprintf(uc_plat->name, LED_MAX_NAME_SIZE,
				 "%s:%s-%d",
				 cp ? "" : led_colors[color],
				 func ? func : "", enumerator);
		} else {
			snprintf(uc_plat->name, LED_MAX_NAME_SIZE,
				 "%s:%s",
				 cp ? "" : led_colors[color],
				 func ? func : "");
		}
		uc_plat->label = uc_plat->name;
	}

	return uc_plat->label;
}

static const char *led_get_label(struct udevice *dev, ofnode node)
{
	const char *label;

	label = ofnode_read_string(node, "label");
	if (!label)
		label = led_get_function_name(dev);
	if (!label && !ofnode_read_string(node, "compatible"))
		label = ofnode_get_name(node);

	return label;
}

static int led_post_bind(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	const char *default_state;

	if (!uc_plat->label)
		uc_plat->label = led_get_label(dev, dev_ofnode(dev));

	uc_plat->default_state = LEDST_COUNT;

	default_state = dev_read_string(dev, "default-state");
	if (!default_state)
		return 0;

	if (!strncmp(default_state, "on", 2))
		uc_plat->default_state = LEDST_ON;
	else if (!strncmp(default_state, "off", 3))
		uc_plat->default_state = LEDST_OFF;
	else
		return 0;

	if (IS_ENABLED(CONFIG_LED_BLINK)) {
		const char *trigger;

		trigger = dev_read_string(dev, "linux,default-trigger");
		if (trigger && !strncmp(trigger, "pattern", 7))
			uc_plat->default_state = LEDST_BLINK;
	}

	/*
	 * In case the LED has default-state DT property, trigger
	 * probe() to configure its default state during startup.
	 */
	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static int led_post_probe(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	int default_period_ms = 1000;
	int ret = 0;

	switch (uc_plat->default_state) {
	case LEDST_ON:
	case LEDST_OFF:
		ret = led_set_state(dev, uc_plat->default_state);
		break;
	case LEDST_BLINK:
		ret = led_set_period(dev, default_period_ms);
		if (!ret)
			ret = led_set_state(dev, uc_plat->default_state);
		break;
	default:
		break;
	}

	return ret;
}

#if defined(CONFIG_LED_BOOT) || defined(CONFIG_LED_ACTIVITY)
static int led_init(struct uclass *uc)
{
	struct led_uc_priv *priv = uclass_get_priv(uc);
	ofnode led_node;
	int ret;

#ifdef CONFIG_LED_BOOT
	ret = ofnode_options_get_by_phandle("boot-led", &led_node);
	if (!ret)
		priv->boot_led_label = led_get_label(NULL, led_node);
	priv->boot_led_period = ofnode_options_read_int("boot-led-period-ms", 250);
#endif

#ifdef CONFIG_LED_ACTIVITY
	ret = ofnode_options_get_by_phandle("activity-led", &led_node);
	if (!ret)
		priv->activity_led_label = led_get_label(NULL, led_node);
	priv->activity_led_period = ofnode_options_read_int("activity-led-period-ms",
							    250);
#endif

	return 0;
}
#endif

UCLASS_DRIVER(led) = {
	.id		= UCLASS_LED,
	.name		= "led",
	.per_device_plat_auto	= sizeof(struct led_uc_plat),
	.post_bind	= led_post_bind,
	.post_probe	= led_post_probe,
#if defined(CONFIG_LED_BOOT) || defined(CONFIG_LED_ACTIVITY)
	.init		= led_init,
	.priv_auto	= sizeof(struct led_uc_priv),
#endif
};
