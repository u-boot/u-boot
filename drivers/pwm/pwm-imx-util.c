/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Basic support for the pwm modul on imx6.
 *
 * Based on linux:drivers/pwm/pwm-imx.c
 * from
 * Sascha Hauer <s.hauer@pengutronix.de>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>

/* pwm_id from 0..3 */
struct pwm_regs *pwm_id_to_reg(int pwm_id)
{
	switch (pwm_id) {
	case 0:
		return (struct pwm_regs *)PWM1_BASE_ADDR;
	case 1:
		return (struct pwm_regs *)PWM2_BASE_ADDR;
	case 2:
		return (struct pwm_regs *)PWM3_BASE_ADDR;
	case 3:
		return (struct pwm_regs *)PWM4_BASE_ADDR;
	default:
		printf("unknown pwm_id: %d\n", pwm_id);
		break;
	}
	return NULL;
}

int pwm_imx_get_parms(int period_ns, int duty_ns, unsigned long *period_c,
		      unsigned long *duty_c, unsigned long *prescale)
{
	unsigned long long c;

	/*
	 * we have not yet a clock framework for imx6, so add the clock
	 * value here as a define. Replace it when we have the clock
	 * framework.
	 */
	c = CONFIG_IMX6_PWM_PER_CLK;
	c = c * period_ns;
	do_div(c, 1000000000);
	*period_c = c;

	*prescale = *period_c / 0x10000 + 1;

	*period_c /= *prescale;
	c = *period_c * (unsigned long long)duty_ns;
	do_div(c, period_ns);
	*duty_c = c;

	/*
	 * according to imx pwm RM, the real period value should be
	 * PERIOD value in PWMPR plus 2.
	 */
	if (*period_c > 2)
		*period_c -= 2;
	else
		*period_c = 0;

	return 0;
}
