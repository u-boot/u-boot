/*
 * (C) Copyright 2006
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

#if defined(CONFIG_USB_OHCI_NEW) && defined(CFG_USB_OHCI_CPU_INIT)
# ifdef CONFIG_CPU_MONAHANS

#include <asm/arch/pxa-regs.h>

int usb_cpu_init()
{
	/* Enable USB host clock. */
	CKENA |= (CKENA_2_USBHOST |  CKENA_20_UDC);
	udelay(100);

	/* Configure Port 2 for Host (USB Client Registers) */
	UP2OCR = 0x3000c;

#if 0
	GPIO2_2 = 0x801; /* USBHPEN - Alt. Fkt. 1 */
	GPIO3_2 = 0x801; /* USBHPWR - Alt. Fkt. 1 */
#endif

	UHCHR |= UHCHR_FHR;
	wait_ms(11);
	UHCHR &= ~UHCHR_FHR;

	UHCHR |= UHCHR_FSBIR;
	while (UHCHR & UHCHR_FSBIR)
		udelay(1);

#if 0
	UHCHR |= UHCHR_PCPL; /* USBHPEN is active low */
	UHCHR |= UHCHR_PSPL; /* USBHPWR is active low */
#endif

	UHCHR &= ~UHCHR_SSEP0;
	UHCHR &= ~UHCHR_SSEP1;
	UHCHR &= ~UHCHR_SSE;

	return 0;
}

int usb_cpu_stop()
{
	/* may not want to do this */
	/* CKENA &= ~(CKENA_2_USBHOST |  CKENA_20_UDC); */
	return 0;
}

int usb_cpu_init_fail()
{
	return 0;
}

# endif /* CONFIG_CPU_MONAHANS */
#endif /* defined(CONFIG_USB_OHCI) && defined(CFG_USB_OHCI_CPU_INIT) */
