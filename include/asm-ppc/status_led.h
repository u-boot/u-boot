/*
 * asm/status_led.h
 *
 * MPC8xx/MPC8260/MPC5xx based status led support functions
 */

#ifndef __ASM_STATUS_LED_H__
#define __ASM_STATUS_LED_H__

/* if not overriden */
#ifndef CONFIG_BOARD_SPECIFIC_LED
# if defined(CONFIG_8xx)
#  include <mpc8xx.h>
# elif defined(CONFIG_8260)
#  include <mpc8260.h>
# elif defined(CONFIG_5xx)
#  include <mpc5xx.h>
# else
#  error CPU specific Status LED header file missing.
#endif

/* led_id_t is unsigned long mask */
typedef unsigned long led_id_t;

static inline void __led_init (led_id_t mask, int state)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

#ifdef STATUS_LED_PAR
	immr->STATUS_LED_PAR &= ~mask;
#endif
#ifdef STATUS_LED_ODR
	immr->STATUS_LED_ODR &= ~mask;
#endif

#if (STATUS_LED_ACTIVE == 0)
	if (state == STATUS_LED_ON)
		immr->STATUS_LED_DAT &= ~mask;
	else
		immr->STATUS_LED_DAT |= mask;
#else
	if (state == STATUS_LED_ON)
		immr->STATUS_LED_DAT |= mask;
	else
		immr->STATUS_LED_DAT &= ~mask;
#endif
#ifdef STATUS_LED_DIR
	immr->STATUS_LED_DIR |= mask;
#endif
}

static inline void __led_toggle (led_id_t mask)
{
	((immap_t *) CFG_IMMR)->STATUS_LED_DAT ^= mask;
}

static inline void __led_set (led_id_t mask, int state)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

#if (STATUS_LED_ACTIVE == 0)
	if (state == STATUS_LED_ON)
		immr->STATUS_LED_DAT &= ~mask;
	else
		immr->STATUS_LED_DAT |= mask;
#else
	if (state == STATUS_LED_ON)
		immr->STATUS_LED_DAT |= mask;
	else
		immr->STATUS_LED_DAT &= ~mask;
#endif

}

#endif

#endif	/* __ASM_STATUS_LED_H__ */
