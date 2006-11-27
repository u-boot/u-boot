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
# ifdef CONFIG_AT91RM9200

#include <asm/arch/hardware.h>

int usb_cpu_init()
{
	/* Enable USB host clock. */
	*AT91C_PMC_SCER = AT91C_PMC_UHP;	/* 48MHz clock enabled for UHP */
	*AT91C_PMC_PCER = 1 << AT91C_ID_UHP;	/* Peripheral Clock Enable Register */
	return 0;
}

int usb_cpu_stop()
{
	/* Initialization failed */
	*AT91C_PMC_PCDR = 1 << AT91C_ID_UHP;	/* Peripheral Clock Disable Register */
	*AT91C_PMC_SCDR = AT91C_PMC_UHP;	/* 48MHz clock disabled for UHP */
	return 0;
}

int usb_cpu_init_fail()
{
	usb_cpu_stop();
}

# endif /* CONFIG_AT91RM9200 */
#endif /* defined(CONFIG_USB_OHCI) && defined(CFG_USB_OHCI_CPU_INIT) */
