/*
 * Copyright (C) 2012 Boundary Devices Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <watchdog.h>

static int force_idle_bus(void *priv)
{
	int i;
	int sda, scl;
	ulong elapsed, start_time;
	struct i2c_pads_info *p = (struct i2c_pads_info *)priv;
	int ret = 0;

	gpio_direction_input(p->sda.gp);
	gpio_direction_input(p->scl.gp);

	imx_iomux_v3_setup_pad(p->sda.gpio_mode);
	imx_iomux_v3_setup_pad(p->scl.gpio_mode);

	sda = gpio_get_value(p->sda.gp);
	scl = gpio_get_value(p->scl.gp);
	if ((sda & scl) == 1)
		goto exit;		/* Bus is idle already */

	printf("%s: sda=%d scl=%d sda.gp=0x%x scl.gp=0x%x\n", __func__,
		sda, scl, p->sda.gp, p->scl.gp);
	/* Send high and low on the SCL line */
	for (i = 0; i < 9; i++) {
		gpio_direction_output(p->scl.gp, 0);
		udelay(50);
		gpio_direction_input(p->scl.gp);
		udelay(50);
	}
	start_time = get_timer(0);
	for (;;) {
		sda = gpio_get_value(p->sda.gp);
		scl = gpio_get_value(p->scl.gp);
		if ((sda & scl) == 1)
			break;
		WATCHDOG_RESET();
		elapsed = get_timer(start_time);
		if (elapsed > (CONFIG_SYS_HZ / 5)) {	/* .2 seconds */
			ret = -EBUSY;
			printf("%s: failed to clear bus, sda=%d scl=%d\n",
					__func__, sda, scl);
			break;
		}
	}
exit:
	imx_iomux_v3_setup_pad(p->sda.i2c_mode);
	imx_iomux_v3_setup_pad(p->scl.i2c_mode);
	return ret;
}

static void * const i2c_bases[] = {
	(void *)I2C1_BASE_ADDR,
	(void *)I2C2_BASE_ADDR,
#ifdef I2C3_BASE_ADDR
	(void *)I2C3_BASE_ADDR,
#endif
};

/* i2c_index can be from 0 - 2 */
void setup_i2c(unsigned i2c_index, int speed, int slave_addr,
		struct i2c_pads_info *p)
{
	if (i2c_index >= ARRAY_SIZE(i2c_bases))
		return;
	/* Enable i2c clock */
	enable_i2c_clk(1, i2c_index);
	/* Make sure bus is idle */
	force_idle_bus(p);
	bus_i2c_init(i2c_bases[i2c_index], speed, slave_addr,
			force_idle_bus, p);
}
