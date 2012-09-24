/*
 * Copyright (C) 2012 Sughosh Ganu <urwithsughosh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>

#include <asm/arch/da8xx-usb.h>

int usb_cpu_init(void)
{
	/* enable psc for usb2.0 */
	lpsc_on(DAVINCI_LPSC_USB20);

	/* enable psc for usb1.0 */
	lpsc_on(DAVINCI_LPSC_USB11);

	/* start the on-chip usb phy and its pll */
	if (usb_phy_on())
		return 0;

	return 1;
}

int usb_cpu_stop(void)
{
	usb_phy_off();

	/* turn off the usb clock and assert the module reset */
	lpsc_disable(DAVINCI_LPSC_USB11);
	lpsc_disable(DAVINCI_LPSC_USB20);

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}
