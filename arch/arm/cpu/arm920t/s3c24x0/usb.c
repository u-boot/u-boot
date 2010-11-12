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

#if defined(CONFIG_USB_OHCI_NEW) && \
    defined(CONFIG_SYS_USB_OHCI_CPU_INIT) && \
    defined(CONFIG_S3C24X0)

#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

int usb_cpu_init(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	struct s3c24x0_gpio *gpio = s3c24x0_get_base_gpio();

	/*
	 * Set the 48 MHz UPLL clocking. Values are taken from
	 * "PLL value selection guide", 6-23, s3c2400_UM.pdf.
	 */
	writel((40 << 12) + (1 << 4) + 2, &clk_power->upllcon);
	/* 1 = use pads related USB for USB host */
	writel(readl(&gpio->misccr) | 0x8, &gpio->misccr);

	/*
	 * Enable USB host clock.
	 */
	writel(readl(&clk_power->clkcon) | (1 << 4), &clk_power->clkcon);

	return 0;
}

int usb_cpu_stop(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	/* may not want to do this */
	writel(readl(&clk_power->clkcon) & ~(1 << 4), &clk_power->clkcon);
	return 0;
}

int usb_cpu_init_fail(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	writel(readl(&clk_power->clkcon) & ~(1 << 4), &clk_power->clkcon);
	return 0;
}

#endif /* defined(CONFIG_USB_OHCI_NEW) && \
	   defined(CONFIG_SYS_USB_OHCI_CPU_INIT) && \
	   defined(CONFIG_S3C24X0) */
