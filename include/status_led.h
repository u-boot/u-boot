/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * The purpose of this code is to signal the operational status of a
 * target which usually boots over the network; while running in
 * PCBoot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 */

#ifndef _STATUS_LED_H_
#define	_STATUS_LED_H_

#ifdef CONFIG_LED_STATUS

#define LED_STATUS_PERIOD	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ)
#ifdef CONFIG_LED_STATUS1
#define LED_STATUS_PERIOD1	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ1)
#endif /* CONFIG_LED_STATUS1 */
#ifdef CONFIG_LED_STATUS2
#define LED_STATUS_PERIOD2	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ2)
#endif /* CONFIG_LED_STATUS2 */
#ifdef CONFIG_LED_STATUS3
#define LED_STATUS_PERIOD3	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ3)
#endif /* CONFIG_LED_STATUS3 */
#ifdef CONFIG_LED_STATUS4
#define LED_STATUS_PERIOD4	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ4)
#endif /* CONFIG_LED_STATUS4 */
#ifdef CONFIG_LED_STATUS5
#define LED_STATUS_PERIOD5	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ5)
#endif /* CONFIG_LED_STATUS5 */

void status_led_init(void);
void status_led_tick(unsigned long timestamp);
void status_led_set(int led, int state);

static inline void status_led_boot_blink(void)
{
#ifdef CONFIG_LED_STATUS_BOOT_ENABLE
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_BLINKING);
#endif
}

#if defined(CONFIG_LED_STATUS_BOARD_SPECIFIC)
/* led_id_t is unsigned long mask */
typedef unsigned long led_id_t;

extern void __led_toggle (led_id_t mask);
extern void __led_init (led_id_t mask, int state);
extern void __led_set (led_id_t mask, int state);
void __led_blink(led_id_t mask, int freq);
#else
# error Status LED configuration missing
#endif

#else

static inline void status_led_init(void) { }
static inline void status_led_set(int led, int state) { }
static inline void status_led_boot_blink(void) { }

#endif	/* CONFIG_LED_STATUS	*/
#endif	/* _STATUS_LED_H_	*/
