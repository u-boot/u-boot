// SPDX-License-Identifier: GPL-2.0+
/*
 * Software blinking helpers
 * Copyright (C) 2024 IOPSYS Software Solutions AB
 * Author: Mikhail Kshevetskiy <mikhail.kshevetskiy@iopsys.eu>
 */

#include <cyclic.h>
#include <dm.h>
#include <led.h>
#include <time.h>
#include <stdlib.h>

#define CYCLIC_NAME_PREFIX	"led_sw_blink_"

static void led_sw_blink(struct cyclic_info *c)
{
	struct led_sw_blink *sw_blink;
	struct udevice *dev;
	struct led_ops *ops;

	sw_blink = container_of(c, struct led_sw_blink, cyclic);
	dev = sw_blink->dev;
	ops = led_get_ops(dev);

	switch (sw_blink->state) {
	case LED_SW_BLINK_ST_OFF:
		sw_blink->state = LED_SW_BLINK_ST_ON;
		ops->set_state(dev, LEDST_ON);
		break;
	case LED_SW_BLINK_ST_ON:
		sw_blink->state = LED_SW_BLINK_ST_OFF;
		ops->set_state(dev, LEDST_OFF);
		break;
	case LED_SW_BLINK_ST_NOT_READY:
		/*
		 * led_set_period has been called, but
		 * led_set_state(LDST_BLINK) has not yet,
		 * so doing nothing
		 */
		break;
	default:
		break;
	}
}

int led_sw_set_period(struct udevice *dev, int period_ms)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct led_sw_blink *sw_blink = uc_plat->sw_blink;
	struct led_ops *ops = led_get_ops(dev);
	int half_period_us;

	half_period_us = period_ms * 1000 / 2;

	if (!sw_blink) {
		int len = sizeof(struct led_sw_blink) +
			  strlen(CYCLIC_NAME_PREFIX) +
			  strlen(uc_plat->label) + 1;

		sw_blink = calloc(1, len);
		if (!sw_blink)
			return -ENOMEM;

		sw_blink->dev = dev;
		sw_blink->state = LED_SW_BLINK_ST_DISABLED;
		strcpy((char *)sw_blink->cyclic_name, CYCLIC_NAME_PREFIX);
		strcat((char *)sw_blink->cyclic_name, uc_plat->label);

		uc_plat->sw_blink = sw_blink;
	}

	if (sw_blink->state == LED_SW_BLINK_ST_DISABLED) {
		cyclic_register(&sw_blink->cyclic, led_sw_blink,
				half_period_us, sw_blink->cyclic_name);
	} else {
		sw_blink->cyclic.delay_us = half_period_us;
		sw_blink->cyclic.start_time_us = timer_get_us();
	}

	sw_blink->state = LED_SW_BLINK_ST_NOT_READY;
	ops->set_state(dev, LEDST_OFF);

	return 0;
}

bool led_sw_is_blinking(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct led_sw_blink *sw_blink = uc_plat->sw_blink;

	if (!sw_blink)
		return false;

	return sw_blink->state > LED_SW_BLINK_ST_NOT_READY;
}

bool led_sw_on_state_change(struct udevice *dev, enum led_state_t state)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct led_sw_blink *sw_blink = uc_plat->sw_blink;

	if (!sw_blink || sw_blink->state == LED_SW_BLINK_ST_DISABLED)
		return false;

	if (state == LEDST_BLINK) {
		struct led_ops *ops = led_get_ops(dev);

		/*
		 * toggle LED initially and start blinking on next
		 * led_sw_blink() call.
		 */
		switch (ops->get_state(dev)) {
		case LEDST_ON:
			ops->set_state(dev, LEDST_OFF);
			sw_blink->state = LED_SW_BLINK_ST_OFF;
		default:
			ops->set_state(dev, LEDST_ON);
			sw_blink->state = LED_SW_BLINK_ST_ON;
		}

		return true;
	}

	/* stop blinking */
	uc_plat->sw_blink = NULL;
	cyclic_unregister(&sw_blink->cyclic);
	free(sw_blink);

	return false;
}
