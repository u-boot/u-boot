/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <status_led.h>

/*
 * The purpose of this code is to signal the operational status of a
 * target which usually boots over the network; while running in
 * PCBoot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_STATUS_LED

typedef struct {
	ulong	mask;
	int	state;
	int	period;
	int	cnt;
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
};

#define MAX_LED_DEV	(sizeof(led_dev)/sizeof(led_dev_t))

static int status_led_init_done = 0;

static void status_led_init (void)
{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    int i;

    for (i=0; i<MAX_LED_DEV; ++i) {
	led_dev_t *ld = &led_dev[i];

	immr->STATUS_LED_PAR &= ~(ld->mask);
#ifdef STATUS_LED_ODR
	immr->STATUS_LED_ODR &= ~(ld->mask);
#endif
#if (STATUS_LED_ACTIVE == 0)
	if (ld->state == STATUS_LED_ON)
		immr->STATUS_LED_DAT &= ~(ld->mask);
	else
		immr->STATUS_LED_DAT |=   ld->mask ;
#else
	if (ld->state == STATUS_LED_ON)
		immr->STATUS_LED_DAT |=   ld->mask ;
	else
		immr->STATUS_LED_DAT &= ~(ld->mask);
#endif
	immr->STATUS_LED_DIR |=   ld->mask ;
    }

    status_led_init_done  = 1;
}

void status_led_tick (ulong timestamp)
{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    int i;

    if (!status_led_init_done)
	status_led_init();

    for (i=0; i<MAX_LED_DEV; ++i) {
	led_dev_t *ld = &led_dev[i];

	if (ld->state != STATUS_LED_BLINKING)
		continue;

	if (++(ld->cnt) >= ld->period) {
		immr->STATUS_LED_DAT ^= ld->mask;
		ld->cnt -= ld->period;
	}
    }
}

void status_led_set (int led, int state)
{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    led_dev_t *ld;

    if (led < 0 || led >= MAX_LED_DEV)
	return;

    if (!status_led_init_done)
	status_led_init();

    ld = &led_dev[led];

    switch (state) {
    default:
	return;
    case STATUS_LED_BLINKING:
	ld->cnt = 0;		/* always start with full period	*/
	/* fall through */	/* always start with LED _ON_		*/
    case STATUS_LED_ON:
#if (STATUS_LED_ACTIVE == 0)
	immr->STATUS_LED_DAT &= ~(ld->mask);
#else
	immr->STATUS_LED_DAT |=   ld->mask ;
#endif
	break;
    case STATUS_LED_OFF:
#if (STATUS_LED_ACTIVE == 0)
	immr->STATUS_LED_DAT |=   ld->mask ;
#else
	immr->STATUS_LED_DAT &= ~(ld->mask);
#endif
	break;
    }
    ld->state = state;
}

#endif	/* CONFIG_STATUS_LED */
