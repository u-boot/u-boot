/*
 * (C) Copyright 2006
 * DENX Software Engineering <mk@denx.de>
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

#if defined(CONFIG_USB_OHCI_NEW) && defined(CFG_USB_OHCI_CPU_INIT)
# if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410)

#if defined(CONFIG_S3C2400)
# include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
# include <s3c2410.h>
#endif

int usb_cpu_init (void)
{

	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/*
	 * Set the 48 MHz UPLL clocking. Values are taken from
	 * "PLL value selection guide", 6-23, s3c2400_UM.pdf.
	 */
	clk_power->UPLLCON = ((40 << 12) + (1 << 4) + 2);
	gpio->MISCCR |= 0x8; /* 1 = use pads related USB for USB host */

	/*
	 * Enable USB host clock.
	 */
	clk_power->CLKCON |= (1 << 4);

	return 0;
}

int usb_cpu_stop (void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	/* may not want to do this */
	clk_power->CLKCON &= ~(1 << 4);
	return 0;
}

int usb_cpu_init_fail (void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	clk_power->CLKCON &= ~(1 << 4);
	return 0;
}

# endif /* defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410) */
#endif /* defined(CONFIG_USB_OHCI) && defined(CFG_USB_OHCI_CPU_INIT) */
