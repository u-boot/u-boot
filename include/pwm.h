/*
 * header file for pwm driver.
 *
 * Copyright (c) 2011 samsung electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _pwm_h_
#define _pwm_h_

int	pwm_init		(int pwm_id, int div, int invert);
int	pwm_config		(int pwm_id, int duty_ns, int period_ns);
int	pwm_enable		(int pwm_id);
void	pwm_disable		(int pwm_id);

#endif /* _pwm_h_ */
