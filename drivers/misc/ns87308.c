/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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

#include <config.h>

#ifdef CFG_NS87308

#include <ns87308.h>

void initialise_ns87308 (void)
{
#ifdef CFG_NS87308_PS2MOD
	unsigned char data;

	/*
	 * Switch floppy drive to PS/2 mode.
	 */
	read_pnp_config(SUPOERIO_CONF1, &data);
	data &= 0xFB;
	write_pnp_config(SUPOERIO_CONF1, data);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_KBC1)
	PNP_SET_DEVICE_BASE(LDEV_KBC1, CFG_NS87308_KBC1_BASE);
	write_pnp_config(LUN_CONFIG_REG, 0);
	write_pnp_config(CBASE_HIGH, 0x00);
	write_pnp_config(CBASE_LOW, 0x64);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_MOUSE)
	PNP_ACTIVATE_DEVICE(LDEV_MOUSE);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_RTC_APC)
	PNP_SET_DEVICE_BASE(LDEV_RTC_APC, CFG_NS87308_RTC_BASE);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_FDC)
	PNP_SET_DEVICE_BASE(LDEV_FDC, CFG_NS87308_FDC_BASE);
	write_pnp_config(LUN_CONFIG_REG, 0x40);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_RARP)
	PNP_SET_DEVICE_BASE(LDEV_PARP, CFG_NS87308_LPT_BASE);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_UART1)
	PNP_SET_DEVICE_BASE(LDEV_UART1, CFG_NS87308_UART1_BASE);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_UART2)
	PNP_SET_DEVICE_BASE(LDEV_UART2, CFG_NS87308_UART2_BASE);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_GPIO)
	PNP_SET_DEVICE_BASE(LDEV_GPIO, CFG_NS87308_GPIO_BASE);
#endif

#if (CFG_NS87308_DEVS & CFG_NS87308_POWRMAN)
#ifndef CFG_NS87308_PWMAN_BASE
	PNP_ACTIVATE_DEVICE(LDEV_POWRMAN);
#else
	PNP_SET_DEVICE_BASE(LDEV_POWRMAN, CFG_NS87308_PWMAN_BASE);

	/*
	 * Enable all units
	 */
	write_pm_reg(CFG_NS87308_PWMAN_BASE, PWM_FER1, 0x7d);
	write_pm_reg(CFG_NS87308_PWMAN_BASE, PWM_FER2, 0x87);

#ifdef CFG_NS87308_PMC1
	write_pm_reg(CFG_NS87308_PWMAN_BASE, PWM_PMC1, CFG_NS87308_PMC1);
#endif

#ifdef CFG_NS87308_PMC2
	write_pm_reg(CFG_NS87308_PWMAN_BASE, PWM_PMC2, CFG_NS87308_PMC2);
#endif

#ifdef CFG_NS87308_PMC3
	write_pm_reg(CFG_NS87308_PWMAN_BASE, PWM_PMC3, CFG_NS87308_PMC3);
#endif
#endif
#endif

#ifdef CFG_NS87308_CS0_BASE
	PNP_PGCS_CSLINE_BASE(0, CFG_NS87308_CS0_BASE);
	PNP_PGCS_CSLINE_CONF(0, CFG_NS87308_CS0_CONF);
#endif

#ifdef CFG_NS87308_CS1_BASE
	PNP_PGCS_CSLINE_BASE(1, CFG_NS87308_CS1_BASE);
	PNP_PGCS_CSLINE_CONF(1, CFG_NS87308_CS1_CONF);
#endif

#ifdef CFG_NS87308_CS2_BASE
	PNP_PGCS_CSLINE_BASE(2, CFG_NS87308_CS2_BASE);
	PNP_PGCS_CSLINE_CONF(2, CFG_NS87308_CS2_CONF);
#endif
}

#endif
