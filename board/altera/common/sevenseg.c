/*
 * (C) Copyright 2003, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * common/sevenseg.c
 *
 * NIOS PIO based seven segment led support functions
 */

#include <common.h>
#include <nios-io.h>

#ifdef	CONFIG_SEVENSEG

#define SEVENDEG_MASK_DP	((SEVENSEG_DIGIT_DP << 8) | SEVENSEG_DIGIT_DP)

#ifdef	SEVENSEG_WRONLY	/* emulate read access */
#if (SEVENSEG_ACTIVE == 0)
static unsigned int sevenseg_portval = ~0;
#else
static unsigned int sevenseg_portval = 0;
#endif
#endif

static int sevenseg_init_done = 0;

static inline void __sevenseg_set_masked (unsigned int mask, int value)
{
	nios_pio_t *piop __attribute__((unused)) = (nios_pio_t*)SEVENSEG_BASE;

#ifdef	SEVENSEG_WRONLY	/* emulate read access */

#if (SEVENSEG_ACTIVE == 0)
	if (value)
		sevenseg_portval &= ~mask;
	else
		sevenseg_portval |= mask;
#else
	if (value)
		sevenseg_portval |= mask;
	else
		sevenseg_portval &= ~mask;
#endif

	piop->data = sevenseg_portval;

#else	/* !SEVENSEG_WRONLY */

#if (SEVENSEG_ACTIVE == 0)
	if (value)
		piop->data &= ~mask;
	else
		piop->data |= mask;
#else
	if (value)
		piop->data |= mask;
	else
		piop->data &= ~mask;
#endif

#endif	/* SEVENSEG_WRONLY */
}

static inline void __sevenseg_toggle_masked (unsigned int mask)
{
	nios_pio_t *piop = (nios_pio_t*)SEVENSEG_BASE;

#ifdef	SEVENSEG_WRONLY	/* emulate read access */

	sevenseg_portval ^= mask;
	piop->data = sevenseg_portval;

#else	/* !SEVENSEG_WRONLY */

	piop->data ^= mask;

#endif	/* SEVENSEG_WRONLY */
}

static inline void __sevenseg_set (unsigned int value)
{
	nios_pio_t *piop __attribute__((unused)) = (nios_pio_t*)SEVENSEG_BASE;

#ifdef	SEVENSEG_WRONLY	/* emulate read access */

#if (SEVENSEG_ACTIVE == 0)
	sevenseg_portval = (sevenseg_portval &   SEVENDEG_MASK_DP)
		         | ((~value)         & (~SEVENDEG_MASK_DP));
#else
	sevenseg_portval = (sevenseg_portval & SEVENDEG_MASK_DP)
		         | (value);
#endif

	piop->data = sevenseg_portval;

#else	/* !SEVENSEG_WRONLY */

#if (SEVENSEG_ACTIVE == 0)
	piop->data = (piop->data &   SEVENDEG_MASK_DP)
		   | ((~value)   & (~SEVENDEG_MASK_DP));
#else
	piop->data = (piop->data & SEVENDEG_MASK_DP)
		   | (value);
#endif

#endif	/* SEVENSEG_WRONLY */
}

static inline void __sevenseg_init (void)
{
	nios_pio_t *piop __attribute__((unused)) = (nios_pio_t*)SEVENSEG_BASE;

	__sevenseg_set(0);

#ifndef	SEVENSEG_WRONLY	/* setup direction */

	piop->direction |= mask;

#endif	/* SEVENSEG_WRONLY */
}


void sevenseg_set(int value)
{
	unsigned char	digits[] = {
		SEVENSEG_DIGITS_0,
		SEVENSEG_DIGITS_1,
		SEVENSEG_DIGITS_2,
		SEVENSEG_DIGITS_3,
		SEVENSEG_DIGITS_4,
		SEVENSEG_DIGITS_5,
		SEVENSEG_DIGITS_6,
		SEVENSEG_DIGITS_7,
		SEVENSEG_DIGITS_8,
		SEVENSEG_DIGITS_9,
		SEVENSEG_DIGITS_A,
		SEVENSEG_DIGITS_B,
		SEVENSEG_DIGITS_C,
		SEVENSEG_DIGITS_D,
		SEVENSEG_DIGITS_E,
		SEVENSEG_DIGITS_F
	};

	if (!sevenseg_init_done) {
		__sevenseg_init();
		sevenseg_init_done++;
	}

	switch (value & SEVENSEG_MASK_CTRL) {

		case SEVENSEG_RAW:
			__sevenseg_set( (
				(digits[((value & SEVENSEG_MASK_VAL) >>  4)] << 8) |
				digits[((value & SEVENSEG_MASK_VAL) & 0xf)] ) );
			return;
			break;	/* paranoia */

		case SEVENSEG_OFF:
			__sevenseg_set(0);
			__sevenseg_set_masked(SEVENDEG_MASK_DP, 0);
			return;
			break;	/* paranoia */

		case SEVENSEG_SET_DPL:
			__sevenseg_set_masked(SEVENSEG_DIGIT_DP, 1);
			return;
			break;	/* paranoia */

		case SEVENSEG_SET_DPH:
			__sevenseg_set_masked((SEVENSEG_DIGIT_DP << 8), 1);
			return;
			break;	/* paranoia */

		case SEVENSEG_RES_DPL:
			__sevenseg_set_masked(SEVENSEG_DIGIT_DP, 0);
			return;
			break;	/* paranoia */

		case SEVENSEG_RES_DPH:
			__sevenseg_set_masked((SEVENSEG_DIGIT_DP << 8), 0);
			return;
			break;	/* paranoia */

		case SEVENSEG_TOG_DPL:
			__sevenseg_toggle_masked(SEVENSEG_DIGIT_DP);
			return;
			break;	/* paranoia */

		case SEVENSEG_TOG_DPH:
			__sevenseg_toggle_masked((SEVENSEG_DIGIT_DP << 8));
			return;
			break;	/* paranoia */

		case SEVENSEG_LO:
		case SEVENSEG_HI:
		case SEVENSEG_STR:
		default:
			break;
	}
}

#endif	/* CONFIG_SEVENSEG */
