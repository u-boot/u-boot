/*
 * (C) Copyright 2007
 * Markus Klotzbuecher, DENX Software Engineering <mk@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
