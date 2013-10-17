/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <status_led.h>

/*
 * The purpose of this code is to signal the operational status of a
 * target which usually boots over the network; while running in
 * U-Boot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 */

/* ------------------------------------------------------------------------- */

typedef struct {
	led_id_t mask;
	int state;
	int period;
	int cnt;
} led_dev_t;

led_dev_t led_dev[] = {
    {	STATUS_LED_BIT,
	STATUS_LED_STATE,
	STATUS_LED_PERIOD,
	0,
    },
#if defined(STATUS_LED_BIT1)
    {	STATUS_LED_BIT1,
	STATUS_LED_STATE1,
	STATUS_LED_PERIOD1,
	0,
    },
#endif
#if defined(STATUS_LED_BIT2)
    {	STATUS_LED_BIT2,
	STATUS_LED_STATE2,
	STATUS_LED_PERIOD2,
	0,
    },
#endif
#if defined(STATUS_LED_BIT3)
    {	STATUS_LED_BIT3,
	STATUS_LED_STATE3,
	STATUS_LED_PERIOD3,
	0,
    },
#endif
};

#define MAX_LED_DEV	(sizeof(led_dev)/sizeof(led_dev_t))

static int status_led_init_done = 0;

static void status_led_init (void)
{
	led_dev_t *ld;
	int i;

	for (i = 0, ld = led_dev; i < MAX_LED_DEV; i++, ld++)
		__led_init (ld->mask, ld->state);
	status_led_init_done = 1;
}

void status_led_tick (ulong timestamp)
{
	led_dev_t *ld;
	int i;

	if (!status_led_init_done)
		status_led_init ();

	for (i = 0, ld = led_dev; i < MAX_LED_DEV; i++, ld++) {

		if (ld->state != STATUS_LED_BLINKING)
			continue;

		if (++ld->cnt >= ld->period) {
			__led_toggle (ld->mask);
			ld->cnt -= ld->period;
		}

	}
}

void status_led_set (int led, int state)
{
	led_dev_t *ld;

	if (led < 0 || led >= MAX_LED_DEV)
		return;

	if (!status_led_init_done)
		status_led_init ();

	ld = &led_dev[led];

	ld->state = state;
	if (state == STATUS_LED_BLINKING) {
		ld->cnt = 0;		/* always start with full period    */
		state = STATUS_LED_ON;	/* always start with LED _ON_       */
	}
	__led_set (ld->mask, state);
}
