/*
 * PWM BACKLIGHT driver for Board based on EXYNOS.
 *
 * Author: Donghwa Lee  <dh09.lee@samsung.com>
 *
 * Derived from linux/drivers/video/backlight/pwm_backlight.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <common.h>
#include <pwm.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pwm.h>
#include <asm/arch/pwm_backlight.h>

static struct pwm_backlight_data *pwm;

static int exynos_pwm_backlight_update_status(void)
{
	int brightness = pwm->brightness;
	int max = pwm->max_brightness;

	if (brightness == 0) {
		pwm_config(pwm->pwm_id, 0, pwm->period);
		pwm_disable(pwm->pwm_id);
	} else {
		pwm_config(pwm->pwm_id,
			brightness * pwm->period / max, pwm->period);
		pwm_enable(pwm->pwm_id);
	}
	return 0;
}

int exynos_pwm_backlight_init(struct pwm_backlight_data *pd)
{
	pwm = pd;

	exynos_pwm_backlight_update_status();

	return 0;
}
