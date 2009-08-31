/*
 * (C) Copyright 2007
 * Markus Klotzbuecher, DENX Software Engineering <mk@denx.de>
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

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)

#include <mpc5xxx.h>

int usb_cpu_init(void)
{
	/* Set the USB Clock						     */
	*(vu_long *)MPC5XXX_CDM_48_FDC = CONFIG_USB_CLOCK;

#ifdef CONFIG_PSC3_USB /* USB is using the alternate configuration */
	/* remove all PSC3 USB bits first before ORing in ours */
	*(vu_long *)MPC5XXX_GPS_PORT_CONFIG &= ~0x00804f00;
#else
	/* remove all USB bits first before ORing in ours */
	*(vu_long *)MPC5XXX_GPS_PORT_CONFIG &= ~0x00807000;
#endif
	/* Activate USB port						     */
	*(vu_long *)MPC5XXX_GPS_PORT_CONFIG |= CONFIG_USB_CONFIG;

	return 0;
}

int usb_cpu_stop(void)
{
	return 0;
}

int usb_cpu_init_fail(void)
{
	return 0;
}

#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT) */
